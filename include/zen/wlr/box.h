#ifndef ZEN_BOX_H
#define ZEN_BOX_H

#include <wlr/util/box.h>

void zn_wlr_fbox_closest_point(const struct wlr_fbox* box, double x, double y,
    double* dest_x, double* dest_y);

#endif  // ZEN_BOX_H