
#include "player.h"
#include "assets.h"
#include "common.h"
#include "room.h"
#include "web/input.h"
#include "web/render.h"

namespace ld53::game {

struct PlayerMovementState {
  bool up{false}, down{false}, left{false}, right{false};
};

void initPlayer(flecs::world &ecs) {
  ecs.component<PlayerMovementState>()
      .member<bool>("up")
      .member<bool>("down")
      .member<bool>("left")
      .member<bool>("right");

  auto room = ecs.entity().is_a<Rooms::Level1>().child_of<RoomInstances>();
  ecs.add<CurrentRoom>(room);
  ecs.add<CurrentRoomType, Rooms::Level1>();
  ecs.entity<Player>()
      .emplace<Position>(18 * 16, 7 * 16)
      .emplace<GridPosition>(16, 7)
      .add<render::Image, assets::Tileset::PlayerIdleDown>()
      .add<PlayerMovementState>()
      .add(MovingState::Inactive)
      .add<CanPush>()
      .add<Weighted>()
      .set([&](AnimationSet &set) {
        set.idle_down = ecs.entity<assets::Tileset::PlayerIdleDown>().view();
        set.walk_down = ecs.entity<assets::Tileset::PlayerWalkDown>().view();
        set.idle_up = ecs.entity<assets::Tileset::PlayerIdleUp>().view();
        set.walk_up = ecs.entity<assets::Tileset::PlayerWalkUp>().view();
        set.idle_left = ecs.entity<assets::Tileset::PlayerIdleLeft>().view();
        set.walk_left = ecs.entity<assets::Tileset::PlayerWalkLeft>().view();
        set.idle_right = ecs.entity<assets::Tileset::PlayerIdleRight>().view();
        set.walk_right = ecs.entity<assets::Tileset::PlayerWalkRight>().view();
      })
      .add<render::Depth, render::Depth::Player>()
      .child_of(room);

  ecs.system<PlayerMovementState, const input::InputData, LastDirAnimation>(
         "processPlayerInput")
      .kind(flecs::PreUpdate)
      .term_at(1)
      .src<Player>()
      .term_at(3)
      .src<Player>()
      .write<GridPosition, Previous>()
      .each([](flecs::entity e, PlayerMovementState &state,
               const input::InputData &data, LastDirAnimation &dir) {
        auto ecs = e.world();
        switch (data.type) {
        case input::InputType::Up:
          state.up = data.pressed;
          dir.direction = LastDirAnimation::Direction::Up;
          break;
        case input::InputType::Down:
          state.down = data.pressed;
          dir.direction = LastDirAnimation::Direction::Down;
          break;
        case input::InputType::Left:
          state.left = data.pressed;
          dir.direction = LastDirAnimation::Direction::Left;
          break;
        case input::InputType::Right:
          state.right = data.pressed;
          dir.direction = LastDirAnimation::Direction::Right;
          break;
        case input::InputType::Fire: {
          auto player = e.world().entity<Player>();
          auto mail = player.target<Holding>();
          if (data.pressed || !mail)
            return;

          mail.enable();
          auto playerPos = player.get<GridPosition>();
          auto pos = mail.get_mut<GridPosition>();
          auto posLast = mail.get_mut<GridPosition, Previous>();
          *posLast = *playerPos;
          auto absPos = mail.get_mut<Position>();
          int ox = 0;
          int oy = 0;
          switch (dir.direction) {
          case LastDirAnimation::Direction::Up:
            oy = -1;
            break;
          case LastDirAnimation::Direction::Down:
            oy = 1;
            break;
          case LastDirAnimation::Direction::Left:
            ox = -1;
            break;
          case LastDirAnimation::Direction::Right:
            ox = 1;
            break;
          }
          pos->x = playerPos->x + ox;
          pos->y = playerPos->y + oy;

          absPos->x = playerPos->x * 16;
          absPos->y = playerPos->y * 16;

          mail.set<Velocity>({ox, oy});

          player.remove<Holding>(flecs::Wildcard);
          break;
        }
        case input::InputType::Restart:
          if (data.pressed)
            return;

          ecs.add<ChangeRoom>(
              ecs.singleton<CurrentRoomType>().target<CurrentRoomType>());
          break;
        }
      });

  ecs.system<const PlayerMovementState, GridPosition>("movePlayer")
      .with(MovingState::Inactive)
      .each([](flecs::entity e, const PlayerMovementState &state,
               GridPosition &grid) {
        if (state.up) {
          grid.y -= 1;
        } else if (state.down) {
          grid.y += 1;
        } else if (state.left) {
          grid.x -= 1;
        } else if (state.right) {
          grid.x += 1;
        }
      });
}
} // namespace ld53::game