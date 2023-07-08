#pragma once

#include <wayland-server-core.h>

struct zn_ray_data_device {
  struct wl_global *global;
  struct wl_list resource_list;
};

struct zn_ray_data_device *zn_ray_data_device_create(
    struct wl_display *display);

void zn_ray_data_device_destroy(struct zn_ray_data_device *self);
