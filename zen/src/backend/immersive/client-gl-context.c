#include "client-gl-context.h"

#include <wayland-server.h>
#include <zwin-gl-protocol.h>

#include "client-gl-base-technique.h"
#include "client-gl-buffer.h"
#include "client-gl-program.h"
#include "client-gl-rendering-unit.h"
#include "client-gl-shader.h"
#include "client-gl-texture.h"
#include "client-gl-vertex-array.h"
#include "client-virtual-object.h"
#include "shm-buffer.h"
#include "xr-compositor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

static void
zn_client_gl_context_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_context *self = zn_client_gl_context_get(resource);

  zn_client_gl_context_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_rendering_unit(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *virtual_object_resource)
{
  if (zn_client_gl_context_get(resource) == NULL) {
    return;
  }

  struct zn_client_virtual_object *client_virtual_object =
      zn_client_virtual_object_get(virtual_object_resource);

  zn_client_gl_rendering_unit_create(client, id, client_virtual_object);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_buffer(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  zn_client_gl_buffer_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_shader(struct wl_client *client,
    struct wl_resource *resource UNUSED, uint32_t id,
    struct wl_resource *buffer_resource, uint32_t type)
{
  struct zn_shm_buffer *shm_buffer = zn_shm_buffer_get(buffer_resource);

  zn_client_gl_shader_create(client, id, shm_buffer->zn_buffer, type);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_program(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  zn_client_gl_program_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_texture(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  zn_client_gl_texture_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_sampler(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t id UNUSED)
{}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_vertex_array(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  zn_client_gl_vertex_array_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_context_protocol_create_gl_base_technique(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *rendering_unit_resource)
{
  if (zn_client_gl_context_get(resource) == NULL) {
    return;
  }

  struct zn_client_gl_rendering_unit *rendering_unit =
      zn_client_gl_rendering_unit_get(rendering_unit_resource);

  zn_client_gl_base_technique_create(client, id, rendering_unit);
}

static const struct zwn_gl_context_interface implementation = {
    .destroy = zn_client_gl_context_protocol_destroy,
    .create_gl_rendering_unit =
        zn_client_gl_context_protocol_create_gl_rendering_unit,
    .create_gl_buffer = zn_client_gl_context_protocol_create_gl_buffer,
    .create_gl_shader = zn_client_gl_context_protocol_create_gl_shader,
    .create_gl_program = zn_client_gl_context_protocol_create_gl_program,
    .create_gl_texture = zn_client_gl_context_protocol_create_gl_texture,
    .create_gl_sampler = zn_client_gl_context_protocol_create_gl_sampler,
    .create_gl_vertex_array =
        zn_client_gl_context_protocol_create_gl_vertex_array,
    .create_gl_base_technique =
        zn_client_gl_context_protocol_create_gl_base_technique,
};

struct zn_client_gl_context *
zn_client_gl_context_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

static void
zn_client_gl_context_handle_xr_compositor_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_context *self =
      zn_container_of(listener, self, xr_compositor_destroy_listener);

  zn_client_gl_context_destroy(self);
}

struct zn_client_gl_context *
zn_client_gl_context_create(struct wl_client *client, uint32_t id,
    struct zn_xr_compositor *xr_compositor)
{
  struct zn_client_gl_context *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xr_compositor = xr_compositor;

  self->resource = wl_resource_create(client, &zwn_gl_context_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_context_handle_destroy);

  self->xr_compositor_destroy_listener.notify =
      zn_client_gl_context_handle_xr_compositor_destroy;
  wl_signal_add(&self->xr_compositor->events.destroy,
      &self->xr_compositor_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_client_gl_context_destroy(struct zn_client_gl_context *self)
{
  zwn_gl_context_send_destroyed(self->resource);

  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->xr_compositor_destroy_listener.link);
  free(self);
}
