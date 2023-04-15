#include "client-gl-rendering-unit.h"

#include <zwin-gl-protocol.h>

#include "client-virtual-object.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/gl-rendering-unit.h"
#include "zen/xr-dispatcher.h"

static void zn_client_gl_rendering_unit_destroy(
    struct zn_client_gl_rendering_unit *self);

static void
zn_client_gl_rendering_unit_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_rendering_unit *self =
      wl_resource_get_user_data(resource);

  zn_client_gl_rendering_unit_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_rendering_unit_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static const struct zwn_gl_rendering_unit_interface implementation = {
    .destroy = zn_client_gl_rendering_unit_protocol_destroy,
};

static void
zn_client_gl_rendering_unit_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_rendering_unit *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zn_client_gl_rendering_unit_destroy(self);
}

static void
zn_client_gl_rendering_unit_handle_zn_gl_rendering_unit_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_rendering_unit *self =
      zn_container_of(listener, self, zn_gl_rendering_unit_destroy_listener);

  zn_client_gl_rendering_unit_destroy(self);
}

struct zn_client_gl_rendering_unit *
zn_client_gl_rendering_unit_create(struct wl_client *client, uint32_t id,
    struct zn_client_virtual_object *virtual_object)
{
  struct zn_client_gl_rendering_unit *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->virtual_object = virtual_object;

  self->zn_gl_rendering_unit = zn_xr_dispatcher_get_new_gl_rendering_unit(
      virtual_object->dispatcher, virtual_object->zn_virtual_object);
  if (self->zn_gl_rendering_unit == NULL) {
    zn_error("Failed to get new zn_gl_rendering_unit");
    goto err_free;
  }

  self->resource =
      wl_resource_create(client, &zwn_gl_rendering_unit_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_gl_rendering_unit;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_rendering_unit_handle_destroy);

  self->virtual_object_destroy_listener.notify =
      zn_client_gl_rendering_unit_handle_virtual_object_destroy;
  wl_signal_add(
      &virtual_object->events.destroy, &self->virtual_object_destroy_listener);

  self->zn_gl_rendering_unit_destroy_listener.notify =
      zn_client_gl_rendering_unit_handle_zn_gl_rendering_unit_destroy;
  wl_signal_add(&self->zn_gl_rendering_unit->events.destroy,
      &self->zn_gl_rendering_unit_destroy_listener);

  return self;

err_gl_rendering_unit:
  zn_xr_dispatcher_destroy_gl_rendering_unit(
      virtual_object->dispatcher, self->zn_gl_rendering_unit);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_rendering_unit_destroy(struct zn_client_gl_rendering_unit *self)
{
  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->zn_gl_rendering_unit_destroy_listener.link);
  zn_xr_dispatcher_destroy_gl_rendering_unit(
      self->virtual_object->dispatcher, self->zn_gl_rendering_unit);
  free(self);
}
