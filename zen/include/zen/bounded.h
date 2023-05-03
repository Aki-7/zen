#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_virtual_object;

struct zn_bounded {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_virtual_object *virtual_object;  // @nonnull, @outlive

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener commit_listener;

  struct {
    uint32_t serial;
    vec3 half_size;
    struct wl_event_source *idle;  // @nullable, @owning
  } scheduled_config;

  bool added;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};
