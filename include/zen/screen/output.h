#pragma once

#include <bits/types/FILE.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>

#include "zen/screen.h"

struct zn_output {
  struct wlr_output *wlr_output;  // nonnull, reference

  /** nonnull, automatically destroyed when wlr_output is destroyed */
  struct wlr_output_damage *damage;

  struct zn_screen *screen;  // nonnull, owning

  struct wl_listener wlr_output_destroy_listener;
  struct wl_listener damage_frame_listener;

  struct wl_event_source *second_timer_source;

  FILE *ffmpeg;
  void *buffer;
};

void zn_output_box_effective_to_transformed_coords(struct zn_output *self,
    struct wlr_fbox *effective, struct wlr_box *transformed);

struct zn_output *zn_output_create(struct wlr_output *wlr_output);
