#ifndef NDEBUG

#include <assert.h>

#include "ui.internal.h"

struct ui_window_base *ui__check_cast_to_base(void *obj)
{
  return obj; /* this cast always succeeds */
}

struct ui_window_dock *ui__check_cast_to_dock(void *obj)
{
  struct ui_window_base *base = obj;
  assert(base->type == UI__WINDOW_TYPE_DOCK);
  return obj;
}

struct ui_window_root *ui__check_cast_to_root(void *obj)
{
  struct ui_window_base *base = obj;
  assert(base->type == UI__WINDOW_TYPE_ROOT);
  return obj;
}

#endif
