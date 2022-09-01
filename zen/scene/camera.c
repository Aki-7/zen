#include "zen/scene/camera.h"

#include "zen-common.h"

struct zn_camera *
zn_camera_create(GLuint framebuffer_id)
{
  struct zn_camera *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->framebuffer_id = framebuffer_id;

  return self;

err:
  return NULL;
}

void
zn_camera_destroy(struct zn_camera *self)
{
  free(self);
}
