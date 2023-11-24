#ifndef NDEBUG

#include "../macros.h"
#include "ui.internal.h"

struct ui_window_base *ui__check_cast_to_base(void *obj)
{
  return obj; /* this cast always succeeds */
}

struct ui_window_leaf *ui__check_cast_to_leaf(void *obj)
{
  struct ui_window_base *base = obj;
  umps_assert(base->type == UI__WINDOW_TYPE_LEAF);
  return obj;
}

struct ui_window_dock *ui__check_cast_to_dock(void *obj)
{
  struct ui_window_base *base = obj;
  umps_assert(base->type == UI__WINDOW_TYPE_DOCK);
  return obj;
}

struct ui_window_root *ui__check_cast_to_root(void *obj)
{
  struct ui_window_base *base = obj;
  umps_assert(base->type == UI__WINDOW_TYPE_ROOT);
  return obj;
}

#else

/* the file must have a declaration */
void umps__debug_do_nothing(void)
{
}

#endif
