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
    assert(x >= 0 && x < ROOM_WIDTH);
    assert(y >= 0 && y < ROOM_HEIGHT);
    return objects[x + y * ROOM_WIDTH];
  }
  std::vector<flecs::entity_t> &get_objects(int x, int y) {
    assert(x >= 0 && x < ROOM_WIDTH);
    assert(y >= 0 && y < ROOM_HEIGHT);
    return objects[x + y * ROOM_WIDTH];
  }
};

struct Rooms {
  struct Level1 {};
  struct Level2 {};
  struct Level3 {};
  struct Level4 {};
  struct Level5 {};
  struct EndingScreen {};
};
using InitialRoom = Rooms::Level1;

struct RoomInstances {};
struct CurrentRoomType {};
struct CurrentRoom {};

struct NextRoom {};
struct ChangeRoom {};

void initRoom(flecs::world &ecs);
} // namespace ld53::game
