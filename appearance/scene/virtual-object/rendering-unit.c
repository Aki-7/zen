#include "rendering-unit.h"

#include <zen-common.h>

static void zn_rendering_unit_destroy(struct zn_rendering_unit* self);

static void
zn_rendering_unit_handle_zgnr_rendering_unit_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_rendering_unit* self =
      zn_container_of(listener, self, zgnr_rendering_unit_destroy_listener);

  zn_rendering_unit_destroy(self);
}

struct zn_rendering_unit*
zn_rendering_unit_create(struct zgnr_rendering_unit* zgnr_rendering_unit)
{
  struct zn_rendering_unit* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_rendering_unit = zgnr_rendering_unit;

  self->zgnr_rendering_unit_destroy_listener.notify =
      zn_rendering_unit_handle_zgnr_rendering_unit_destroy;
  wl_signal_add(&self->zgnr_rendering_unit->events.destroy,
      &self->zgnr_rendering_unit_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_rendering_unit_destroy(struct zn_rendering_unit* self)
{
  wl_list_remove(&self->zgnr_rendering_unit_destroy_listener.link);
  free(self);
}
