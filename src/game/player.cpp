
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

  auto room = ecs.entity().is_a<Rooms::TestRoom>().child_of<RoomInstances>();
  ecs.add<CurrentRoom>(room);
  ecs.add<CurrentRoomType, Rooms::TestRoom>();
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
      .child_of(room);

  ecs.system<PlayerMovementState, const input::InputData, LastDirAnimation>(
         "processPlayerInput")
      .kind(flecs::PreUpdate)
      .term_at(1)
      .src<Player>()
      .term_at(3)
      .src<Player>()
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
        case input::InputType::Restart:
          if (data.pressed)
            return;
          auto room = ecs.entity()
                          .is_a(ecs.singleton<CurrentRoomType>()
                                    .target<CurrentRoomType>())
                          .child_of<RoomInstances>();
          auto prev = ecs.singleton<CurrentRoom>().target<CurrentRoom>();
          ecs.add<CurrentRoom>(room);

          e.world()
              .entity<Player>()
              .set<Position>({18 * 16, 7 * 16})
              .set<GridPosition>({16, 7})
              .set<GridPosition, Previous>({-1, -1})
              .child_of(room);

          prev.destruct();

          break;
        }
      });

  ecs.system<const PlayerMovementState, GridPosition>("movePlayer")
      .with(MovingState::Inactive)
      .each([](flecs::entity e, const PlayerMovementState &state,
               GridPosition &grid) {
        // TODO: Collisions
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