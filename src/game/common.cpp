
#include "common.h"
#include "assets.h"
#include "player.h"
#include "room.h"
#include "web/render.h"

#include <cmath>

namespace ld53::game {

struct LastDirAnimation {
  enum class Direction {
    Up,
    Down,
    Left,
    Right,
  } direction;
};

void initGame(flecs::world &ecs) {
  ecs.component<Position>().member<int>("x").member<int>("y");
  ecs.component<TileType>().add(flecs::Exclusive);
  ecs.component<World>();
  ecs.component<MovingState>();
  ecs.component<GridPosition>().member<int>("x").member<int>("y");

  ecs.component<AnimationSet>()
      .member<flecs::entity_view>("walk_down")
      .member<flecs::entity_view>("walk_up")
      .member<flecs::entity_view>("walk_left")
      .member<flecs::entity_view>("walk_right")
      .member<flecs::entity_view>("idle_down")
      .member<flecs::entity_view>("idle_up")
      .member<flecs::entity_view>("idle_left")
      .member<flecs::entity_view>("idle_right")
      .add_second<LastDirAnimation>(flecs::With);

  ecs.system<const GridPosition>("setPosition")
      .kind(flecs::PreUpdate)
      .without<Position>()
      .each([](flecs::entity e, const GridPosition &pos) {
        e.emplace<Position>(pos.x * 16, pos.y * 16);
      });

  ecs.system<const GridPosition>("makePreviousGridPos")
      .kind(flecs::PreUpdate)
      .without<GridPosition, Previous>()
      .each([](flecs::entity e, const GridPosition &pos) {
        e.emplace<GridPosition, Previous>(-1, -1);
      });

  ecs.system<const Room, const RoomObjects, GridPosition, const GridPosition>(
         "validateMovement")
      .kind(flecs::PostUpdate)
      .term_at(1)
      .parent()
      .term_at(2)
      .parent()
      .term_at(4)
      .second<Previous>()
      .each([](flecs::entity e, const Room &room, const RoomObjects &objs,
               GridPosition &pos, const GridPosition &prev) {
        auto tile = e.world().entity(room.get_tile(pos.x, pos.y));
        auto ty = tile.get<TileType>();
        if (ty && *ty == TileType::Solid) {
          pos = prev;
          return;
        }
        auto objects = objs.get_objects(pos.x, pos.y);
        for (auto &ent : objects) {
          auto obj = e.world().entity(ent);
          if (e == obj)
            continue;
          auto ty = obj.get<TileType>();
          if (ty && *ty == TileType::Solid) {
            pos = prev;
            break;
          }
        }
      });

  ecs.system<const GridPosition, const Position, const AnimationSet,
             LastDirAnimation>("updateMovementAnimation")
      .kind(flecs::PostUpdate)
      .each([](flecs::entity e, const GridPosition &grid, const Position &pos,
               const AnimationSet &set, LastDirAnimation &dir) {
        int targetX = grid.x * 16;
        int targetY = grid.y * 16;
        if (targetX > pos.x) { // Right
          if (set.walk_right)
            e.add<render::Image>(set.walk_right);
          dir.direction = LastDirAnimation::Direction::Right;
        } else if (targetX < pos.x) { // Left
          if (set.walk_left)
            e.add<render::Image>(set.walk_left);
          dir.direction = LastDirAnimation::Direction::Left;
        } else if (targetY > pos.y) { // Down
          if (set.walk_down)
            e.add<render::Image>(set.walk_down);
          dir.direction = LastDirAnimation::Direction::Down;
        } else if (targetY < pos.y) { // Up
          if (set.walk_up)
            e.add<render::Image>(set.walk_up);
          dir.direction = LastDirAnimation::Direction::Up;
        } else if (e.has(MovingState::Inactive)) {
          switch (dir.direction) {
          case LastDirAnimation::Direction::Right:
            if (set.idle_right)
              e.add<render::Image>(set.idle_right);
            break;
          case LastDirAnimation::Direction::Left:
            if (set.idle_left)
              e.add<render::Image>(set.idle_left);
            break;
          case LastDirAnimation::Direction::Down:
            if (set.idle_down)
              e.add<render::Image>(set.idle_down);
            break;
          case LastDirAnimation::Direction::Up:
            if (set.idle_up)
              e.add<render::Image>(set.idle_up);
            break;
          }
        }
      });

  ecs.system<const GridPosition, Position>("moveTowardsGrid")
      .kind(flecs::PostUpdate)
      .each([](flecs::entity e, const GridPosition &grid, Position &pos) {
        int targetX = grid.x * 16;
        int targetY = grid.y * 16;
        if (targetX != pos.x) {
          e.add(MovingState::Active);
          pos.x += std::copysign(1, targetX - pos.x);
        } else if (targetY != pos.y) {
          e.add(MovingState::Active);
          pos.y += std::copysign(1, targetY - pos.y);
        } else {
          e.add(MovingState::Inactive);
        }
      });

  ecs.system<RoomObjects, const GridPosition, const GridPosition>(
         "updateObjectRoomMap")
      .kind(flecs::PostUpdate)
      .term_at(1)
      .parent()
      .term_at(3)
      .second<Previous>()
      .each([](flecs::entity e, RoomObjects &room, const GridPosition &pos,
               const GridPosition &prev) {
        if (pos.x == prev.x && pos.y == prev.y)
          return;
        if (prev.x != -1) {
          auto &prevObjs = room.get_objects(prev.x, prev.y);
          prevObjs.erase(std::find(prevObjs.begin(), prevObjs.end(), e));
        }
        auto &objs = room.get_objects(pos.x, pos.y);
        objs.push_back(e);
      });
  ecs.system<const GridPosition, GridPosition>("commitGridPos")
      .kind(flecs::PostUpdate)
      .term_at(2)
      .second<Previous>()
      .each([](const GridPosition &pos, GridPosition &prev) { prev = pos; });
  ecs.system<>("makeWorldPosition")
      .with<Position>()
      .without<Position, World>()
      .write<Position, World>()
      .each([](flecs::entity e) { e.add<Position, World>(); });
  ecs.system<const Position, Position, const Position>("updateWorldPosition")
      .kind(flecs::PostUpdate)
      .term_at(2)
      .second<World>()
      .self()
      .term_at(3)
      .second<World>()
      .term_at(3)
      .parent()
      .cascade()
      .optional()
      .iter([](flecs::iter &it, const Position *pos, Position *outPosition,
               const Position *parentPosition) {
        if (parentPosition) {
          for (auto i : it) {
            outPosition[i].x = pos[i].x + parentPosition->x;
            outPosition[i].y = pos[i].y + parentPosition->y;
          }
        } else {
          for (auto i : it) {
            outPosition[i].x = pos[i].x;
            outPosition[i].y = pos[i].y;
          }
        }
      });

  initRoom(ecs);
  initPlayer(ecs);
}
} // namespace ld53::game