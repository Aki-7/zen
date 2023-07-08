#include "zen/ray-data-offer.h"

#include <unistd.h>

#include "zen-common.h"
#include "zen/data-source.h"

#define DATA_DEVICE_ALL_ACTIONS                 \
  (ZWN_DATA_DEVICE_MANAGER_DND_ACTION_COPY |    \
      ZWN_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | \
      ZWN_DATA_DEVICE_MANAGER_DND_ACTION_ASK)

static void zn_ray_data_offer_destroy(struct zn_ray_data_offer *self);

static uint32_t
zn_ray_data_offer_choose_action(struct zn_ray_data_offer *self)
{
  uint32_t offer_actions = self->actions;
  uint32_t preferred_action = self->preferred_action;

  uint32_t source_actions;
  if (self->source->actions >= 0) {
    source_actions = self->source->actions;
  } else {
    source_actions = ZWN_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
  }

  uint32_t available_actions = offer_actions & source_actions;
  if (!available_actions) {
    return ZWN_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  }

  if ((preferred_action & available_actions) != 0) {
    return preferred_action;
  }

  return 1 << (ffs(available_actions) - 1);
}

void
zn_ray_data_offer_update_action(struct zn_ray_data_offer *self)
{
  uint32_t action = zn_ray_data_offer_choose_action(self);
  if (self->source->current_dnd_action == action) {
    return;
  }

  self->source->current_dnd_action = action;

  if (self->in_ask) {
    return;
  }

  zn_data_source_action(self->source, action);

  zwn_data_offer_send_action(self->resource, action);
}

static void
zn_ray_data_offer_handle_destroy(struct wl_resource *resource)
{
  struct zn_ray_data_offer *self = wl_resource_get_user_data(resource);

  zn_ray_data_offer_destroy(self);
}

/**
 * @param resource can be inert (resource->user_data == NULL)
 */
static void
zn_ray_data_offer_protocol_accept(struct wl_client *client,
    struct wl_resource *resource, uint32_t serial, const char *mime_type)
{
  UNUSED(client);
  UNUSED(serial);

  struct zn_ray_data_offer *self = wl_resource_get_user_data(resource);
  if (self == NULL) {
    return;
  }

  zn_data_source_accept(self->source, mime_type);
}

/**
 * @param resource can be inert (resource->user_data == NULL)
 */
static void
zn_ray_data_offer_protocol_receive(struct wl_client *client,
    struct wl_resource *resource, const char *mime_type, int32_t fd)
{
  UNUSED(client);

  struct zn_ray_data_offer *self = wl_resource_get_user_data(resource);
  if (self == NULL) {
    close(fd);
    return;
  }

  zn_data_source_send(self->source, mime_type, fd);

  close(fd);
}

/**
 * @param resource can be inert (resource->user_data == NULL)
 */
static void
zn_ray_data_offer_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zn_ray_data_offer_source_dnd_finish(struct zn_ray_data_offer *self)
{
  if (self->source->actions < 0) {
    return;
  }

  if (self->in_ask) {
    zn_data_source_action(self->source, self->source->current_dnd_action);
  }

  zn_data_source_dnd_finished(self->source);
}

/**
 * @param resource can be inert (resource->user_data == NULL)
 */
static void
zn_ray_data_offer_protocol_finish(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);

  struct zn_ray_data_offer *self = wl_resource_get_user_data(resource);
  if (self == NULL) {
    return;
  }

  if (!self->source->accepted) {
    zn_warn("Premature finish request");
    return;
  }

  uint32_t action = self->source->current_dnd_action;
  if (action == ZWN_DATA_DEVICE_MANAGER_DND_ACTION_NONE ||
      action == ZWN_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
    zn_warn("Offer finished with an invalid action");
    return;
  }

  zn_ray_data_offer_source_dnd_finish(self);
}

/**
 * @param resource can be inert (resource->user_data == NULL)
 */
static void
zn_ray_data_offer_protocol_set_actions(struct wl_client *client,
    struct wl_resource *resource, uint32_t dnd_actions,
    uint32_t preferred_action)
{
  UNUSED(client);

  struct zn_ray_data_offer *self = wl_resource_get_user_data(resource);
  if (self == NULL) {
    return;
  }

  if (dnd_actions & ~DATA_DEVICE_ALL_ACTIONS) {
    zn_warn("invalid action mask %x", dnd_actions);
    return;
  }

  if (preferred_action && (!(preferred_action & dnd_actions) ||
                              __builtin_popcount(preferred_action) > 1)) {
    zn_warn("invalid action %x", preferred_action);
    return;
  }

  self->actions = dnd_actions;
  self->preferred_action = preferred_action;

  zn_ray_data_offer_update_action(self);
}

static const struct zwn_data_offer_interface implementation = {
    .accept = zn_ray_data_offer_protocol_accept,
    .receive = zn_ray_data_offer_protocol_receive,
    .destroy = zn_ray_data_offer_protocol_destroy,
    .finish = zn_ray_data_offer_protocol_finish,
    .set_actions = zn_ray_data_offer_protocol_set_actions,
};

static void
zn_ray_data_offer_handle_source_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_ray_data_offer *self =
      zn_container_of(listener, self, source_destroy_listener);

  zn_ray_data_offer_destroy(self);
}

struct zn_ray_data_offer *
zn_ray_data_offer_create(
    struct wl_client *client, uint32_t id, struct zn_data_source *source)
{
  struct zn_ray_data_offer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->source = source;

  self->resource = wl_resource_create(client, &zwn_data_offer_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zn_ray_data_offer_handle_destroy);

  self->source_destroy_listener.notify =
      zn_ray_data_offer_handle_source_destroy;
  wl_signal_add(&source->events.destroy, &self->source_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_ray_data_offer_destroy(struct zn_ray_data_offer *self)
{
  wl_list_remove(&self->source_destroy_listener.link);
  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  free(self);
}
