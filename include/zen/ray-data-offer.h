#pragma once

#include <wayland-server-core.h>
#include <zwin-protocol.h>

struct zn_ray_data_offer {
  struct wl_resource *resource;
  struct zn_data_source *source;  // nonnull

  uint32_t actions;
  enum zwn_data_device_manager_dnd_action preferred_action;
  bool in_ask;

  struct wl_listener source_destroy_listener;
};

void zn_ray_data_offer_update_action(struct zn_ray_data_offer *self);

/**
 * @param source is nonnull
 */
struct zn_ray_data_offer *zn_ray_data_offer_create(
    struct wl_client *client, uint32_t id, struct zn_data_source *source);
