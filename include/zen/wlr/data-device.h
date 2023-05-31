#pragma once

#include <wayland-server-core.h>

struct zn_wlr_data_device_manager;

struct zn_wlr_data_device {
  struct wl_resource *resource;
  struct wl_list link;  // zn_wlr_data_device_manager::data_device_list
  struct zn_wlr_data_device_manager *manager;
};

struct zn_wlr_data_device *zn_wlr_data_device_create(struct wl_client *client,
    uint32_t id, struct zn_wlr_data_device_manager *manager);
