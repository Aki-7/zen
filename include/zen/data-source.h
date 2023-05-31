#pragma once

#include <wayland-server-protocol.h>

struct zn_data_source_interface {
  void (*target)(void *impl_data, const char *mime_type);
  void (*send)(void *impl_data, const char *mime_type, int fd);
  void (*cancelled)(void *impl_data);
  void (*dnd_drop_performed)(void *impl_data);
  void (*dnd_finished)(void *impl_data);
  void (*action)(
      void *impl_data, enum wl_data_device_manager_dnd_action dnd_action);
  struct wl_client *(*get_client)(void *impl_data);
};

struct zn_data_source {
  void *impl_data;
  const struct zn_data_source_interface *impl;

  struct wl_array mime_types;
  int32_t actions;

  bool accepted;   // TODO: set value
  bool finalized;  // TODO: set value

  enum wl_data_device_manager_dnd_action current_dnd_action;  // TODO: set value

  struct {
    struct wl_signal destroy;
  } events;
};

void zn_data_source_accept(struct zn_data_source *self, const char *mime_type);

void zn_data_source_send(
    struct zn_data_source *self, const char *mime_type, int fd);

void zn_data_source_cancelled(struct zn_data_source *self);

void zn_data_source_dnd_drop_performed(struct zn_data_source *self);

void zn_data_source_dnd_finished(struct zn_data_source *self);

void zn_data_source_action(struct zn_data_source *self,
    enum wl_data_device_manager_dnd_action dnd_action);

struct wl_client *zn_data_source_get_client(struct zn_data_source *self);

void zn_data_source_notify_offer(
    struct zn_data_source *self, const char *mime_type);

void zn_data_source_notify_set_actions(
    struct zn_data_source *self, uint32_t dnd_actions);

struct zn_data_source *zn_data_source_create(
    void *impl_data, const struct zn_data_source_interface *implementation);

void zn_data_source_destroy(struct zn_data_source *self);
