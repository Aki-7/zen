#pragma once

#include <wayland-server-core.h>

struct zn_xr_system_manager;

struct zn_xr {
  struct zn_xr_system_manager *xr_system_manager;  // @nonnull, @owning
};

struct zn_xr *zn_xr_create(struct wl_display *display);

void zn_xr_destroy(struct zn_xr *self);
