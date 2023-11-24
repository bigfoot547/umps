#include <string.h>
#include <stdlib.h>

#include "uimenu.internal.h"
#include "../macros.h"

void uimenu_item_button_init(struct uimenu_item_button *button, unsigned id, const char *text, uimenu_button_action *action)
{
  button->header.type = UMPS__MENU_TYPE_BUTTON;
  button->header.next = button->header.prev = NULL;

  button->id = id;
  button->text = text ? strdup(text) : NULL;
  button->enabled = true;
  button->action = action;
}

void uimenu_item_menu_init(struct uimenu_item_menu *menu, const char *text)
{
  menu->header.type = UMPS__MENU_TYPE_MENU;
  menu->header.next = menu->header.prev = NULL;

  menu->text = text ? strdup(text) : NULL;
  menu->head = menu->tail = NULL;
  menu->nchildren = 0;
}

struct uimenu_item_header *uimenu__item_menu_spacer_create(void)
{
  struct uimenu_item_header *spacer = malloc(sizeof(struct uimenu_item_header));
  spacer->type = UMPS__MENU_TYPE_SPACER;
  spacer->next = spacer->prev = NULL;
  return spacer;
}

/* menu items will be freed on removal */
void uimenu_menu_add_spacer(struct uimenu_item_menu *menu, struct uimenu_item_header *where, bool after)
{
  struct uimenu_item_header *spc = uimenu__item_menu_spacer_create();
  uimenu_menu_add(menu, where, spc, after);
}

void uimenu_menu_add(struct uimenu_item_menu *menu, struct uimenu_item_header *where, struct uimenu_item_header *item, bool after)
{
  if (!menu->head) {
    /* edge case: menu has no children :((((( */
    menu->head = menu->tail = item;
    item->next = item->prev = NULL;
    ++menu->nchildren;
    return;
  }

  umps_assert(where);

  if (after) {
    item->prev = where;
    item->next = where->next;
    where->next = item;

    if (!item->next) menu->tail = item;
  } else {
    item->next = where;
    item->prev = where->prev;
    where->prev = item;

    if (!item->prev) menu->head = item;
  }

  ++menu->nchildren;
}

void uimenu__item_free(struct uimenu_item_header *item)
{
  switch (item->type) {
    case UMPS__MENU_TYPE_SPACER:
      free(item);
      break;
    case UMPS__MENU_TYPE_BUTTON: {
      struct uimenu_item_button *btn = (struct uimenu_item_button *)item;
      free(btn->text);
      free(btn);
      break;
    }
    case UMPS__MENU_TYPE_MENU:
      uimenu_menu_free((struct uimenu_item_menu *)item);
      break;
    default:
      umps_trap;
  }
}

void uimenu_menu_remove(struct uimenu_item_menu *menu, struct uimenu_item_header *item)
{
  if (item->prev) {
    item->prev->next = item->next;
  } else {
    menu->head = item->next;
  }

  if (item->next) {
    item->next->prev = item->prev;
  } else {
    menu->tail = item->prev;
  }

  --menu->nchildren;
}

/* frees children */
void uimenu_menu_free(struct uimenu_item_menu *menu)
{
  for (struct uimenu_item_header *cur = menu->head, *next; cur; cur = next) {
    next = cur->next;
    uimenu__item_free(cur);
  }
  free(menu->text);
  free(menu);
}
