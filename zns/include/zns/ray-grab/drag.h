#pragma once

#include <wayland-server-core.h>

#include "zen/ray.h"

struct zn_ray;
struct zn_data_source;

struct zns_drag_ray_grab {
  struct zn_ray_grab base;

  struct zn_data_source *source;  // nullable
  struct zns_node *focus;         // nonnull
  struct wl_client *client;       // nullable

  struct wl_listener focus_destroy_listener;
  struct wl_listener source_destroy_listener;
};

void zns_drag_ray_grab_start(struct zn_ray *ray, struct zn_data_source *source,
    struct wl_client *client);
