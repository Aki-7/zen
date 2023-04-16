#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_buffer;

struct zn_buffer_interface {
  void *(*begin_access)(struct zn_buffer *self);
  /// @return false if errors occur during the access
  bool (*end_access)(struct zn_buffer *self);

  ssize_t (*get_size)(struct zn_buffer *self);
};

struct zn_buffer {
  void *impl_data;                         // @nullable, @outlive
  const struct zn_buffer_interface *impl;  // @nonnull, @outlive
};

struct zn_buffer *zn_buffer_create(
    void *impl_data, const struct zn_buffer_interface *implementation);

void zn_buffer_destroy(struct zn_buffer *self);

UNUSED static inline void *
zn_buffer_begin_access(struct zn_buffer *self)
{
  return self->impl->begin_access(self);
}

UNUSED static inline bool
zn_buffer_end_access(struct zn_buffer *self)
{
  return self->impl->end_access(self);
}

UNUSED static inline ssize_t
zn_buffer_get_size(struct zn_buffer *self)
{
  return self->impl->get_size(self);
}

#ifdef __cplusplus
}
#endif
