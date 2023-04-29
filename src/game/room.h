#pragma once

#include <array>
#include <flecs.h>

namespace ld53::game {

constexpr int ROOM_WIDTH = 20;
constexpr int ROOM_HEIGHT = 15;
struct Room {
  struct IsDirty {};

  std::array<flecs::entity_t, ROOM_WIDTH * ROOM_HEIGHT> tiles{};

  flecs::entity_t get_tile(int x, int y) const {
    return tiles[x + y * ROOM_WIDTH];
  }
  void set_tile(int x, int y, flecs::entity_t tile) {
    tiles[x + y * ROOM_WIDTH] = tile;
  }
};

struct Rooms {
  struct TestRoom {};
};

void initRoom(flecs::world &ecs);
} // namespace ld53::game
