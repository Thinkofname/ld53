#pragma once

#include <emscripten/val.h>
#include <flecs.h>

extern flecs::world *gWorld;

namespace ld53 {
std::string locateFile(const char *path);
}