#ifndef UMPS_UI_H_INCLUDED
#define UMPS_UI_H_INCLUDED

/* window data types */

#include "config.h"

/* base window type */
struct ui_window_base; /* base window type */
struct ui_window_dock; /* dock window type: has windows docked at the four cardinal directions and center */
struct ui_window_root; /* the root window: a special dock window with a possible list of floating/dialog windows */

void ui_init(void); /* sets up the UI */

void ui_handle(void); /* handles the UI */

extern struct ui_window_root *ui_root;

#endif /* include guard */
