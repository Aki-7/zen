#include "zen/wlr/data-device.h"

#include <wayland-server-protocol.h>

#include "zen-common.h"
#include "zen/wlr/data-device-manager.h"

static void zn_wlr_data_device_destroy(struct zn_wlr_data_device *self);

static void
zn_wlr_data_device_handle_destroy(struct wl_resource *resource)
{
  struct zn_wlr_data_device *self = wl_resource_get_user_data(resource);

  zn_wlr_data_device_destroy(self);
}

static void
zn_wlr_data_device_protocol_start_drag(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *source_resource,
    struct wl_resource *origin, struct wl_resource *icon, uint32_t serial)
{
  UNUSED(client);
  UNUSED(origin);
  UNUSED(icon);

  struct zn_wlr_data_device *self = wl_resource_get_user_data(resource);
  struct zn_wlr_data_source *source = NULL;
  if (source_resource != NULL) {
    source = wl_resource_get_user_data(source_resource);
  }

  struct zn_wlr_data_device_manager_start_drag_event event = {
      .serial = serial,
      .data_source = source,
      .client = client,
  };

  zn_wlr_data_device_manager_notify_request_start_drag(self->manager, &event);
}

static void
zn_wlr_data_device_protocol_set_selection(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *source, uint32_t serial)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(source);
  UNUSED(serial);
}

static void
zn_wlr_data_device_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct wl_data_device_interface implementation = {
    .start_drag = zn_wlr_data_device_protocol_start_drag,
    .set_selection = zn_wlr_data_device_protocol_set_selection,
    .release = zn_wlr_data_device_protocol_release,
};

struct zn_wlr_data_device *
zn_wlr_data_device_create(struct wl_client *client, uint32_t id,
    struct zn_wlr_data_device_manager *manager)
{
  struct zn_wlr_data_device *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->manager = manager;

  self->resource = wl_resource_create(client, &wl_data_device_interface, 3, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_wlr_data_device_handle_destroy);

  wl_list_init(&self->link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_wlr_data_device_destroy(struct zn_wlr_data_device *self)
{
  wl_list_remove(&self->link);
  free(self);
}
