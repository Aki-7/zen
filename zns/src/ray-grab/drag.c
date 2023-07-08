#include "zns/ray-grab/drag.h"

#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <stdlib.h>

#include "zen-common.h"
#include "zen/appearance/ray.h"
#include "zen/cursor.h"
#include "zen/data-source.h"
#include "zen/server.h"
#include "zns/shell.h"

static void zns_drag_ray_grab_destroy(struct zns_drag_ray_grab *self);

static void
zns_drag_ray_grab_set_focus(
    struct zns_drag_ray_grab *self, struct zns_node *node)
{
  UNUSED(self);
  UNUSED(node);

  if (self->focus == node) {
    return;
  }

  // TODO: Check that the next focus is valid

  if (self->focus) {
    wl_list_remove(&self->focus_destroy_listener.link);
    wl_list_init(&self->focus_destroy_listener.link);

    zns_node_data_device_leave(self->focus);
  }

  self->focus = node;

  if (node) {
    wl_signal_add(&node->events.destroy, &self->focus_destroy_listener);

    self->source->accepted = false;

    zns_node_data_device_enter(self->focus, self->base.ray->origin,
        self->base.ray->direction, self->source);
  }
}

static void
zns_drag_ray_grab_motion_relative(struct zn_ray_grab *grab_base, vec3 origin,
    float polar, float azimuthal, uint32_t time_msec)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zns_drag_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zns_node *node;

  float next_polar = self->base.ray->angle.polar + polar;
  if (next_polar < 0)
    next_polar = 0;
  else if (next_polar > M_PI)
    next_polar = M_PI;

  float next_azimuthal = self->base.ray->angle.azimuthal + azimuthal;
  while (next_azimuthal >= 2 * M_PI) next_azimuthal -= 2 * M_PI;
  while (next_azimuthal < 0) next_azimuthal += 2 * M_PI;

  vec3 next_origin;
  glm_vec3_add(self->base.ray->origin, origin, next_origin);

  zn_ray_move(self->base.ray, next_origin, next_polar, next_azimuthal);

  float distance = FLT_MAX;
  mat4 identity = GLM_MAT4_IDENTITY_INIT;

  node = zns_node_ray_cast(server->shell->root, self->base.ray->origin,
      self->base.ray->direction, identity, &distance);

  zn_ray_set_length(self->base.ray, node ? distance : DEFAULT_RAY_LENGTH);

  zna_ray_commit(self->base.ray->appearance);

  zns_drag_ray_grab_set_focus(self, node);

  if (self->focus) {
    zns_node_data_device_motion(self->focus, self->base.ray->origin,
        self->base.ray->direction, time_msec);
  }
}

static void
zns_drag_ray_grab_button(struct zn_ray_grab *grab_base, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);

  struct zns_drag_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_server *server = zn_server_get_singleton();

  if (self->source != NULL && state == WLR_BUTTON_RELEASED) {
    if (self->focus) {
      zns_node_data_device_drop(self->focus);
    }

    zn_data_source_dnd_drop_performed(self->source);

    zns_drag_ray_grab_set_focus(self, NULL);
  }

  if (state == WLR_BUTTON_RELEASED) {  // TODO: counter pressing pointer button
    zn_cursor_end_grab(server->scene->cursor);
    zn_ray_end_grab(grab_base->ray);
  }
}

static void
zns_drag_ray_grab_axis(struct zn_ray_grab *grab_base, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab_base);
  UNUSED(time_msec);
  UNUSED(source);
  UNUSED(orientation);
  UNUSED(delta);
  UNUSED(delta_discrete);
}

static void
zns_drag_ray_grab_frame(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
}

static void
zns_drag_ray_grab_rebase(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
}

static void
zns_drag_ray_grab_cancel(struct zn_ray_grab *grab_base)
{
  struct zns_drag_ray_grab *self = zn_container_of(grab_base, self, base);
  zns_drag_ray_grab_destroy(self);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_drag_ray_grab_motion_relative,
    .button = zns_drag_ray_grab_button,
    .axis = zns_drag_ray_grab_axis,
    .frame = zns_drag_ray_grab_frame,
    .rebase = zns_drag_ray_grab_rebase,
    .cancel = zns_drag_ray_grab_cancel,
};

static void
zns_drag_ray_grab_handle_focus_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_drag_ray_grab *self =
      zn_container_of(listener, self, focus_destroy_listener);

  zns_drag_ray_grab_set_focus(self, NULL);
}

static void
zns_drag_ray_grab_handle_source_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_drag_ray_grab *self =
      zn_container_of(listener, self, source_destroy_listener);
  zn_ray_end_grab(self->base.ray);
}

static struct zns_drag_ray_grab *
zns_drag_ray_grab_create(
    struct zn_data_source *source, struct wl_client *client)
{
  struct zns_drag_ray_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;
  self->source = source;
  self->client = client;
  self->focus = NULL;

  self->focus_destroy_listener.notify = zns_drag_ray_grab_handle_focus_destroy;
  wl_list_init(&self->focus_destroy_listener.link);

  self->source_destroy_listener.notify =
      zns_drag_ray_grab_handle_source_destroy;

  if (source == NULL) {
    wl_signal_add(&source->events.destroy, &self->source_destroy_listener);
  } else {
    wl_list_init(&self->source_destroy_listener.link);
  }

  return self;

err:
  return NULL;
}

static void
zns_drag_ray_grab_destroy(struct zns_drag_ray_grab *self)
{
  wl_list_remove(&self->source_destroy_listener.link);
  wl_list_remove(&self->focus_destroy_listener.link);
  free(self);
}

void
zns_drag_ray_grab_start(
    struct zn_ray *ray, struct zn_data_source *source, struct wl_client *client)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zns_drag_ray_grab *self = zns_drag_ray_grab_create(source, client);

  if (self == NULL) {
    return;
  }

  zn_shell_ray_clear_focus(server->shell);
  zn_ray_start_grab(ray, &self->base);
}
