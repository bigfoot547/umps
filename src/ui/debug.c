#ifndef NDEBUG

#include "../macros.h"
#include "ui.internal.h"

#include <stdio.h>

const char *ui__debug_types[] = {
#define WT_DEF(_t) "UI__WINDOW_TYPE_" #_t
  UI__FOREACH_WINDOW_TYPE(WT_DEF)
#undef WT_DEF
};

size_t ui__debug_type_count = sizeof(ui__debug_types) / sizeof(ui__debug_types[0]);

const char *ui__debug_type_to_str(unsigned type)
{
  if (type >= ui__debug_type_count) return "???";
  return ui__debug_types[type];
}

struct ui_window_base *ui__check_cast_to_base(void *obj, const char *file, const char *func, int line)
{
  umps_unused(file);
  umps_unused(func);
  umps_unused(line);
  return obj; /* this cast always succeeds */
}

#define UMPS__DEBUG_DO_ERROR(_o, _fi, _fn, _ln, _t) \
  struct ui_window_base *base = _o;                                             \
  if (base->type != _t) {                                                       \
    fprintf(stderr, "!!!!!\n!!!!!\n!!!!! UMPS UI bad cast at %s:%d %s (expect %s got %s:%u) !!!!!\n!!!!!\n!!!!!\n", \
      _fi, _ln, _fn, #_t, ui__debug_type_to_str(base->type), base->type);       \
    umps_trap;                                                                  \
  }                                                                             \
  return _o;

struct ui_window_leaf *ui__check_cast_to_leaf(void *obj, const char *file, const char *func, int line)
{
  UMPS__DEBUG_DO_ERROR(obj, file, func, line, UI__WINDOW_TYPE_LEAF);
}

struct ui_window_dock *ui__check_cast_to_dock(void *obj, const char *file, const char *func, int line)
{
  UMPS__DEBUG_DO_ERROR(obj, file, func, line, UI__WINDOW_TYPE_DOCK);
}

struct ui_window_root *ui__check_cast_to_root(void *obj, const char *file, const char *func, int line)
{
  UMPS__DEBUG_DO_ERROR(obj, file, func, line, UI__WINDOW_TYPE_ROOT);
}

#else

/* the file must have a declaration */
void umps__debug_do_nothing(void);

#endif
