#pragma once

#include "base-unit.h"
#include "zen/board.h"
#include "zns/board.h"

struct zna_board_nameplate_unit {
  struct zna_base_unit *base_unit;
};

void zna_board_nameplate_unit_commit(struct zna_board_nameplate_unit *self,
    struct zn_board *board, struct znr_virtual_object *virtual_object,
    uint32_t damage);

void zna_board_nameplate_unit_setup_renderer_objects(
    struct zna_board_nameplate_unit *self, struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object);

void zna_board_nameplate_unit_teardown_renderer_objects(
    struct zna_board_nameplate_unit *self);

struct zna_board_nameplate_unit *zna_board_nameplate_unit_create(
    struct zna_system *system);

void zna_board_nameplate_unit_destroy(struct zna_board_nameplate_unit *self);
