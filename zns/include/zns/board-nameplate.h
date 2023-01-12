#pragma once

#include <cglm/types.h>

#include "node.h"

struct zns_board;

/**
 * The life time of given zns_board must be longer than this object
 */
struct zns_board_nameplate {
  struct zns_board *board;

  struct zns_node *node;

  struct {
    float width;
    float height;
  } geometry;
};

/**
 * returns transformation matrix relative to the world space
 */
void zns_board_nameplate_get_transform(
    struct zns_board_nameplate *self, mat4 transform);

struct zns_board_nameplate *zns_board_nameplate_create(struct zns_board *board);

void zns_board_nameplate_destroy(struct zns_board_nameplate *self);
