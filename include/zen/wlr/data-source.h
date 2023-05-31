#pragma once

#include <wayland-server-core.h>

struct zn_wlr_data_source {
  struct wl_resource *resource;
  struct zn_data_source *zn_data_source;
};

struct zn_wlr_data_source *zn_wlr_data_source_create(
    struct wl_client *client, uint32_t id);
