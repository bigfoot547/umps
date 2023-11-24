#ifndef UMPS_UIMENU_INTERNAL_H_INCLUDED
#define UMPS_UIMENU_INTERNAL_H_INCLUDED

#include <stdbool.h>

#define UMPS__MENU_TYPE_SPACER (0)
#define UMPS__MENU_TYPE_BUTTON (1)
#define UMPS__MENU_TYPE_MENU   (2)

/* this header is right out of a university data structures slide deck */

struct uimenu_item_header {
  unsigned type;
  struct uimenu_item_header *next, *prev;
};

struct uimenu_item_button;

typedef void (uimenu_button_action)(struct uimenu_item_button *);

struct uimenu_item_button {
  struct uimenu_item_header header;
  unsigned id;
  char *text;

  uimenu_button_action *action;

  bool enabled;
};

struct uimenu_item_menu {
  struct uimenu_item_header header;
  char *text;

  struct uimenu_item_header *head, *tail;
  unsigned nchildren;
};

void uimenu_item_button_init(struct uimenu_item_button *button, unsigned id, const char *text, uimenu_button_action *action);
void uimenu_item_menu_init(struct uimenu_item_menu *menu, const char *text);

/* menu items will be freed on removal */
void uimenu_menu_add_spacer(struct uimenu_item_menu *menu, struct uimenu_item_header *where, bool after);
void uimenu_menu_add(struct uimenu_item_menu *menu, struct uimenu_item_header *where, struct uimenu_item_header *item, bool after);

void uimenu_menu_remove(struct uimenu_item_menu *menu, struct uimenu_item_header *item);

/* frees children */
void uimenu_menu_free(struct uimenu_item_menu *menu);

#endif /* include guard */
