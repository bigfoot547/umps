#include "ui.internal.h"
#include "macros.h"
#include <curses.h>

#define UI__ROOT_MIN_Y (24)
#define UI__ROOT_MIN_X (80)

/* top margin for menu bar */
#define UI__ROOT_MARGIN_TOP    (1)

/* bottom margin for status bar */
#define UI__ROOT_MARGIN_BOTTOM (1)

#define UI__ROOT_MARGIN_LEFT   (0)
#define UI__ROOT_MARGIN_RIGHT  (0)

const char *ui__status_text = "Ready";

WINDOW *ui__root_place_content_window(struct ui_window_root *root)
{
  int maxy, maxx;
  int begy, begx;
  getmaxyx(root->super.cwindow, maxy, maxx);
  getbegyx(root->super.cwindow, begy, begx);

  begy += UI__ROOT_MARGIN_TOP;
  maxy -= UI__ROOT_MARGIN_TOP + UI__ROOT_MARGIN_BOTTOM;

  begx += UI__ROOT_MARGIN_LEFT;
  begx -= UI__ROOT_MARGIN_LEFT + UI__ROOT_MARGIN_RIGHT;

  return newwin(maxy, maxx, begy, begx);
}

void ui__root_draw_menu(struct ui_window_root *root);
void ui__root_draw_status(struct ui_window_root *root);

void ui__root_draw_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = ui__cast(root, base);

  int maxy, maxx;
  getmaxyx(base->cwindow, maxy, maxx);
  if (root->undersize_scr) {
    for (int y = 0; y < maxy; ++y)
      mvwhline(base->cwindow, y, 0, y < 3 ? ' ' : '/', maxx);

    mvwprintw(base->cwindow, 0, 0, "Your terminal is too small! It must be at least %dx%d.", UI__ROOT_MIN_X, UI__ROOT_MIN_Y);

    wrefresh(base->cwindow);
    return;
  }

  ui__root_draw_menu(root);
  ui__root_draw_status(root);

  wnoutrefresh(base->cwindow);

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
  WINDOW *mywin = root->super.cwindow;
  attron(A_REVERSE);
  mvwhline(mywin, 0, 0, ' ', getmaxx(mywin));
  mvwaddstr(mywin, 0, 0, " UMPS v0.1.0-dev  File  Edit  Filters  Window  Help");
  attroff(A_REVERSE);
}

void ui__root_draw_status(struct ui_window_root *root)
{
  WINDOW *mywin = root->super.cwindow;

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

  int maxy, maxx;
  getmaxyx(base->cwindow, maxy, maxx);

  if (maxy < UI__ROOT_MIN_Y || maxx < UI__ROOT_MIN_X) {
    root->undersize_scr = true;
    return;
  }

  root->undersize_scr = false;

  if (root->content) {
    delwin(root->content->cwindow);
    root->content->cwindow = ui__root_place_content_window(root);
    ui__call_layout_proc(root->content);
  }
}

void ui__root_set_content(struct ui_window_root *root, struct ui_window_base *window)
{
  umps_assert(!window->parent);
  umps_assert(!window->cwindow);
  umps_assert(!root->content);

  window->cwindow = ui__root_place_content_window(root);
  root->content = window;
  window->parent = ui__cast(base, root);
}

void ui__root_set_floating(struct ui_window_root *root, struct ui_window_base *window)
{
  umps_assert(!window->parent);
  umps_assert(!window->cwindow);
  umps_assert(!root->floating);

  
}
