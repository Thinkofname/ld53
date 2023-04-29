#include "room.h"

#include <array>
#include <unordered_map>

#include "assets.h"
#include "game/common.h"
#include "web/render.h"

namespace ld53::game {

/*
      "#^^^^^^^^^^^^^^^^^^#"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#                  #"
      "#vvvvvvvvvvvvvvvvvv#",
      {{' ', grass}, {'v', top}, {'#', both}, {'^', bottom}}
*/

template <class T>
flecs::entity
makeRoom(flecs::world &ecs, const char *mapData,
         std::unordered_map<unsigned char, flecs::entity_t> tiles) {
  flecs::entity e = ecs.prefab<T>();
  auto room = e.emplace_override<Position>(0, 0)
                  .add<render::DependsOn, assets::Tileset>()
                  .override<render::DependsOn, assets::Tileset>()
                  .get_mut<Room>();
  auto stone = ecs.id<ld53::assets::Tileset::GrassWithStone>();
  auto grass = ecs.id<ld53::assets::Tileset::Grass>();
  for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; i++) {
    room->tiles[i] = tiles[mapData[i]];
    if (room->tiles[i] == grass && rand() % 150 == 0) {
      room->tiles[i] = stone;
    }
  }
  return e;
}

void initRoom(flecs::world &ecs) {
  ecs.component<RoomObjects>().add(EcsAlwaysOverride);
  ecs.component<Room>().add_second<RoomObjects>(flecs::With);

  auto grass = ecs.id<ld53::assets::Tileset::Grass>();
  auto top = ecs.id<ld53::assets::Tileset::TreeTop>();
  auto bottom = ecs.id<ld53::assets::Tileset::TreeBottom>();
  auto both = ecs.id<ld53::assets::Tileset::TreeBoth>();
  auto wall = ecs.id<ld53::assets::Tileset::Wall>();
  auto wallBottom = ecs.id<ld53::assets::Tileset::WallBottom>();
  auto wireTR = ecs.id<ld53::assets::Tileset::WireTR>();
  auto wireLR = ecs.id<ld53::assets::Tileset::WireLR>();

  auto room = makeRoom<Rooms::TestRoom>(ecs,
                                        "#^^^^^^^^^^^^^^^^^^#"
                                        "#                  #"
                                        "#   WWW            #"
                                        "#   WBW            #"
                                        "#   W W            #"
                                        "#   W W            #"
                                        "#   B B            #"
                                        "#    L-----        #"
                                        "#                  #"
                                        "#                  #"
                                        "#                  #"
                                        "#                  #"
                                        "#                  #"
                                        "#                  #"
                                        "#vvvvvvvvvvvvvvvvvv#",
                                        {{' ', grass},
                                         {'v', top},
                                         {'#', both},
                                         {'^', bottom},
                                         {'B', wallBottom},
                                         {'W', wall},
                                         {'L', wireTR},
                                         {'-', wireLR}})
                  .with(flecs::ChildOf, [&]() {
                    ecs.entity("MailTest")
                        .add<MailObject>()
                        .emplace<GridPosition>(10, 9)
                        .add<render::Image, assets::Tileset::Mail>();
                    ecs.entity()
                        .emplace<GridPosition>(5, 3)
                        .add<render::Image, assets::Tileset::Mailbox>();
                    ecs.entity()
                        .emplace<GridPosition>(11, 7)
                        .add<render::Image, assets::Tileset::ButtonPlate>();
                    ecs.entity()
                        .emplace<GridPosition>(14, 7)
                        .add<render::Image, assets::Tileset::Box>();
                    ecs.entity()
                        .emplace<GridPosition>(5, 6)
                        .add<render::Image, assets::Tileset::Gate>()
                        .add(TileType::Solid);
                  });
}
} // namespace ld53::game
