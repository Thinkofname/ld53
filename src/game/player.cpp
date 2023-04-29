
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

  ecs.entity<Player>()
      .emplace<Position>(256, 64)
      .emplace<GridPosition>(15, 9)
      .add<render::Image, assets::Tileset::PlayerIdleDown>()
      .add<PlayerMovementState>()
      .add(MovingState::Inactive)
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
      .child_of(ecs.entity("TestRoomInstance").is_a<Rooms::TestRoom>());

  ecs.system<PlayerMovementState, const input::InputData>("updateMovementState")
      .kind(flecs::PreUpdate)
      .term_at(1)
      .src<Player>()
      .each([](PlayerMovementState &state, const input::InputData &data) {
        switch (data.type) {
        case input::InputType::Up:
          state.up = data.pressed;
          break;
        case input::InputType::Down:
          state.down = data.pressed;
          break;
        case input::InputType::Left:
          state.left = data.pressed;
          break;
        case input::InputType::Right:
          state.right = data.pressed;
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