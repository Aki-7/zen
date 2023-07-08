#include "zen/ray-data-device.h"

#include <zwin-protocol.h>

#include "zen-common.h"

static void
zn_ray_data_device_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zn_ray_data_device_protocol_set_length(struct wl_client *client,
    struct wl_resource *resource, uint32_t serial, wl_fixed_t length)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(serial);
  UNUSED(length);
}

static void
zn_ray_data_device_protocol_start_drag(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *source,
    struct wl_resource *origin, struct wl_resource *icon, uint32_t serial)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(source);
  UNUSED(origin);
  UNUSED(icon);
  UNUSED(serial);
}

static void
zn_ray_data_device_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zwn_data_device_interface implementation = {
    .set_length = zn_ray_data_device_protocol_set_length,
    .start_drag = zn_ray_data_device_protocol_start_drag,
    .release = zn_ray_data_device_protocol_release,
};

static void
zn_ray_data_device_protocol_manager_create_data_source(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zn_ray_data_device_protocol_manager_get_data_device(struct wl_client *client,
    struct wl_resource *resource, uint32_t id, struct wl_resource *seat)
{
  UNUSED(seat);

  struct zn_ray_data_device *self = wl_resource_get_user_data(resource);

  struct wl_resource *data_device_resource =
      wl_resource_create(client, &zwn_data_device_interface, 1, id);
  if (data_device_resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(data_device_resource, &implementation, self,
      zn_ray_data_device_handle_destroy);

  wl_list_insert(
      &self->resource_list, wl_resource_get_link(data_device_resource));
}

static const struct zwn_data_device_manager_interface manager_implementation = {
    .create_data_source =
        zn_ray_data_device_protocol_manager_create_data_source,
    .get_data_device = zn_ray_data_device_protocol_manager_get_data_device,
};

static void
zn_ray_data_device_bind_manager(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_ray_data_device *self = data;

  struct wl_resource *resource = wl_resource_create(
      client, &zwn_data_device_manager_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &manager_implementation, self, NULL);
}

struct zn_ray_data_device *
zn_ray_data_device_create(struct wl_display *display)
{
  struct zn_ray_data_device *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global = wl_global_create(display, &zwn_data_device_manager_interface,
      1, self, zn_ray_data_device_bind_manager);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_data_device_manager wl_global");
    goto err_free;
  }

  wl_list_init(&self->resource_list);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ray_data_device_destroy(struct zn_ray_data_device *self)
{
  wl_list_remove(&self->resource_list);
  wl_global_destroy(self->global);
  free(self);
}
