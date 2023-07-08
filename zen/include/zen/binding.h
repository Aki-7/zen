#pragma once

#include <wlr/types/wlr_keyboard.h>

#include "zen/config.h"

/// @return true if the key is used and should not be used any more
typedef bool (*zn_binding_handler_t)(
    const char *name, uint32_t key, uint32_t modifiers, void *user_data);

struct zn_binding_item {
  const char *name;  // @nonnull, @outlive
  uint32_t key;
  uint32_t modifiers;
  zn_binding_handler_t handler;  // nonnull
  void *user_data;
};

struct zn_binding {
  struct wl_array items;  // [zn_binding_item]
};

/// @param name must be a string literal
void zn_binding_add(struct zn_binding *self, const char *name,
    zn_binding_handler_t handler, void *user_data);

/// @return true if the key is used and should not be used any more
/// @param modifiers is a bitfield of enum wlr_keyboard_modifier
bool zn_binding_handle_key(
    struct zn_binding *self, uint32_t key, uint32_t modifiers);

void zn_binding_remap(struct zn_binding *self);

struct zn_binding *zn_binding_create(void);

void zn_binding_destroy(struct zn_binding *self);
