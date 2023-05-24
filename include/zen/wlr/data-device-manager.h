#pragma once

#include <wayland-server-core.h>

struct zn_wlr_data_device_manager {
  struct wl_global *global;
};

struct zn_wlr_data_device_manager *zn_wlr_data_device_manager_create(
    struct wl_display *display);

void zn_wlr_data_device_manager_destroy(
    struct zn_wlr_data_device_manager *self);
