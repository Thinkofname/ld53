#pragma once

#include <array>
#include <flecs.h>
#include <vector>

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

struct RoomObjects {
  // I'm not even going to pretend this is a good way of doing this
  std::array<std::vector<flecs::entity_t>, ROOM_WIDTH * ROOM_HEIGHT> objects{};

  const std::vector<flecs::entity_t> &get_objects(int x, int y) const {
    return objects[x + y * ROOM_WIDTH];
  }
  std::vector<flecs::entity_t> &get_objects(int x, int y) {
    return objects[x + y * ROOM_WIDTH];
  }
};

struct Rooms {
  struct TestRoom {};
};

void initRoom(flecs::world &ecs);
} // namespace ld53::game