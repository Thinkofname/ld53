#pragma

#include <flecs.h>

namespace ld53::game {

struct Position {
  int x{0}, y{0};
};
struct World {};
struct Previous {};

struct GridPosition {
  int x{0}, y{0};
};

enum class MovingState {
  Inactive,
  Active,
};

enum class TileType {
  None,
  Solid,
};

struct MailObject {};
struct MailBox {
  struct Full {};
};

struct ConnectedTo {};

struct Pushable {};
struct CanPush {};
struct Weighted {};
struct WeightActivated {};
struct ActivatedBy {};
struct Gate {};

struct Holding {};

struct Velocity {
  int x{0}, y{0};
};

struct AnimationSet {
  flecs::entity_view walk_down{};
  flecs::entity_view walk_up{};
  flecs::entity_view walk_left{};
  flecs::entity_view walk_right{};

  flecs::entity_view idle_down{};
  flecs::entity_view idle_up{};
  flecs::entity_view idle_left{};
  flecs::entity_view idle_right{};
};

struct LastDirAnimation {
  enum class Direction {
    Up,
    Down,
    Left,
    Right,
  } direction;
};

void initGame(flecs::world &ecs);
} // namespace ld53::game
