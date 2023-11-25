#include "ui.internal.h"
#include "macros.h"
#include "ui/uimenu.internal.h"
#include <curses.h>
#include <string.h>

#define UI__ROOT_MIN_Y (24)
#define UI__ROOT_MIN_X (80)

/* top margin for menu bar */
#define UI__ROOT_MARGIN_TOP    (1)

/* bottom margin for status bar */
#define UI__ROOT_MARGIN_BOTTOM (1)

#define UI__ROOT_MARGIN_LEFT   (0)
#define UI__ROOT_MARGIN_RIGHT  (0)

const char *ui__status_text = "Ready";

void ui__root_place_content_window(struct ui_window_root *root, struct ui_dims *dims)
{
  memcpy(dims, &root->super.dims, sizeof(struct ui_dims));

  dims->begy += UI__ROOT_MARGIN_TOP;
  dims->maxy -= UI__ROOT_MARGIN_TOP + UI__ROOT_MARGIN_BOTTOM;

  dims->begx += UI__ROOT_MARGIN_LEFT;
  dims->maxx -= UI__ROOT_MARGIN_LEFT + UI__ROOT_MARGIN_RIGHT;
}

void ui__root_draw_menu(struct ui_window_root *root);
void ui__root_draw_status(struct ui_window_root *root);

void ui__root_draw_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = ui__cast(root, base);

  int maxy, maxx;
  getmaxyx(root->cwindow, maxy, maxx);
  if (root->undersize_scr) {
    for (int y = 0; y < maxy; ++y)
      mvwhline(root->cwindow, y, 0, y < 3 ? ' ' : '/', maxx);

    mvwprintw(root->cwindow, 0, 0, "Your terminal is too small! It must be at least %dx%d.", UI__ROOT_MIN_X, UI__ROOT_MIN_Y);

    wrefresh(root->cwindow);
    return;
  }

  ui__root_draw_menu(root);
  ui__root_draw_status(root);

  wnoutrefresh(root->cwindow);
  wnoutrefresh(root->menu_cwindow);

  if (root->content) {
    ui__call_draw_proc(root->content);
  }

  if (root->floating) {
    ui__call_draw_proc(root->floating);
  }

  doupdate();
}

void ui__root_draw_menu(struct ui_window_root *root)
{
  wattron(root->menu_cwindow, A_REVERSE);
  mvwhline(root->menu_cwindow, 0, 0, ' ', getmaxx(root->menu_cwindow));
  mvwaddstr(root->menu_cwindow, 0, 0, " UMPS v0.1.0-dev");

  char *text;
  int idx = 0;
  for (struct uimenu_item_header *item = root->menu_root->head; item; item = item->next, ++idx) {
    waddstr(root->menu_cwindow, "  ");

    switch (item->type) {
      case UMPS__MENU_TYPE_SPACER:
        waddstr(root->menu_cwindow, "--");
        continue;
      case UMPS__MENU_TYPE_BUTTON:
        text = ((struct uimenu_item_button *)item)->text;
        break;
      case UMPS__MENU_TYPE_MENU:
        text = ((struct uimenu_item_menu *)item)->text;
        break;
      default:
        umps_trap;
    }

    if (item == root->menu_selected)
      wattroff(root->menu_cwindow, A_REVERSE);

    if (text) waddstr(root->menu_cwindow, text);
    else waddstr(root->menu_cwindow, "???");

    wattron(root->menu_cwindow, A_REVERSE);
  }

  wattroff(root->menu_cwindow, A_REVERSE);
}

void ui__root_redraw_menu(struct ui_window_root *root)
{
  ui__root_draw_menu(root);
  wnoutrefresh(root->menu_cwindow);
  doupdate();
}

void ui__root_draw_status(struct ui_window_root *root)
{
  WINDOW *mywin = root->cwindow;

  attron(A_REVERSE);
  mvwhline(mywin, getmaxy(mywin)-1, 0, ' ', getmaxx(mywin));

  if (ui__status_text) {
    mvwaddstr(mywin, getmaxy(mywin)-1, 1, ui__status_text);
  }

  attroff(A_REVERSE);
}

void ui__root_layout_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = ui__cast(root, base);

  if (root->cwindow != stdscr) {
    if (root->cwindow) delwin(root->cwindow);
    root->cwindow = newwin(root->super.dims.maxy, root->super.dims.maxx, root->super.dims.begy, root->super.dims.begx);
  }

  delwin(root->menu_cwindow);
  root->menu_cwindow = newwin(1, root->super.dims.maxx, 0, 0);

  if (root->super.dims.maxy < UI__ROOT_MIN_Y || root->super.dims.maxx < UI__ROOT_MIN_X) {
    root->undersize_scr = true;
    return;
  }

  root->undersize_scr = false;

  if (root->content) {
    ui__root_place_content_window(root, &root->content->dims);
    ui__call_layout_proc(root->content);
  }
}

struct ui_window_base *ui__root_control_proc(struct ui_window_base *base, ui_control inp)
{
  struct ui_window_root *root = ui__cast(root, base);

  if (root->undersize_scr) return NULL;

  switch (inp) {
    case NS('h'):
      ui__status_text = "Hello...";
      ui__call_draw_proc(ui__cast(base, ui_root));
      break;
    case NS('w'):
      ui__status_text = "World!";
      ui__call_draw_proc(ui__cast(base, ui_root));
      break;
    case NS('m'):
      if (root->menu_selected) {
        root->menu_selected = NULL;
      } else {
        root->menu_selected = root->menu_root->head;
      }
      ui__root_redraw_menu(root);
      return NULL;
    case KEY_LEFT:
    case KEY_RIGHT:
      if (root->menu_selected) {
        struct uimenu_item_header *nxt = inp == KEY_RIGHT ? root->menu_selected->next : root->menu_selected->prev;
        if (nxt) {
          root->menu_selected = nxt;
          ui__root_redraw_menu(root);
        }
        return NULL;
      }
      break;
  }

  if (root->menu_selected) {
    return NULL;
  } else {
    return root->content;
  }
}

void ui__root_set_content(struct ui_window_root *root, struct ui_window_base *window)
{
  umps_assert(!window->parent);
  umps_assert(!root->content);

  ui__root_place_content_window(root, &window->dims);
  root->content = window;
  window->parent = ui__cast(base, root);

  ui__call_layout_proc(window);
}

void ui__root_set_floating(struct ui_window_root *root, struct ui_window_base *window)
{
  umps_assert(!window->parent);
  umps_assert(!root->floating);

  
}
