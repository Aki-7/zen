#pragma once

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

struct zn_wlr_data_offer {
  struct wl_resource *resource;
  struct zn_data_source *source;  // nonnull

  uint32_t actions;
  enum wl_data_device_manager_dnd_action preferred_action;
  bool in_ask;

  struct wl_listener source_destroy_listener;
};

void zn_wlr_data_offer_update_action(struct zn_wlr_data_offer *self);

/**
 * @param source is nonnull
 */
struct zn_wlr_data_offer *zn_wlr_data_offer_create(
    struct wl_client *client, uint32_t id, struct zn_data_source *source);

void zn_wlr_data_offer_destroy(struct zn_wlr_data_offer *self);
