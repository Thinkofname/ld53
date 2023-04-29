#pragma once

#include <flecs.h>

namespace ld53::render {

struct ImageAsset {
  const char *path;
};

struct Image {};

struct ImageTile {
  int x{0}, y{0};
};

void initRender(flecs::world &ecs);
} // namespace ld53::render