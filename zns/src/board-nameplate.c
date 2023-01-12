#include "zns/board-nameplate.h"

#include <cglm/affine.h>
#include <zen-common.h>
#include <zgnr/intersection.h>

#include "zen/server.h"
#include "zns/board.h"
#include "zns/shell.h"

static float
zns_board_nameplate_ray_cast(struct zns_board_nameplate *self, vec3 origin,
    vec3 direction, float *u, float *v)
{
  vec3 v0 = {-self->geometry.width / 2.f, 0, 0};
  vec3 v1 = {+self->geometry.width / 2.f, 0, 0};
  vec3 v2 = {-self->geometry.width / 2.f, -self->geometry.height, 0};

  mat4 transform;
  zns_board_nameplate_get_transform(self, transform);

  glm_mat4_mulv3(transform, v0, 1, v0);
  glm_mat4_mulv3(transform, v1, 1, v1);
  glm_mat4_mulv3(transform, v2, 1, v2);

  return zgnr_intersection_ray_parallelogram(
      origin, direction, v0, v1, v2, u, v, false);
}

static bool
zns_board_nameplate_node_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_board_nameplate *self = user_data;

  float u, v;
  float d = zns_board_nameplate_ray_cast(self, origin, direction, &u, &v);

  if (d >= *distance) return false;

  *distance = d;

  return true;
}

static bool
zns_board_nameplate_node_ray_motion(
    void *user_data, vec3 origin, vec3 direction, uint32_t time_msec)
{
  UNUSED(user_data);
  UNUSED(origin);
  UNUSED(direction);
  UNUSED(time_msec);
  return true;
}

static bool
zns_board_nameplate_node_ray_enter(void *user_data, vec3 origin, vec3 direction)
{
  UNUSED(user_data);
  UNUSED(origin);
  UNUSED(direction);
  return true;
}

static bool
zns_board_nameplate_node_ray_leave(void *user_data)
{
  UNUSED(user_data);
  return true;
}

static bool
zns_board_nameplate_node_ray_button(void *user_data, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(user_data);
  UNUSED(time_msec);
  UNUSED(button);
  UNUSED(state);
  return true;
}

static bool
zns_board_nameplate_node_ray_axis(void *user_data, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(user_data);
  UNUSED(time_msec);
  UNUSED(source);
  UNUSED(orientation);
  UNUSED(delta);
  UNUSED(delta_discrete);
  return true;
}

static bool
zns_board_nameplate_node_ray_frame(void *user_data)
{
  UNUSED(user_data);
  return true;
}

static const struct zns_node_interface node_implementation = {
    .ray_cast = zns_board_nameplate_node_ray_cast,
    .ray_motion = zns_board_nameplate_node_ray_motion,
    .ray_enter = zns_board_nameplate_node_ray_enter,
    .ray_leave = zns_board_nameplate_node_ray_leave,
    .ray_button = zns_board_nameplate_node_ray_button,
    .ray_axis = zns_board_nameplate_node_ray_axis,
    .ray_frame = zns_board_nameplate_node_ray_frame,
};

void
zns_board_nameplate_get_transform(
    struct zns_board_nameplate *self, mat4 transform)
{
  glm_mat4_copy(self->board->zn_board->geometry.transform, transform);
  glm_translate_y(transform, -0.01);
}

struct zns_board_nameplate *
zns_board_nameplate_create(struct zns_board *board)
{
  struct zns_board_nameplate *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->node = zns_node_create(server->shell->root, self, &node_implementation);
  if (self->node == NULL) {
    zn_error("Failed to create zns_node");
    goto err_free;
  }

  self->board = board;
  self->geometry.width = 0.4;
  self->geometry.height = 0.04;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zns_board_nameplate_destroy(struct zns_board_nameplate *self)
{
  zns_node_destroy(self->node);
  free(self);
}
