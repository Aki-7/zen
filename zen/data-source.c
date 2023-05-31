#include "zen/data-source.h"

#include "zen-common.h"

#define DATA_DEVICE_ALL_ACTIONS                \
  (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |    \
      WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | \
      WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)

void
zn_data_source_accept(struct zn_data_source *self, const char *mime_type)
{
  self->accepted = (mime_type != NULL);
  self->impl->target(self->impl_data, mime_type);
}

void
zn_data_source_send(struct zn_data_source *self, const char *mime_type, int fd)
{
  self->impl->send(self->impl_data, mime_type, fd);
}

void
zn_data_source_cancelled(struct zn_data_source *self)
{
  self->impl->cancelled(self->impl_data);
}

void
zn_data_source_dnd_drop_performed(struct zn_data_source *self)
{
  self->impl->dnd_drop_performed(self->impl_data);
}

void
zn_data_source_dnd_finished(struct zn_data_source *self)
{
  self->impl->dnd_finished(self->impl_data);
}

void
zn_data_source_action(struct zn_data_source *self,
    enum wl_data_device_manager_dnd_action dnd_action)
{
  self->current_dnd_action = dnd_action;
  self->impl->action(self->impl_data, dnd_action);
}

struct wl_client *
zn_data_source_get_client(struct zn_data_source *self)
{
  return self->impl->get_client(self->impl_data);
}

void
zn_data_source_notify_offer(struct zn_data_source *self, const char *mime_type)
{
  if (self->finalized) {
    zn_debug("Offering MIME type after starting dnd or selection");
  }

  const char **mime_type_ptr;
  wl_array_for_each (mime_type_ptr, &self->mime_types) {
    if (strcmp(*mime_type_ptr, mime_type) == 0) {
      zn_debug("Ignoring duplicated MIME type offer %s", mime_type);
      return;
    }
  }

  char *dup_mime_type = strdup(mime_type);
  if (dup_mime_type == NULL) {
    zn_warn("Failed to duplicate mime type string");
    return;
  }

  char **p = wl_array_add(&self->mime_types, sizeof(*p));
  if (p == NULL) {
    free(dup_mime_type);
    zn_warn("Failed to add item to wl_array");
    return;
  }

  *p = dup_mime_type;
}

void
zn_data_source_notify_set_actions(
    struct zn_data_source *self, uint32_t dnd_actions)
{
  UNUSED(dnd_actions);

  if (self->actions >= 0) {
    zn_warn("Cannot set actions more than once");
    return;
  }

  if (dnd_actions & ~DATA_DEVICE_ALL_ACTIONS) {
    zn_warn("invalid action mask %x", dnd_actions);
    return;
  }

  if (self->finalized) {
    zn_warn("Invalid action change after start_drag or set_selection");
    return;
  }

  self->actions = dnd_actions;
}

struct zn_data_source *
zn_data_source_create(
    void *impl_data, const struct zn_data_source_interface *implementation)
{
  struct zn_data_source *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  wl_array_init(&self->mime_types);
  self->actions = -1;
  self->finalized = false;
  self->accepted = false;
  self->current_dnd_action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_data_source_destroy(struct zn_data_source *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  char **p;
  wl_array_for_each (p, &self->mime_types) {
    free(*p);
  }
  wl_array_release(&self->mime_types);

  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
