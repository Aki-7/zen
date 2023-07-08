#include "zen/ray-data-source.h"

#include <zwin-protocol.h>

#include "zen-common.h"
#include "zen/data-source.h"

static void zn_ray_data_source_destroy(struct zn_ray_data_source *self);

static void
zn_ray_data_source_handle_destroy(struct wl_resource *resource)
{
  struct zn_ray_data_source *self = wl_resource_get_user_data(resource);

  zn_ray_data_source_destroy(self);
}

static void
zn_ray_data_source_protocol_offer(struct wl_client *client,
    struct wl_resource *resource, const char *mime_type)
{
  UNUSED(client);
  struct zn_ray_data_source *self = wl_resource_get_user_data(resource);

  zn_data_source_notify_offer(self->zn_data_source, mime_type);
}

static void
zn_ray_data_source_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

static void
zn_ray_data_source_protocol_set_actions(struct wl_client *client,
    struct wl_resource *resource, uint32_t dnd_actions)
{
  UNUSED(client);
  struct zn_ray_data_source *self = wl_resource_get_user_data(resource);

  zn_data_source_notify_set_actions(self->zn_data_source, dnd_actions);
}

static const struct zwn_data_source_interface implementation = {
    .offer = zn_ray_data_source_protocol_offer,
    .destroy = zn_ray_data_source_protocol_destroy,
    .set_actions = zn_ray_data_source_protocol_set_actions,
};

static void
zn_ray_data_source_target(void *impl_data, const char *mime_type)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_target(self->resource, mime_type);
}

static void
zn_ray_data_source_send(void *impl_data, const char *mime_type, int fd)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_send(self->resource, mime_type, fd);
}

static void
zn_ray_data_source_cancelled(void *impl_data)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_cancelled(self->resource);
}

static void
zn_ray_data_source_dnd_drop_performed(void *impl_data)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_dnd_drop_performed(self->resource);
}

static void
zn_ray_data_source_dnd_finished(void *impl_data)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_dnd_finished(self->resource);
}

static void
zn_ray_data_source_action(
    void *impl_data, enum wl_data_device_manager_dnd_action dnd_action)
{
  struct zn_ray_data_source *self = impl_data;

  zwn_data_source_send_action(self->resource, dnd_action);
}

static struct wl_client *
zn_ray_data_source_get_client(void *impl_data)
{
  struct zn_ray_data_source *self = impl_data;

  return wl_resource_get_client(self->resource);
}

static const struct zn_data_source_interface data_source_implementation = {
    .target = zn_ray_data_source_target,
    .send = zn_ray_data_source_send,
    .cancelled = zn_ray_data_source_cancelled,
    .dnd_drop_performed = zn_ray_data_source_dnd_drop_performed,
    .dnd_finished = zn_ray_data_source_dnd_finished,
    .action = zn_ray_data_source_action,
    .get_client = zn_ray_data_source_get_client,
};

struct zn_ray_data_source *
zn_ray_data_source_create(struct wl_client *client, uint32_t id)
{
  struct zn_ray_data_source *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_data_source =
      zn_data_source_create(self, &data_source_implementation);
  if (self->zn_data_source == NULL) {
    zn_error("Failed to create a zn_data_source");
    goto err_free;
  }

  self->resource =
      wl_resource_create(client, &zwn_data_source_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_data_source;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_ray_data_source_handle_destroy);

  return self;

err_data_source:
  zn_data_source_destroy(self->zn_data_source);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_ray_data_source_destroy(struct zn_ray_data_source *self)
{
  zn_data_source_destroy(self->zn_data_source);
  free(self);
}
