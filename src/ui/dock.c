#include <stdlib.h>
#include <assert.h>

#include "ui.internal.h"

unsigned ui__dock_position_opposite(unsigned position)
{
  switch (position)
  {
    case UI__WINDOW_DOCK_TOP:
      return UI__WINDOW_DOCK_BOTTOM;
    case UI__WINDOW_DOCK_BOTTOM:
      return UI__WINDOW_DOCK_TOP;
    case UI__WINDOW_DOCK_LEFT:
      return UI__WINDOW_DOCK_RIGHT;
    case UI__WINDOW_DOCK_RIGHT:
      return UI__WINDOW_DOCK_LEFT;
    default:
      assert(false); /* trap: this function should never be called here! */
  }
}

void ui__dock_adjust(struct ui_window_dock *dock, unsigned flags, int *maxy, int *maxx, int *begy, int *begx)
{
  if (flags & 1)
  {
    if (dock->children[UI__WINDOW_DOCK_TOP])
    {
      unsigned top_maxy = getmaxy(dock->children[UI__WINDOW_DOCK_TOP]->cwindow);
      *maxy -= top_maxy;
      *begy += top_maxy;
    }

    if (dock->children[UI__WINDOW_DOCK_BOTTOM])
    {
      unsigned bottom_maxy = getmaxy(dock->children[UI__WINDOW_DOCK_BOTTOM]->cwindow);
      *maxy -= bottom_maxy;
    }
  }

  if (flags & 2)
  {
    if (dock->children[UI__WINDOW_DOCK_LEFT])
    {
      unsigned left_maxx = getmaxx(dock->children[UI__WINDOW_DOCK_LEFT]->cwindow);
      *maxx -= left_maxx;
      *begx += left_maxx;
    }

    if (dock->children[UI__WINDOW_DOCK_RIGHT])
    {
      unsigned right_maxx = getmaxx(dock->children[UI__WINDOW_DOCK_RIGHT]->cwindow);
      *maxx -= right_maxx;
    }
  }
}

/* This is hand-coded to fit with the window priorities, as specified here in ascending order:
 * - Top/bottom
 * - Left/right
 * - Center
 *
 * Note that other methods (layout proc namely) require this same order to be present in the values of the dock constants. */
WINDOW *ui__dock_place_window(struct ui_window_dock *dock, unsigned position, float size)
{
  int maxy, maxx;
  int begy, begx;

  int out_lines, out_cols;
  int out_y, out_x;

  getmaxyx(dock->super.cwindow, maxy, maxx);
  getbegyx(dock->super.cwindow, begy, begx);

  switch (position)
  {
    case UI__WINDOW_DOCK_TOP:
      out_lines = maxy * size;
      out_y = begy;
      out_cols = maxx;
      out_x = begx;
      break;
    case UI__WINDOW_DOCK_BOTTOM:
      out_lines = maxy * size;
      out_y = maxy - out_lines + begy;
      out_cols = maxx;
      out_x = begx;
      break;
    case UI__WINDOW_DOCK_LEFT:
      ui__dock_adjust(dock, 1, &maxy, &maxx, &begy, &begx);
      out_lines = maxy;
      out_y = begy;
      out_cols = maxx * size;
      out_x = begx;
      break;
    case UI__WINDOW_DOCK_RIGHT:
      ui__dock_adjust(dock, 1, &maxy, &maxx, &begy, &begx);
      out_lines = maxy;
      out_y = begy;
      out_cols = maxx * size;
      out_x = maxx - out_cols + begx;
      break;
    case UI__WINDOW_DOCK_CENTER:
      ui__dock_adjust(dock, 3, &maxy, &maxx, &begy, &begx);
      out_lines = maxy;
      out_y = begy;
      out_cols = maxx;
      out_x = begx;
      break;
    default:
      assert(false);
  }

  return newwin(out_lines, out_cols, out_y, out_x);
}

void ui__dock_add_child(struct ui_window_dock *dock, struct ui_window_base *child, unsigned position, float size)
{
  assert(!dock->children[position]); /* TODO: handle gracefully (technically invalid usage) */
  assert(!child->parent); /* TODO: take this window from its current parent */
  assert(!child->cwindow);

  child->parent = (struct ui_window_base *)dock;
  dock->children[position] = child;
  dock->childsizes[position] = size;

  if (position != UI__WINDOW_DOCK_CENTER)
  {
    unsigned opposite = ui__dock_position_opposite(position);
    if (dock->children[opposite] && size + dock->childsizes[opposite] > 1)
    {
      dock->childsizes[position] = 1 - dock->childsizes[opposite];
    }
  }

  /* now set up the window for this child */
  child->cwindow = ui__dock_place_window(dock, position, size);
}

void ui__dock_default_draw_proc(struct ui_window_base *base)
{
  struct ui_window_dock *dock = (struct ui_window_dock *)base;

  wrefresh(dock->super.cwindow);
  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    if (dock->children[i])
      ui__call_draw_proc(dock->children[i]);
  }
}

void ui__dock_default_layout_proc(struct ui_window_base *base)
{
  struct ui_window_dock *dock = (struct ui_window_dock *)base;

  /* fix the layout of children */
  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    struct ui_window_base *child = dock->children[i];
    if (!child) continue;

    delwin(child->cwindow);
    child->cwindow = ui__dock_place_window(dock, i, dock->childsizes[i]);
    ui__call_layout_proc(child);
  }
}

void ui__root_draw_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = (struct ui_window_root *)base;
  ui__dock_default_draw_proc(base);

  if (root->floating) ui__call_draw_proc(root->floating);
}

void ui__root_layout_proc(struct ui_window_base *base)
{
  struct ui_window_root *root = (struct ui_window_root *)base;
  ui__dock_default_layout_proc(base);

  /* TODO: adjust floating window position :) */
  if (root->floating) ui__call_layout_proc(root->floating);
}
