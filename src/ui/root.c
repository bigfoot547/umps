#include <assert.h>

#include "ui.internal.h"

/* top margin for menu bar */
#define UI__ROOT_MARGIN_TOP    (1)

/* bottom margin for status bar */
#define UI__ROOT_MARGIN_BOTTOM (1)

#define UI__ROOT_MARGIN_LEFT   (0)
#define UI__ROOT_MARGIN_RIGHT  (0)

const char *ui__status_text = NULL;

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

void ui__root_draw_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = ui__cast(root, base);

  attron(A_REVERSE);
  mvwhline(base->cwindow, 0, 0, ' ', getmaxx(base->cwindow));
  mvwhline(base->cwindow, getmaxy(base->cwindow)-1, 0, ' ', getmaxx(base->cwindow));

  if (ui__status_text) {
    mvwprintw(base->cwindow, getmaxy(base->cwindow)-1, 1, "Status: %s", ui__status_text);
  }

  attroff(A_REVERSE);

  wnoutrefresh(base->cwindow);

  if (root->content) {
    ui__call_draw_proc(root->content);
  }

  if (root->floating) {
    ui__call_draw_proc(root->floating);
  }

  doupdate();
}

void ui__root_layout_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = ui__cast(root, base);

  if (root->content) {
    delwin(root->content->cwindow);
    root->content->cwindow = ui__root_place_content_window(root);
    ui__call_layout_proc(root->content);
  }
}

void ui__root_set_content(struct ui_window_root *root, struct ui_window_base *window)
{
  assert(!window->parent);
  assert(!window->cwindow);
  assert(!root->content);

  window->cwindow = ui__root_place_content_window(root);
  root->content = window;
  window->parent = ui__cast(base, root);
}
