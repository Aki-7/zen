#pragma once

#include "zen/cursor.h"

struct zn_data_source;

struct zn_drag_cursor_grab {
  struct zn_cursor_grab base;
  struct zn_data_source *source;  // nullable
  struct wl_client *client;       // nonnull
  struct wlr_surface *focus;      // nullable

  struct wl_listener focus_destroy_listener;
  struct wl_listener source_destroy_listener;
};

void zn_drag_cursor_grab_start(struct zn_cursor *cursor,
    struct zn_data_source *source, struct wl_client *client);
