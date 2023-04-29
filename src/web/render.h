#pragma once

#include <flecs.h>
#include <emscripten/val.h>

namespace ld53::render
{

    struct ImageAsset
    {
        const char *path;
    };

    struct HTMLImage
    {
        struct IsLoaded
        {
        };
        emscripten::val image;
    };

    struct Image
    {
    };

    void initRender(flecs::world &ecs);
}