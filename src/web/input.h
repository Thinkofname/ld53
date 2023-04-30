#pragma once

#include <flecs.h>

namespace ld53::input {

enum class InputType {
  Up,
  Down,
  Left,
  Right,
  Fire,
  Restart,
};

struct InputData {
  bool pressed;
  InputType type;
};

void initInput(flecs::world &ecs);
} // namespace ld53::input