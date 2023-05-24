#include "zen/wlr/data-device-manager.h"

#include <wayland-server-protocol.h>

#include "zen-common.h"

static void
zn_wlr_data_device_manager_protocol_create_data_source(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zn_wlr_data_device_manager_protocol_get_data_device(struct wl_client *client,
    struct wl_resource *resource, uint32_t id, struct wl_resource *seat)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
  UNUSED(seat);
}

static const struct wl_data_device_manager_interface implementation = {
    .create_data_source =
        zn_wlr_data_device_manager_protocol_create_data_source,
    .get_data_device = zn_wlr_data_device_manager_protocol_get_data_device,
};

static void
zn_wlr_data_device_manager_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_wlr_data_device_manager *self = data;

  struct wl_resource *resource = wl_resource_create(
      client, &wl_data_device_manager_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zn_wlr_data_device_manager *
zn_wlr_data_device_manager_create(struct wl_display *display)
{
  struct zn_wlr_data_device_manager *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global = wl_global_create(display, &wl_data_device_manager_interface, 1,
      self, zn_wlr_data_device_manager_bind);
  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_wlr_data_device_manager_destroy(struct zn_wlr_data_device_manager *self)
{
  wl_global_destroy(self->global);
  free(self);
}
