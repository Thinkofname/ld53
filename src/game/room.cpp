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
flecs::entity makeRoom(flecs::world &ecs, const char *mapData,
                       const std::unordered_map<char, flecs::entity_t> &tiles) {
  flecs::entity e = ecs.prefab<T>();
  auto room = e.emplace_override<Position>(0, 0)
                  .add<render::DependsOn, assets::Tileset>()
                  .override<render::DependsOn, assets::Tileset>()
                  .get_mut<Room>();
  auto stone = ecs.id<ld53::assets::Tileset::GrassWithStone>();
  auto grass = ecs.id<ld53::assets::Tileset::Grass>();
  for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; i++) {
    room->tiles[i] = tiles.at(mapData[i]);
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
  struct GateInverted {};
  struct ButtonPlate {};
};

void initRoom(flecs::world &ecs) {
  ecs.component<RoomObjects>().add(EcsAlwaysOverride);
  ecs.component<Room>().add_second<RoomObjects>(flecs::With);
  ecs.component<NextRoom>().add(flecs::Exclusive);
  ecs.component<ChangeRoom>().add(flecs::Exclusive);

  auto grass = ecs.id<ld53::assets::Tileset::Grass>();
  auto grassTall = ecs.id<ld53::assets::Tileset::GrassTall>();
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
  std::unordered_map<char, flecs::entity_t> tiles{
      {' ', grass},      {'v', top},    {'#', both},   {'^', bottom},
      {'B', wallBottom}, {'W', wall},   {'L', wireTR}, {'-', wireLR},
      {'|', wireTB},     {'/', wireTL}, {'=', wireBR}, {'@', grassTall}};

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
  ecs.prefab<Prefab::GateInverted>()
      .add<render::Image, assets::Tileset::Gate>()
      .add<Gate>()
      .add<Inverted>()
      .add(TileType::Solid);
  ecs.prefab<Prefab::ButtonPlate>()
      .add<render::Image, assets::Tileset::ButtonPlate>()
      .add<WeightActivated>()
      .add<render::Depth, render::Depth::Background>()
      .override<render::Depth, render::Depth::Background>();

  makeRoom<Rooms::EndingScreen>(ecs,
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
                                tiles)
      .with(flecs::ChildOf, [&]() {
        ecs.entity()
            .emplace<Position>(0, 0)
            .add<render::Image, assets::EndingScreen>()
            .add<render::Depth, render::Depth::Background>();
      });
  makeRoom<Rooms::Level5>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#    W     WWWWWWWW#"
                          "#    W     BBBBBBBB#"
                          "#    W     @       #"
                          "#    W     WWW@WWWW#"
                          "#@@ @B     WBB@BBBB#"
                          "#          W  @    #"
                          "#          B       #"
                          "#          WWWWWWWW#"
                          "#WWW W     WBBBBBBB#"
                          "#BBB W     W       #"
                          "#    W     W       #"
                          "#    W     W       #"
                          "#    W     B       #"
                          "#vvvvBvvvvvvvvvvvvv#",
                          tiles)
      .with(
          flecs::ChildOf,
          [&]() {
            ecs.entity().emplace<GridPosition>(18, 13).is_a<Prefab::Mailbox>();
            ecs.entity().emplace<GridPosition>(18, 5).is_a<Prefab::Mail>();

            auto gate1 =
                ecs.entity().emplace<GridPosition>(14, 7).is_a<Prefab::Gate>();
            auto gate3 =
                ecs.entity().emplace<GridPosition>(14, 2).is_a<Prefab::Gate>();
            auto gate4 =
                ecs.entity().emplace<GridPosition>(11, 2).is_a<Prefab::Gate>();
            auto gate5 =
                ecs.entity().emplace<GridPosition>(11, 13).is_a<Prefab::Gate>();

            auto gateMail1 =
                ecs.entity().emplace<GridPosition>(14, 3).is_a<Prefab::Gate>();
            auto gateMail2 =
                ecs.entity().emplace<GridPosition>(15, 3).is_a<Prefab::Gate>();

            auto gate6 =
                ecs.entity().emplace<GridPosition>(3, 5).is_a<Prefab::Gate>();
            auto gate7 =
                ecs.entity().emplace<GridPosition>(4, 9).is_a<Prefab::Gate>();

            ecs.entity().emplace<GridPosition>(8, 7).is_a<Prefab::Box>();
            ecs.entity().emplace<GridPosition>(2, 11).is_a<Prefab::Box>();
            ecs.entity().emplace<GridPosition>(2, 12).is_a<Prefab::Box>();

            ecs.entity()
                .emplace<GridPosition>(12, 6)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate1);
            ecs.entity()
                .emplace<GridPosition>(18, 2)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate3)
                .add<ConnectedTo>(gate4)
                .add<ConnectedTo>(gate5);
            ecs.entity()
                .emplace<GridPosition>(18, 3)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate3)
                .add<ConnectedTo>(gate4)
                .add<ConnectedTo>(gate5);
            ecs.entity()
                .emplace<GridPosition>(2, 1)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate6)
                .add<ConnectedTo>(gateMail1);
            ecs.entity()
                .emplace<GridPosition>(2, 7)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate7);

            ecs.entity()
                .emplace<GridPosition>(6, 13)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gateMail2);
          })
      .add<NextRoom, Rooms::EndingScreen>();
  makeRoom<Rooms::Level4>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#                  #"
                          "#    W             #"
                          "#    W             #"
                          "#W WWW@@@@@@@@@@@@@#"
                          "#B BBW             #"
                          "#    W             #"
                          "#    W             #"
                          "#    W             #"
                          "#    B             #"
                          "#    W             #"
                          "#    W             #"
                          "#WWWWWWWWWWWWW     #"
                          "#BBBBBBBBBBBBB     #"
                          "#vvvvvvvvvvvvvvvvvv#",
                          tiles)
      .with(
          flecs::ChildOf,
          [&]() {
            ecs.entity().emplace<GridPosition>(1, 13).is_a<Prefab::Mailbox>();
            ecs.entity().emplace<GridPosition>(17, 1).is_a<Prefab::Mail>();
            ecs.entity().emplace<GridPosition>(17, 2).is_a<Prefab::Mail>();
            ecs.entity().emplace<GridPosition>(17, 3).is_a<Prefab::Mail>();

            ecs.entity().emplace<GridPosition>(17, 6).is_a<Prefab::Box>();
            ecs.entity().emplace<GridPosition>(17, 8).is_a<Prefab::Box>();

            auto gate1 =
                ecs.entity().emplace<GridPosition>(2, 4).is_a<Prefab::Gate>();
            auto gate2 = ecs.entity()
                             .emplace<GridPosition>(5, 9)
                             .is_a<Prefab::Gate>()
                             .add<Inverted>();

            ecs.entity()
                .emplace<GridPosition>(2, 9)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate1)
                .add<ConnectedTo>(gate2);

            auto gateExit1 =
                ecs.entity().emplace<GridPosition>(7, 13).is_a<Prefab::Gate>();
            auto gateExit2 =
                ecs.entity().emplace<GridPosition>(9, 13).is_a<Prefab::Gate>();
            auto gateExit3 =
                ecs.entity().emplace<GridPosition>(11, 13).is_a<Prefab::Gate>();
            auto gateExit4 =
                ecs.entity().emplace<GridPosition>(13, 13).is_a<Prefab::Gate>();

            ecs.entity()
                .emplace<GridPosition>(7, 8)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gateExit1);
            ecs.entity()
                .emplace<GridPosition>(9, 8)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gateExit2);
            ecs.entity()
                .emplace<GridPosition>(11, 8)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gateExit3);
            ecs.entity()
                .emplace<GridPosition>(13, 8)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gateExit4);
          })
      .add<NextRoom, Rooms::Level5>();
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
                          "#                  #"
                          "#                  #"
                          "#                  #"
                          "#vvvvvvvvvvvvvvvvvv#",
                          tiles)
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
      .add<NextRoom, Rooms::Level4>();
  makeRoom<Rooms::Level2>(ecs,
                          "#^^^^^^^^^^^^^^^^^^#"
                          "#        WW        #"
                          "#        BB        #"
                          "#                  #"
                          "#        WW        #"
                          "#        BBW       #"
                          "#     =----WWWW WWW#"
                          "#     |  @@BBBB BBB#"
                          "#  W -/  @@        #"
                          "#  W     @@        #"
                          "#  W     @@        #"
                          "#  W     @@WW      #"
                          "#  B     @@BW      #"
                          "#        @@ W      #"
                          "#vvvvvvvvvvvBvvvvvv#",
                          tiles)
      .with(
          flecs::ChildOf,
          [&]() {
            ecs.entity().emplace<GridPosition>(11, 13).is_a<Prefab::Mailbox>();
            ecs.entity().emplace<GridPosition>(3, 3).is_a<Prefab::Mailbox>();
            ecs.entity().emplace<GridPosition>(14, 3).is_a<Prefab::Mail>();
            ecs.entity().emplace<GridPosition>(14, 8).is_a<Prefab::Mail>();

            auto gate1 =
                ecs.entity().emplace<GridPosition>(15, 6).is_a<Prefab::Gate>();
            auto gate2 =
                ecs.entity().emplace<GridPosition>(10, 3).is_a<Prefab::Gate>();
            auto gate3 =
                ecs.entity().emplace<GridPosition>(10, 2).is_a<Prefab::Gate>();

            ecs.entity()
                .emplace<GridPosition>(4, 8)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate1);
            ecs.entity()
                .emplace<GridPosition>(4, 10)
                .is_a<Prefab::ButtonPlate>()
                .add<ConnectedTo>(gate2)
                .add<ConnectedTo>(gate3);
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
                          "#@@@@@@   =/       #"
                          "#@@@ @@   |        #"
                          "#@@@@@@            #"
                          "#                  #"
                          "#                  #"
                          "#                  #"
                          "#                  #"
                          "#vvvvvvvvvvvvvvvvvv#",
                          tiles)
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
        ecs.add<CurrentRoomType>(nextRoom);

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
