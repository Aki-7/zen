#pragma once

#include "zen/appearance/system.h"
#include "zen/board.h"

struct zna_board;

enum zna_bounded_damage_flag {
  ZNA_BOARD_DAMAGE_GEOMETRY = 1 << 0,
  ZNA_BOARD_DAMAGE_NAMEPLATE_TEXTURE = 1 << 1,
};

void zna_board_commit(struct zna_board *self, uint32_t damage);

struct zna_board *zna_board_create(
    struct zn_board *board, struct zna_system *system);

void zna_board_destroy(struct zna_board *self);
