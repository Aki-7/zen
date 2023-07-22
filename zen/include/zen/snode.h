#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/util/box.h>

#include "zen-common/util.h"

enum zn_snode_flag {
  ZN_SNODE_FLAG_FOCUSABLE = 1 << 0,
};

struct zn_snode_interface {
  /// @return value is nullable
  struct wlr_texture *(*get_texture)(void *user_data);

  void (*frame)(void *user_data, const struct timespec *when);

  /// @return true if `point` is in input region
  /// @param point is in snode-local coords
  bool (*accepts_input)(void *user_data, const vec2 point);

  void (*pointer_button)(void *user_data, uint32_t time_msec, uint32_t button,
      enum wlr_button_state state);

  void (*pointer_enter)(void *user_data, const vec2 point);

  void (*pointer_motion)(void *user_data, uint32_t time_msec, const vec2 point);

  void (*pointer_leave)(void *user_data);

  void (*pointer_axis)(void *user_data, uint32_t time_msec,
      enum wlr_axis_source source, enum wlr_axis_orientation orientation,
      double delta, int32_t delta_discrete);

  void (*pointer_frame)(void *user_data);

  /// Only snode with ZN_SNODE_FLAG_FOCUSABLE flag will receive this event.
  void (*on_focus)(void *user_data, bool focused);
};

extern const struct zn_snode_interface zn_snode_noop_implementation;

struct wlr_texture *zn_snode_noop_get_texture(void *user_data);

void zn_snode_noop_frame(void *user_data, const struct timespec *when);

bool zn_snode_noop_accepts_input(void *user_data, const vec2 point);

void zn_snode_noop_pointer_button(void *user_data, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state);

void zn_snode_noop_pointer_enter(void *user_data, const vec2 point);

void zn_snode_noop_pointer_motion(
    void *user_data, uint32_t time_msec, const vec2 point);

void zn_snode_noop_pointer_leave(void *user_data);

void zn_snode_noop_pointer_axis(void *user_data, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete);

void zn_snode_noop_pointer_frame(void *user_data);

void zn_snode_noop_on_focus(void *user_data, bool focused);

struct zn_screen;

/// screen node
struct zn_snode {
  void *user_data;                        // @nullable, @outlive if exists
  const struct zn_snode_interface *impl;  // @nonnull, @outlive

  struct zn_snode *parent;  // @nullable, @ref
  vec2 position;            // effective coords, relative to parent
  vec2 absolute_position;   // effective coords, relative to root

  uint32_t flags;  // a bitfield of enum zn_snode_flag

  // When the screen is destroyed, the root snode is destroyed and
  // `position_changed` signal handler will set this NULL.
  // For root snode, this is @outlive
  struct zn_screen *screen;  // @nullable, @ref

  struct wl_list child_node_list;  // zn_snode::link, sorted from back to front
  struct wl_list link;             // zn_snode::child_node_list

  struct wl_listener parent_destroy_listener;
  struct wl_listener parent_position_changed_listener;

  struct {
    struct wl_signal position_changed;  // (NULL)
    struct wl_signal destroy;           // (NULL)
  } events;
};

UNUSED static inline void
zn_snode_pointer_button(struct zn_snode *self, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  self->impl->pointer_button(self->user_data, time_msec, button, state);
}

UNUSED static inline void
zn_snode_pointer_enter(struct zn_snode *self, const vec2 point)
{
  self->impl->pointer_enter(self->user_data, point);
}

UNUSED static inline void
zn_snode_pointer_motion(
    struct zn_snode *self, uint32_t time_msec, const vec2 point)
{
  self->impl->pointer_motion(self->user_data, time_msec, point);
}

UNUSED static inline void
zn_snode_pointer_leave(struct zn_snode *self)
{
  self->impl->pointer_leave(self->user_data);
}

UNUSED static inline void
zn_snode_pointer_axis(struct zn_snode *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  self->impl->pointer_axis(
      self->user_data, time_msec, source, orientation, delta, delta_discrete);
}

UNUSED static inline void
zn_snode_pointer_frame(struct zn_snode *self)
{
  self->impl->pointer_frame(self->user_data);
}

void zn_snode_focus(struct zn_snode *self);

void zn_snode_unfocus(struct zn_snode *self);

bool zn_snode_is_focusable(struct zn_snode *self);

/// @return value is nullable. If `self` is focusable, return `self`.
struct zn_snode *zn_snode_get_focusable_parent(struct zn_snode *self);

//
/// @param point is in snode-local coords
/// @param[out] local_point returns the position of the `point` in a coords
/// local to the snode of the return value. If no snode is found, the value
/// remains unchanged.
/// @return value is nullable
///
/// @remarks point and local_point can be the same one.
struct zn_snode *zn_snode_get_snode_at(
    struct zn_snode *self, vec2 point, vec2 local_point);

void zn_snode_damage_whole(struct zn_snode *self);

/// @param damage is in the snode-local coords
void zn_snode_damage(struct zn_snode *self, struct wlr_fbox *damage);

void zn_snode_move_front(struct zn_snode *self);

void zn_snode_notify_frame(struct zn_snode *self, const struct timespec *when);

/// @param parent is nullable
/// @param position is in the parent local coords
/// The node is placed on top.
void zn_snode_set_position(
    struct zn_snode *self, struct zn_snode *parent, vec2 position);

/// @param position is in the parent local coords
/// Only the position changes, the order of the siblings remains the same.
void zn_snode_change_position(struct zn_snode *self, vec2 position);

/// @param sibling must not be NULL
/// self and sibling must have the same direct nonnull parent.
/// self is placed directly above the sibling.
/// Position doesn't change
void zn_snode_place_above(struct zn_snode *self, struct zn_snode *sibling);

/// @param sibling must not be NULL
/// self and sibling must have the same direct nonnull parent.
/// self is placed directly below the sibling.
/// Position doesn't change
void zn_snode_place_below(struct zn_snode *self, struct zn_snode *sibling);

/// @return value is nullable
struct wlr_texture *zn_snode_get_texture(struct zn_snode *self);

/// @param fbox returns the box of `self` relative to the root in the effective
/// coordinate system.
void zn_snode_get_fbox(struct zn_snode *self, struct wlr_fbox *fbox);

/// @param fbox returns the box of `self` in the layout coordinate system.
void zn_snode_get_layout_fbox(struct zn_snode *self, struct wlr_fbox *fbox);

struct zn_snode *zn_snode_create(
    void *user_data, const struct zn_snode_interface *implementation);

struct zn_snode *zn_snode_create_focusable(
    void *user_data, const struct zn_snode_interface *implementation);

struct zn_snode *zn_snode_create_root(struct zn_screen *screen);

void zn_snode_destroy(struct zn_snode *self);
