#ifndef UMPS_UI_INTERNAL_H_INCLUDED
#define UMPS_UI_INTERNAL_H_INCLUDED

#include "../ui.h"
#include "uimenu.internal.h"
#include "config.h"
#include NCURSES_INCLUDE

#include <stdbool.h>

#define UI__WINDOW_DOCK_TOP    (0u)
#define UI__WINDOW_DOCK_BOTTOM (1u)
#define UI__WINDOW_DOCK_LEFT   (2u)
#define UI__WINDOW_DOCK_RIGHT  (3u)
#define UI__WINDOW_DOCK_CENTER (4u)
#define UI__WINDOW_DOCK_MAX UI__WINDOW_DOCK_CENTER+1

#define UI__WINDOW_FOCUS_NONE (999u)

#define UI__FOREACH_WINDOW_TYPE(O) \
  O(BASE), \
  O(LEAF), \
  O(DOCK), \
  O(ROOT)

enum {
#define WT_DEF(_t) UI__WINDOW_TYPE_ ## _t
  UI__FOREACH_WINDOW_TYPE(WT_DEF)
#undef WT_DEF
};

/*#define UI__WINDOW_TYPE_BASE (0u)
#define UI__WINDOW_TYPE_LEAF (1u)
#define UI__WINDOW_TYPE_DOCK (2u)
#define UI__WINDOW_TYPE_ROOT (3u)*/

#ifdef NDEBUG
#define ui__cast(_t, _v) ((struct ui_window_ ## _t *)(_v))
#else
#define ui__cast(_t, _v) (ui__check_cast_to_ ## _t(_v, __FILE__, __func__, __LINE__))

struct ui_window_base *ui__check_cast_to_base(void *, const char *, const char *, int);
struct ui_window_leaf *ui__check_cast_to_leaf(void *, const char *, const char *, int);
struct ui_window_dock *ui__check_cast_to_dock(void *, const char *, const char *, int);
struct ui_window_root *ui__check_cast_to_root(void *, const char *, const char *, int);
#endif

/* concrete type definitions */

/* called to refresh the window (should refresh children as well) */
typedef void (ui__draw_proc)(struct ui_window_base *);

/* called to recalculate the layout of the window (for resize) */
typedef void (ui__layout_proc)(struct ui_window_base *);

#ifdef NCURSES_WIDE
typedef wint_t ui_control;
#else
typedef int ui_control;
#endif

typedef struct ui_window_base *(ui__control_proc)(struct ui_window_base *, ui_control);

struct ui_window_base {
  unsigned type;
  struct ui_window_base *parent; /* the parent of a window manages its memory */
  struct ui_dims {
    int maxy, maxx;
    int begy, begx;
  } dims;

  ui__draw_proc *draw_proc;
  ui__layout_proc *layout_proc;
  ui__control_proc *control_proc;
};

struct ui_window_leaf {
  struct ui_window_base super;

  WINDOW *cwindow;
};

struct ui_window_dock {
  struct ui_window_base super;

  struct ui_window_base *children[UI__WINDOW_DOCK_MAX];
  float childsizes[UI__WINDOW_DOCK_MAX];

  unsigned focus;
};

struct ui_window_root {
  struct ui_window_base super;
  WINDOW *cwindow, *menu_cwindow;

  struct uimenu_item_header *menu_selected;

  bool undersize_scr;

  struct ui_window_base *content;  /* content of the editor */
  struct ui_window_base *floating; /* floating menu layer */
  struct ui_window_base *modal;    /* modal (dialog) layer */

  struct uimenu_item_menu *menu_root;
};

/* internal utils */

/* in-place constructors */
void ui__init_window_base(struct ui_window_base *);
void ui__init_window_leaf(struct ui_window_leaf *);
void ui__init_window_dock(struct ui_window_dock *);
void ui__init_window_root(struct ui_window_root *, WINDOW *);

struct ui_window_base *ui__find_focused(void);

/* callback utils */
void ui__call_draw_proc(struct ui_window_base *);
void ui__call_layout_proc(struct ui_window_base *);

/* destructor */

void ui__destroy_window(struct ui_window_base *);

/* docked window utils */

void ui__dock_add_child(struct ui_window_dock *, struct ui_window_base *, unsigned position, float size);
void ui__dock_default_draw_proc(struct ui_window_base *base);
void ui__dock_default_layout_proc(struct ui_window_base *base);
struct ui_window_base *ui__dock_default_control_proc(struct ui_window_base *base, ui_control inp);

/* root window hooks */
void ui__root_draw_proc(struct ui_window_base *);
void ui__root_layout_proc(struct ui_window_base *);
struct ui_window_base *ui__root_control_proc(struct ui_window_base *base, ui_control inp);

/* root_window_utilities */

void ui__root_redraw_menu(struct ui_window_root *);
void ui__root_set_content(struct ui_window_root *, struct ui_window_base *);

extern const char *ui__status_text;

#endif /* include guard */
