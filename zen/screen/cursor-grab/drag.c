#include "zen/screen/cursor-grab/drag.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"
#include "zen/board.h"
#include "zen/data-source.h"
#include "zen/server.h"
#include "zen/wlr/data-device-manager.h"
#include "zen/wlr/data-device.h"
#include "zen/wlr/data-offer.h"

static void zn_drag_cursor_grab_destroy(struct zn_drag_cursor_grab *self);

static void
zn_drag_cursor_grab_set_focus(struct zn_drag_cursor_grab *self,
    struct wlr_surface *surface /*nullable*/, double sx, double sy)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_seat *seat = server->input_manager->seat;

  struct wlr_surface *next_focus = surface;
  if (surface && self->source == NULL &&
      wl_resource_get_client(surface->resource) != self->client) {
    next_focus = NULL;
  }

  if (self->focus == next_focus) {
    return;
  }

  if (self->focus) {
    wl_list_remove(&self->focus_destroy_listener.link);
    wl_list_init(&self->focus_destroy_listener.link);

    struct zn_wlr_data_device *data_device;
    wl_list_for_each (
        data_device, &seat->wlr_data_device_manager->data_device_list, link) {
      if (wl_resource_get_client(data_device->resource) !=
          wl_resource_get_client(self->focus->resource)) {
        continue;
      }

      wl_data_device_send_leave(data_device->resource);
    }
  }

  self->focus = next_focus;

  if (next_focus) {
    struct wl_client *focus_client =
        wl_resource_get_client(next_focus->resource);
    wl_signal_add(&next_focus->events.destroy, &self->focus_destroy_listener);

    self->source->accepted = false;

    uint32_t serial = wl_display_next_serial(server->display);

    struct zn_wlr_data_device *data_device;
    wl_list_for_each (
        data_device, &seat->wlr_data_device_manager->data_device_list, link) {
      if (wl_resource_get_client(data_device->resource) != focus_client) {
        continue;
      }

      if (self->source) {
        struct zn_wlr_data_offer *offer =
            zn_wlr_data_offer_create(focus_client, 0, self->source);

        wl_data_device_send_data_offer(data_device->resource, offer->resource);

        char **p;
        wl_array_for_each (p, &self->source->mime_types) {
          wl_data_offer_send_offer(offer->resource, *p);
        }

        zn_wlr_data_offer_update_action(offer);

        wl_data_offer_send_source_actions(
            offer->resource, self->source->actions);

        wl_data_device_send_enter(data_device->resource, serial,
            next_focus->resource, wl_fixed_from_double(sx),
            wl_fixed_from_double(sy), offer->resource);
      }
    }
  }
}

static void
zn_drag_cursor_grab_send_movement(
    struct zn_drag_cursor_grab *self, uint32_t time_msec, bool no_motion)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_seat *seat = server->input_manager->seat;
  struct zn_cursor *cursor = self->base.cursor;

  if (!cursor->board) {
    return;
  }

  double surface_x, surface_y;
  struct wlr_surface *surface = zn_board_get_surface_at(
      cursor->board, cursor->x, cursor->y, &surface_x, &surface_y, NULL);

  zn_drag_cursor_grab_set_focus(self, surface, surface_x, surface_y);

  if (self->focus && !no_motion) {
    struct zn_wlr_data_device *data_device;
    wl_list_for_each (
        data_device, &seat->wlr_data_device_manager->data_device_list, link) {
      if (wl_resource_get_client(data_device->resource) !=
          wl_resource_get_client(self->focus->resource)) {
        continue;
      }

      wl_data_device_send_motion(data_device->resource, time_msec,
          wl_fixed_from_double(surface_x), wl_fixed_from_double(surface_y));
    }
  }
}

static void
zn_drag_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  struct zn_drag_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_move_relative(grab->cursor, dx, dy);

  zn_drag_cursor_grab_send_movement(self, time_msec, false);

  zn_cursor_commit_appearance(grab->cursor);
}

static void
zn_drag_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  struct zn_drag_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_move(grab->cursor, board, x, y);

  zn_drag_cursor_grab_send_movement(self, time_msec, false);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_drag_cursor_grab_drop(struct zn_drag_cursor_grab *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_seat *seat = server->input_manager->seat;

  struct zn_wlr_data_device *data_device;
  struct wl_client *focus_client = NULL;

  if (self->focus) {
    focus_client = wl_resource_get_client(self->focus->resource);
  }

  wl_list_for_each (
      data_device, &seat->wlr_data_device_manager->data_device_list, link) {
    if (focus_client != wl_resource_get_client(data_device->resource)) {
      continue;
    }

    wl_data_device_send_drop(data_device->resource);
  }

  zn_drag_cursor_grab_set_focus(self, NULL, 0, 0);
}

static void
zn_drag_cursor_grab_button(struct zn_cursor_grab *grab, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);

  struct zn_drag_cursor_grab *self = zn_container_of(grab, self, base);

  if (self->source != NULL && state == WLR_BUTTON_RELEASED) {
    zn_drag_cursor_grab_drop(self);

    zn_data_source_dnd_drop_performed(self->source);
  }

  if (state == WLR_BUTTON_RELEASED) {  // TODO: counter pressing cursor button
    zn_cursor_end_grab(grab->cursor);
  }
}

static void
zn_drag_cursor_grab_axis(struct zn_cursor_grab *grab, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab);
  UNUSED(time_msec);
  UNUSED(source);
  UNUSED(orientation);
  UNUSED(delta);
  UNUSED(delta_discrete);
}

static void
zn_drag_cursor_grab_frame(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

static void
zn_drag_cursor_grab_enter(
    struct zn_cursor_grab *grab, struct zn_board *board, double x, double y)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zn_cursor_commit_appearance(grab->cursor);
}

static void
zn_drag_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  zn_cursor_move(grab->cursor, NULL, 0, 0);

  zn_cursor_commit_appearance(grab->cursor);
}

static void
zn_drag_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  struct zn_drag_cursor_grab *self = zn_container_of(grab, self, base);

  zn_drag_cursor_grab_send_movement(self, 0, true);

  zn_cursor_commit_appearance(grab->cursor);
}

static void
zn_drag_cursor_grab_cancel(struct zn_cursor_grab *grab)
{
  struct zn_drag_cursor_grab *self = zn_container_of(grab, self, base);
  zn_drag_cursor_grab_destroy(self);
}

const struct zn_cursor_grab_interface zn_drag_cursor_grab_implementation = {
    .motion_relative = zn_drag_cursor_grab_motion_relative,
    .motion_absolute = zn_drag_cursor_grab_motion_absolute,
    .button = zn_drag_cursor_grab_button,
    .axis = zn_drag_cursor_grab_axis,
    .frame = zn_drag_cursor_grab_frame,
    .enter = zn_drag_cursor_grab_enter,
    .leave = zn_drag_cursor_grab_leave,
    .rebase = zn_drag_cursor_grab_rebase,
    .cancel = zn_drag_cursor_grab_cancel,
};

static void
zn_drag_cursor_grab_handle_focus_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_drag_cursor_grab *self =
      zn_container_of(listener, self, focus_destroy_listener);

  zn_drag_cursor_grab_set_focus(self, NULL, 0, 0);
}

static void
zn_drag_cursor_grab_handle_source_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_drag_cursor_grab *self =
      zn_container_of(listener, self, source_destroy_listener);
  zn_cursor_end_grab(self->base.cursor);
}

static struct zn_drag_cursor_grab *
zn_drag_cursor_grab_create(
    struct zn_data_source *source, struct wl_client *client)
{
  struct zn_drag_cursor_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &zn_drag_cursor_grab_implementation;
  self->source = source;
  self->client = client;
  self->focus = NULL;

  self->focus_destroy_listener.notify =
      zn_drag_cursor_grab_handle_focus_destroy;
  wl_list_init(&self->focus_destroy_listener.link);

  self->source_destroy_listener.notify =
      zn_drag_cursor_grab_handle_source_destroy;

  if (source != NULL) {
    wl_signal_add(&source->events.destroy, &self->source_destroy_listener);
  } else {
    wl_list_init(&self->source_destroy_listener.link);
  }

  return self;

err:
  return NULL;
}

static void
zn_drag_cursor_grab_destroy(struct zn_drag_cursor_grab *self)
{
  wl_list_remove(&self->source_destroy_listener.link);
  wl_list_remove(&self->focus_destroy_listener.link);
  free(self);
}

void
zn_drag_cursor_grab_start(struct zn_cursor *cursor,
    struct zn_data_source *source, struct wl_client *client)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_drag_cursor_grab *self = zn_drag_cursor_grab_create(source, client);
  if (self == NULL) {
    zn_error("Failed to create drag cursor grab");
    return;
  }

  wlr_seat_pointer_clear_focus(server->input_manager->seat->wlr_seat);
  zn_cursor_start_grab(cursor, &self->base);
}
