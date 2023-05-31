#pragma once

#include <wayland-server-core.h>

struct zn_wlr_data_source;

struct zn_wlr_data_device_manager_start_drag_event {
  uint32_t serial;
  struct zn_wlr_data_source *data_source;  // nullable
  struct wl_client *client;                // nonnull
};

struct zn_wlr_data_device_manager {
  struct wl_global *global;

  struct wl_list data_device_list;  // zn_wlr_data_device::link

  struct {
    struct wl_signal request_start_drag;
  } events;
};

void zn_wlr_data_device_manager_notify_request_start_drag(
    struct zn_wlr_data_device_manager *self,
    struct zn_wlr_data_device_manager_start_drag_event *event);

struct zn_wlr_data_device_manager *zn_wlr_data_device_manager_create(
    struct wl_display *display);

void zn_wlr_data_device_manager_destroy(
    struct zn_wlr_data_device_manager *self);
