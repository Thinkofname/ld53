#include "room.h"

#include <array>

#include "assets.h"
#include "game/common.h"

namespace ld53::game {

void initRoom(flecs::world &ecs) {
  ecs.component<Room>();
  auto room = ecs.entity("TestRoom").emplace<Position>(0, 0).get_mut<Room>();
  room->tiles.fill(ecs.id<ld53::assets::Tileset::Grass>());
  room->tiles[42] = ecs.id<ld53::assets::Tileset::GrassWithStone>();

  auto top = ecs.id<ld53::assets::Tileset::TreeTop>();
  auto bottom = ecs.id<ld53::assets::Tileset::TreeBottom>();
  auto both = ecs.id<ld53::assets::Tileset::TreeBoth>();

  // Sides
  for (int y = 0; y < ROOM_HEIGHT; y++) {
    room->set_tile(0, y, both);
    room->set_tile(ROOM_WIDTH - 1, y, both);
  }
  for (int x = 1; x < ROOM_WIDTH - 1; x++) {
    room->set_tile(x, 0, bottom);
    room->set_tile(x, ROOM_HEIGHT - 1, top);
  }
}
} // namespace ld53::game
