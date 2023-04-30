#include "room.h"

#include <array>
#include <unordered_map>

#include "assets.h"
#include "game/common.h"
#include "game/player.h"
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

struct Prefab {
  struct Mailbox {};
  struct Mail {};
  struct Box {};
  struct Gate {};
  struct ButtonPlate {};
};

void initRoom(flecs::world &ecs) {
  ecs.component<RoomObjects>().add(EcsAlwaysOverride);
  ecs.component<Room>().add_second<RoomObjects>(flecs::With);
  ecs.component<NextRoom>().add(flecs::Exclusive);
  ecs.component<ChangeRoom>().add(flecs::Exclusive);

  auto grass = ecs.id<ld53::assets::Tileset::Grass>();
  auto top = ecs.id<ld53::assets::Tileset::TreeTop>();
  auto bottom = ecs.id<ld53::assets::Tileset::TreeBottom>();
  auto both = ecs.id<ld53::assets::Tileset::TreeBoth>();
  auto wall = ecs.id<ld53::assets::Tileset::Wall>();
  auto wallBottom = ecs.id<ld53::assets::Tileset::WallBottom>();
  auto wireTR = ecs.id<ld53::assets::Tileset::WireTR>();
  auto wireLR = ecs.id<ld53::assets::Tileset::WireLR>();
  auto wireTB = ecs.id<ld53::assets::Tileset::WireTB>();
  auto wireTL = ecs.id<ld53::assets::Tileset::WireTL>();
  auto wireBR = ecs.id<ld53::assets::Tileset::WireBR>();

  ecs.prefab<Prefab::Mailbox>()
      .add<render::Image, assets::Tileset::Mailbox>()
      // TODO: Work out why this is needed?
      .override<render::Image, assets::Tileset::Mailbox>()
      .add<MailBox>()
      .add(TileType::SolidPlayer);
  ecs.prefab<Prefab::Mail>()
      .add<MailObject>()
      .add<render::Image, assets::Tileset::Mail>()
      // TODO: Work out why this is needed?
      .override<render::Image, assets::Tileset::Mail>()
      .add<Weighted>();
  ecs.prefab<Prefab::Box>()
      .add<render::Image, assets::Tileset::Box>()
      // TODO: Work out why this is needed?
      .override<render::Image, assets::Tileset::Box>()
      .add(TileType::Solid)
      .add<Pushable>()
      .add<Weighted>()
      .add<render::Depth, render::Depth::Movable>()
      .override<render::Depth, render::Depth::Movable>();
  ecs.prefab<Prefab::Gate>()
      .add<render::Image, assets::Tileset::Gate>()
      .add<Gate>()
      .add(TileType::Solid);
  ecs.prefab<Prefab::ButtonPlate>()
      .add<render::Image, assets::Tileset::ButtonPlate>()
      .add<WeightActivated>()
      .add<render::Depth, render::Depth::Background>()
      .override<render::Depth, render::Depth::Background>();

  makeRoom<Rooms::Level3>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#                  #"
                          "#   WWW   WWW      #"
                          "#   WBW =-WBW      #"
                          "#   W W | W W      #"
                          "#   W W | W W      #"
                          "#   B B | B B      #"
                          "#    L--/  |       #"
                          "#          |       #"
                          "#                  #"
                          "#                  #"
                          "#      WWWW        #"
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
                           {'-', wireLR},
                           {'|', wireTB},
                           {'/', wireTL},
                           {'=', wireBR}})
      .with(flecs::ChildOf, [&]() {});
  makeRoom<Rooms::Level2>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#                  #"
                          "#   WWW   WWW      #"
                          "#   WBW =-WBW      #"
                          "#   W W | W W      #"
                          "#   W W | W W      #"
                          "#   B B | B B      #"
                          "#    L--/  |       #"
                          "#          |       #"
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
                           {'-', wireLR},
                           {'|', wireTB},
                           {'/', wireTL},
                           {'=', wireBR}})
      .with(
          flecs::ChildOf,
          [&]() {
            ecs.entity().emplace<GridPosition>(10, 9).is_a<Prefab::Mail>();
            ecs.entity().emplace<GridPosition>(5, 3).is_a<Prefab::Mailbox>();
            ecs.entity().emplace<GridPosition>(14, 8).is_a<Prefab::Box>();
            auto gate1 =
                ecs.entity().emplace<GridPosition>(5, 6).is_a<Prefab::Gate>();
            auto gate2 =
                ecs.entity().emplace<GridPosition>(11, 6).is_a<Prefab::Gate>();
            ecs.entity()
                .emplace<GridPosition>(11, 9)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate2);
            ecs.entity()
                .emplace<GridPosition>(11, 3)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate1);
          })
      .add<NextRoom, Rooms::Level3>();

  makeRoom<Rooms::Level1>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#                  #"
                          "#   WWWWWWWWW      #"
                          "#   WBBBBBBBW      #"
                          "#   W       W      #"
                          "#   WWWWWWW W      #"
                          "#   BBBBBBB|B      #"
                          "#         =/       #"
                          "#         |        #"
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
                           {'-', wireLR},
                           {'|', wireTB},
                           {'/', wireTL},
                           {'=', wireBR}})
      .with(
          flecs::ChildOf,
          [&]() {
            ecs.entity().emplace<GridPosition>(5, 4).is_a<Prefab::Mail>();
            ecs.entity().emplace<GridPosition>(4, 8).is_a<Prefab::Mailbox>();

            ecs.entity().emplace<GridPosition>(11, 9).is_a<Prefab::Box>();
            auto gate1 =
                ecs.entity().emplace<GridPosition>(11, 5).is_a<Prefab::Gate>();
            ecs.entity()
                .emplace<GridPosition>(10, 9)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate1);

            ecs.entity()
                .emplace<Position>(0, 0)
                .add<render::Image, assets::Tutorial>()
                .add<render::Depth, render::Depth::Background>();
          })
      .add<NextRoom, Rooms::Level2>();

  ecs.system<>("changeOnComplete")
      .with<Room>()
      .with<NextRoom>(flecs::Wildcard)
      .each([](flecs::entity e) {
        auto numToFill = e.world()
                             .filter_builder<>()
                             .with<MailBox>()
                             .without<MailBox::Full>()
                             .not_()
                             .build()
                             .count();
        if (numToFill != 0)
          return;
        e.world().add<ChangeRoom>(e.target<NextRoom>());
      });

  ecs.system<>("changeRoom")
      .with<ChangeRoom>(flecs::Wildcard)
      .each([](flecs::entity e) {
        auto ecs = e.world();
        auto nextRoom = e.target<ChangeRoom>();
        e.remove<ChangeRoom>(flecs::Wildcard);

        auto room = ecs.entity().is_a(nextRoom).child_of<RoomInstances>();
        auto prev = ecs.singleton<CurrentRoom>().target<CurrentRoom>();
        ecs.add<CurrentRoom>(room);

        e.world()
            .entity<Player>()
            .set<Position>({18 * 16, 7 * 16})
            .set<GridPosition>({16, 7})
            .set<GridPosition, Previous>({-1, -1})
            .child_of(room);

        prev.destruct();
      });
}
} // namespace ld53::game
