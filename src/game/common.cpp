
#include "common.h"
#include "room.h"

namespace ld53::game {
void initGame(flecs::world &ecs) {
  ecs.component<Position>().member<int>("x").member<int>("y");
  ecs.component<TileType>().add(flecs::Exclusive);

  ecs.system<>("makeWorldPosition")
      .with<Position>()
      .without<Position, World>()
      .write<Position, World>()
      .each([](flecs::entity e) { e.add<Position, World>(); });
  ecs.system<const Position, Position, const Position>("updateWorldPosition")
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
}
} // namespace ld53::game