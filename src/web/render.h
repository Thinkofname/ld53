#pragma once

#include <flecs.h>

namespace ld53::render {

struct ImageAsset {
  const char *path;
};

struct Image {};
struct DependsOn {};

struct ImageTile {
  int x{0}, y{0};
};

struct AnimatedTile {
  int frames{0};
  float rate{60.0};
};

struct AnimatedTileState {
  int frame{0};
  float nextFrame{1.0};
};

void initRender(flecs::world &ecs);
} // namespace ld53::render