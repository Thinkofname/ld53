#pragma once

#include <flecs.h>
#include <emscripten/val.h>

extern flecs::world *gWorld;

namespace ld53
{
    std::string locateFile(const char *path);
}