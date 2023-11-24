#include <stdlib.h>
#include <assert.h>

#include "macros.h"
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
      umps_trap; /* trap: the center dock has no opposite! (or position is invalid) */
  }
}

void ui__dock_adjust(struct ui_window_dock *dock, unsigned flags, struct ui_dims *dims)
{
  if (flags & 1)
  {
    if (dock->children[UI__WINDOW_DOCK_TOP])
    {
      unsigned top_maxy = dock->children[UI__WINDOW_DOCK_TOP]->dims.maxy;
      dims->maxy -= top_maxy;
      dims->begy += top_maxy;
    }

    if (dock->children[UI__WINDOW_DOCK_BOTTOM])
    {
      unsigned bottom_maxy = dock->children[UI__WINDOW_DOCK_BOTTOM]->dims.maxy;
      dims->maxy -= bottom_maxy;
    }
  }

  if (flags & 2)
  {
    if (dock->children[UI__WINDOW_DOCK_LEFT])
    {
      unsigned left_maxx = dock->children[UI__WINDOW_DOCK_LEFT]->dims.maxx;
      dims->maxx -= left_maxx;
      dims->begx += left_maxx;
    }

    if (dock->children[UI__WINDOW_DOCK_RIGHT])
    {
      unsigned right_maxx = dock->children[UI__WINDOW_DOCK_RIGHT]->dims.maxx;
      dims->maxx -= right_maxx;
    }
  }
}

/* This is hand-coded to fit with the window priorities, as specified here in ascending order:
 * - Top/bottom
 * - Left/right
 * - Center
 *
 * Note that other methods (layout proc namely) require this same order to be present in the values of the dock constants. */
void ui__dock_place_window(struct ui_window_dock *dock, unsigned position, float size, struct ui_dims *dims)
{
  struct ui_dims mydims = dock->super.dims;

  switch (position)
  {
    case UI__WINDOW_DOCK_TOP:
      dims->maxy = mydims.maxy * size;
      dims->begy = mydims.begy;
      dims->maxx = mydims.maxx;
      dims->begx = mydims.begx;
      break;
    case UI__WINDOW_DOCK_BOTTOM:
      dims->maxy = mydims.maxy * size;
      dims->begy = mydims.maxy - dims->maxy + mydims.begy;
      dims->maxx = mydims.maxx;
      dims->begx = mydims.begx;
      break;
    case UI__WINDOW_DOCK_LEFT:
      ui__dock_adjust(dock, 1, &mydims);
      dims->maxy = mydims.maxy;
      dims->begy = mydims.begy;
      dims->maxx = mydims.maxx * size;
      dims->begx = mydims.begx;
      break;
    case UI__WINDOW_DOCK_RIGHT:
      ui__dock_adjust(dock, 1, &mydims);
      dims->maxy = mydims.maxy;
      dims->begy = mydims.begy;
      dims->maxx = mydims.maxx * size;
      dims->begx = mydims.maxx - dims->maxx + mydims.begx;
      break;
    case UI__WINDOW_DOCK_CENTER:
      ui__dock_adjust(dock, 3, &mydims);
      dims->maxy = mydims.maxy;
      dims->begy = mydims.begy;
      dims->maxx = mydims.maxx;
      dims->begx = mydims.begx;
      break;
    default:
      umps_trap;
  }
}

void ui__dock_add_child(struct ui_window_dock *dock, struct ui_window_base *child, unsigned position, float size)
{
  umps_assert_s(!dock->children[position], "child present"); /* TODO: handle gracefully (technically invalid usage) */
  umps_assert_s(!child->parent, "child parent present"); /* TODO: take this window from its current parent */

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
  ui__dock_place_window(dock, position, size, &child->dims);
  ui__call_layout_proc(child);
}

void ui__dock_default_draw_proc(struct ui_window_base *base)
{
  struct ui_window_dock *dock = ui__cast(dock, base);
  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    if (dock->children[i])
      ui__call_draw_proc(dock->children[i]);
  }
}

void ui__dock_default_layout_proc(struct ui_window_base *base)
{
  struct ui_window_dock *dock = ui__cast(dock, base);

  /* fix the layout of children */
  for (unsigned i = 0; i < UI__WINDOW_DOCK_MAX; ++i)
  {
    struct ui_window_base *child = dock->children[i];
    if (!child) continue;

    ui__dock_place_window(dock, i, dock->childsizes[i], &child->dims);
    ui__call_layout_proc(child);
  }
}
