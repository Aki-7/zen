#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_virtual_object {
  void *impl_data;  // @nullable, @outlive if exists

  struct {
    struct wl_signal destroy;
  } events;
};

#ifdef __cplusplus
}
#endif
