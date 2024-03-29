#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "ui.internal.h"
#include "ui/uimenu.internal.h"
#include "../macros.h"
#include "config.h"

struct ui_window_root *ui_root = NULL;

void ui__default_draw_proc(struct ui_window_base *base)
{
  umps_unused(base);
}

void ui__leaf_draw_proc(struct ui_window_base *base)
{
  WINDOW *mywin = ((struct ui_window_leaf *)base)->cwindow;
  int maxy, maxx;
  getmaxyx(mywin, maxy, maxx);

  box(mywin, 0, 0);
  mvwaddstr(mywin, 0, 2, "Traces");
  for (int i = 1; i < maxy-1; ++i) {
    mvwhline(mywin, i, 1, '.', maxx - 2);
  }

  wnoutrefresh(mywin);
}

void ui__leaf_layout_proc(struct ui_window_base *base)
{
  struct ui_window_leaf *leaf = (struct ui_window_leaf *)base;
  if (leaf->cwindow) NCCI(delwin(leaf->cwindow));
  leaf->cwindow = NCCP(newwin(leaf->super.dims.maxy, leaf->super.dims.maxx, leaf->super.dims.begy, leaf->super.dims.begx));
}

void ui__init_window_base(struct ui_window_base *base)
{
  base->type = UI__WINDOW_TYPE_BASE;
  base->parent = NULL;

  base->dims.begy = 0;
  base->dims.begx = 0;
  base->dims.maxy = 0;
  base->dims.maxx = 0;

  base->draw_proc = &ui__default_draw_proc;
  base->layout_proc = NULL;
  base->control_proc = NULL;
}

void ui__init_window_leaf(struct ui_window_leaf *leaf)
{
  ui__init_window_base(&leaf->super);
  leaf->super.type = UI__WINDOW_TYPE_LEAF;
  leaf->super.draw_proc = &ui__leaf_draw_proc;
  leaf->super.layout_proc = &ui__leaf_layout_proc;

  leaf->cwindow = NULL;
}

void ui__init_window_dock(struct ui_window_dock *dock)
{
  ui__init_window_base(&dock->super);
  dock->super.type = UI__WINDOW_TYPE_DOCK;
  dock->super.draw_proc = &ui__dock_default_draw_proc;
  dock->super.layout_proc = &ui__dock_default_layout_proc;
  dock->super.control_proc = &ui__dock_default_control_proc;

  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    dock->children[i] = NULL;
    dock->childsizes[i] = 0.0;
  }

  dock->focus = UI__WINDOW_FOCUS_NONE;
}

void ui__init_window_root(struct ui_window_root *root, WINDOW *cwindow)
{
  ui__init_window_base(&root->super);
  root->super.type = UI__WINDOW_TYPE_ROOT;
  root->super.draw_proc = &ui__root_draw_proc;
  root->super.layout_proc = &ui__root_layout_proc;
  root->super.control_proc = &ui__root_control_proc;

  root->cwindow = cwindow;
  getmaxyx(cwindow, root->super.dims.maxy, root->super.dims.maxx);
  getbegyx(cwindow, root->super.dims.begy, root->super.dims.begx);

  root->menu_prefix = "";
  root->menu_prefix_len = 0;

  root->menu_cwindow = newwin(1, root->super.dims.maxx, 0, 0);
  root->menu_selected = NULL;
  root->menu_scroll = 0;

  root->undersize_scr = false;
  root->content = NULL;
  root->floating = NULL;
  root->modal = NULL;

  root->menu_root = malloc(sizeof(struct uimenu_item_menu));
  uimenu_item_menu_init(root->menu_root, NULL);
}

/* type-specific destructors */
void ui__destroy_window_base(struct ui_window_base *base);
void ui__destroy_window_leaf(struct ui_window_leaf *leaf);
void ui__destroy_window_dock(struct ui_window_dock *dock);

void ui__window_destroy_root(struct ui_window_root *root)
{
  if (root->content)
    ui__destroy_window(root->content);

  if (root->floating)
    ui__destroy_window(root->floating);

  uimenu_menu_free(root->menu_root);

  if (root->cwindow) delwin(root->cwindow);

  ui__destroy_window_base(&root->super);
}

void ui__destroy_window_dock(struct ui_window_dock *dock)
{
  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    if (dock->children[i]) ui__destroy_window(dock->children[i]);
    dock->children[i] = NULL;
  }

  ui__destroy_window_base(&dock->super);
}

void ui__destroy_window_leaf(struct ui_window_leaf *leaf)
{
  if (leaf->cwindow) delwin(leaf->cwindow);
  ui__destroy_window_base(&leaf->super);
}

void ui__destroy_window_base(struct ui_window_base *base)
{
  free(base);
}

void ui__destroy_window(struct ui_window_base *base)
{
  if (!base) return;

  switch (base->type)
  {
    case UI__WINDOW_TYPE_ROOT:
      ui__window_destroy_root(ui__cast(root, base));
      break;
    case UI__WINDOW_TYPE_DOCK:
      ui__destroy_window_dock(ui__cast(dock, base));
      break;
    case UI__WINDOW_TYPE_LEAF:
      ui__destroy_window_leaf(ui__cast(leaf, base));
      break;
    case UI__WINDOW_TYPE_BASE:
      ui__destroy_window_base(base);
  }
}

/* the focused window MUST be a leaf window, or the screen will get clobbered.
 * This is because the focused window will have wgetch() called for it, which
 * calls wrefresh() sometimes. This will clobber windows inside of it. */
struct ui_window_base *ui__find_focused(void)
{
  struct ui_window_base *window = ui__cast(base, ui_root);

  while (true) {
    switch (window->type) {
      case UI__WINDOW_TYPE_ROOT:
        {
          struct ui_window_root *root = ui__cast(root, window);
          if (root->undersize_scr) {
            return window; /* gobble up focus */
          } else if (root->floating) {
            window = root->floating;
          } else if (root->content) {
            window = root->content;
          } else {
            return window;
          }
          break;
        }
      case UI__WINDOW_TYPE_DOCK:
        {
          struct ui_window_dock *dock = ui__cast(dock, window);

          if (dock->focus == UI__WINDOW_FOCUS_NONE || !dock->children[dock->focus])
          {
            for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
            {
              if (dock->children[i])
              {
                dock->focus = i; /* focus the child (skip fallback return statement) */
                goto childfound;
              }
            }

            return window; /* the dock is focused if it is really a leaf */
          }

childfound:
          window = dock->children[dock->focus];
          break;
        }
      default:
        return window; /* a leaf window is focused */
    }
  }
}

WINDOW *ui__find_focused_leaf(struct ui_window_base *start)
{
  switch (start->type) {
    case UI__WINDOW_TYPE_LEAF:
      return ui__cast(leaf, start)->cwindow;
    case UI__WINDOW_TYPE_DOCK: {
      struct ui_window_dock *dock = ui__cast(dock, start);

      for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i) {
        if (dock->children[i]) {
          WINDOW *focus = ui__find_focused_leaf(dock->children[i]);
          if (focus) return focus;
        }
      }

      return NULL;
    }
    case UI__WINDOW_TYPE_ROOT: {
      struct ui_window_root *root = ui__cast(root, start);
      WINDOW *focus;

      if (root->undersize_scr)
        return root->cwindow; /* gobble up focus if the screen is undersize */

      if (root->modal) {
        focus = ui__find_focused_leaf(root->modal);
        if (focus) return focus;
      }

      if (root->floating) {
        focus = ui__find_focused_leaf(root->modal);
        if (focus) return focus;
      }

      if (root->menu_selected) {
        return root->menu_cwindow;
      }

      if (root->content) {
        return ui__find_focused_leaf(root->content);
      }

      return NULL;
    }
    default:
      umps_trap;
  }
}

void ui__call_draw_proc(struct ui_window_base *base)
{
  if (base->draw_proc)
  {
    (*base->draw_proc)(base);
  }
}

void ui__call_layout_proc(struct ui_window_base *base)
{
  if (base->layout_proc)
  {
    (*base->layout_proc)(base);
  }
}

struct ui_window_base *ui__call_control_proc(struct ui_window_base *base, ui_control inp)
{
  if (base->control_proc)
  {
    return (*base->control_proc)(base, inp);
  }

  return NULL;
}

void ui_init(void)
{
  ui_root = malloc(sizeof(struct ui_window_root)); /* TODO: check */
  assert(ui_root);

  initscr();

  raw();
  noecho();
  keypad(stdscr, TRUE);

  curs_set(0);

  ui__init_window_root(ui_root, stdscr);
  ui__call_layout_proc(ui__cast(base, ui_root));

  /* set up UI */

  ui_root->menu_prefix = " " UMPS_NAME " " UMPS_VERSION " ";
  ui_root->menu_prefix_len = strlen(ui_root->menu_prefix);

  struct ui_window_dock *maindock = malloc(sizeof(struct ui_window_dock));
  ui__init_window_dock(maindock);
  ui__root_set_content(ui_root, ui__cast(base, maindock));

  const char *menu_names[] = { "File", "View", "Tools", "Filters", "Help", "1", "22", "333", "4444", "55555", "666666", "7777777", "88888888", "999999999", "0000000000", NULL };
  for (int i = 0; menu_names[i]; ++i) {
    struct uimenu_item_menu *menu = malloc(sizeof(struct uimenu_item_menu));
    uimenu_item_menu_init(menu, menu_names[i]);
    uimenu_menu_add(ui_root->menu_root, ui_root->menu_root->tail, (struct uimenu_item_header *)menu, true);
  }

  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    if (i == UI__WINDOW_DOCK_LEFT) continue;
    struct ui_window_leaf *win_traces = malloc(sizeof(struct ui_window_leaf));
    ui__init_window_leaf(win_traces);
    ui__dock_add_child(maindock, ui__cast(base, win_traces), i, 1./5);
  }

  ui__call_layout_proc(ui__cast(base, ui_root)); /* should call layout proc to check for undersize at least */
  ui__call_draw_proc(ui__cast(base, ui_root));
}

void ui_handle(void)
{
  while (true)
  {
    WINDOW *cwindow = ui__find_focused_leaf(ui__cast(base, ui_root));

    ui_control inp;

#ifdef NCURSES_WIDE
    keypad(cwindow, TRUE);
    wget_wch(cwindow, &inp);
#else
    inp = wgetch(window->cwindow);
#endif

    if (inp == NS('q')) /* TODO: make quitting flag in ui_root */
    {
      break;
    }
    else if (inp == KEY_RESIZE)
    {
      getmaxyx(ui_root->cwindow, ui_root->super.dims.maxy, ui_root->super.dims.maxx);
      getbegyx(ui_root->cwindow, ui_root->super.dims.begy, ui_root->super.dims.begx);
      ui__call_layout_proc(ui__cast(base, ui_root));
      ui__call_draw_proc(ui__cast(base, ui_root));
    }

    struct ui_window_base *target = ui__cast(base, ui_root);
    while (target) {
      target = ui__call_control_proc(target, inp);
    }
  }

  ui__window_destroy_root(ui_root);
  ui_root = NULL;

  endwin();
}
