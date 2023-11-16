#include <curses.h>
#include <stdlib.h>
#include <assert.h>

#include "ui.internal.h"

struct ui_window_root *ui_root = NULL;

void ui__default_draw_proc(struct ui_window_base *base)
{
  redrawwin(base->cwindow);
  wrefresh(base->cwindow);
}

void ui__init_window_base(struct ui_window_base *base)
{
  base->type = UI__WINDOW_TYPE_BASE;
  base->parent = NULL;
  base->cwindow = NULL;
  base->draw_proc = &ui__default_draw_proc;
  base->layout_proc = NULL;
}

void ui__init_window_dock(struct ui_window_dock *dock)
{
  ui__init_window_base(&dock->super);
  dock->super.type = UI__WINDOW_TYPE_DOCK;
  dock->super.draw_proc = &ui__dock_default_draw_proc;
  dock->super.layout_proc = &ui__dock_default_layout_proc;

  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    dock->children[i] = NULL;
    dock->childsizes[i] = 0.0;
  }

  dock->focus = UI__WINDOW_FOCUS_NONE;
}

void ui__init_window_root(struct ui_window_root *root)
{
  ui__init_window_dock(&root->super);
  root->super.super.type = UI__WINDOW_TYPE_ROOT;
  root->super.super.draw_proc = &ui__root_draw_proc;
  root->super.super.layout_proc = &ui__root_layout_proc;

  root->floating = NULL;
}

/* type-specific destructors */
void ui__destroy_window_base(struct ui_window_base *base);
void ui__destroy_window_dock(struct ui_window_dock *dock);

void ui__window_destroy_root(struct ui_window_root *root)
{
  if (root->floating)
    ui__destroy_window(root->floating);

  ui__destroy_window_dock(&root->super);
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

void ui__destroy_window_base(struct ui_window_base *base)
{
  delwin(base->cwindow);
  free(base);
}

void ui__destroy_window(struct ui_window_base *base)
{
  if (!base) return;

  switch (base->type)
  {
    case UI__WINDOW_TYPE_ROOT:
      ui__window_destroy_root((struct ui_window_root *)base);
      break;
    case UI__WINDOW_TYPE_DOCK:
      ui__destroy_window_dock((struct ui_window_dock *)base);
      break;
    case UI__WINDOW_TYPE_BASE:
      ui__destroy_window_base(base);
  }
}

/* the focused window MUST be a leaf window, or the screen will get clobbered.
 * This is because the focused window will have methods like getch() called on it,
 * which brings it to the "foreground", squashing whatever is behind it. */
struct ui_window_base *ui__find_focused(void)
{
  struct ui_window_base *window = (struct ui_window_base *)ui_root;
  if (ui_root->floating) return ui_root->floating;

  while (true)
  {
    switch (window->type)
    {
      case UI__WINDOW_TYPE_DOCK:
      case UI__WINDOW_TYPE_ROOT:
        {
          struct ui_window_dock *dock = (struct ui_window_dock *)window;

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

            return (struct ui_window_base *)dock; /* the dock is focused if it is really a leaf */
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

void ui__traces_draw_proc(struct ui_window_base *base)
{
  box(base->cwindow, 0, 0);
  int maxy, maxx;

  mvwaddstr(base->cwindow, 0, 2, "Traces");
  getmaxyx(base->cwindow, maxy, maxx);
  for (int i = 1; i < maxy-1; ++i)
  {
    mvwhline(base->cwindow, i, 1, '%', maxx-2);
  }
  touchwin(base->cwindow);
  wrefresh(base->cwindow);
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

  ui__init_window_root(ui_root);
  ui_root->super.super.cwindow = stdscr; /* initialize the ncurses window ourselves */

  /* set up UI */

  for (int i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    if (i == UI__WINDOW_DOCK_LEFT) continue;
    struct ui_window_base *win_traces = malloc(sizeof(struct ui_window_base));
    ui__init_window_base(win_traces);
    ui__dock_add_child((struct ui_window_dock *)ui_root, win_traces, i, 1./5);
    win_traces->draw_proc = &ui__traces_draw_proc;
  }

  ui__call_draw_proc((struct ui_window_base *)ui_root);
}

void ui_handle(void)
{
  while (true)
  {
    struct ui_window_base *window = ui__find_focused();

#ifdef NCURSES_WIDE
    wint_t inp;
    wget_wch(window->cwindow, &inp);
#else
    int inp = wgetch(window->cwindow);
#endif

    if (inp == NS('q'))
    {
      break;
    }
    else if (inp == KEY_RESIZE)
    {
      ui__call_layout_proc((struct ui_window_base *)ui_root);
      ui__call_draw_proc((struct ui_window_base *)ui_root);
    }
  }

  ui__window_destroy_root(ui_root);
  ui_root = NULL;

  endwin();
}
