#include "ui.internal.h"
#include "macros.h"
#include "ui/uimenu.internal.h"
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
  WINDOW *mywin = root->cwindow;
  attron(A_REVERSE);
  mvwhline(mywin, 0, 0, ' ', getmaxx(mywin));
  mvwaddstr(mywin, 0, 0, " UMPS v0.1.0-dev");

  char *text;
  for (struct uimenu_item_header *item = root->menu_root->head; item; item = item->next) {
    waddstr(mywin, "  ");
    switch (item->type) {
      case UMPS__MENU_TYPE_SPACER:
        waddstr(mywin, "--");
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

    if (text) waddstr(mywin, text);
    else waddstr(mywin, "???");
  }

  attroff(A_REVERSE);
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
