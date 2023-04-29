
#include <flecs.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <string>

#include "web/render.h"
#include "web/input.h"
#include "game/common.h"
#include "assets.h"

flecs::world *gWorld = nullptr;

void main_loop(void *ecsRaw)
{
    flecs::world ecs{static_cast<flecs::world_t *>(ecsRaw)};
    ecs.progress();
}

int main_init(ecs_world_t *world, ecs_app_desc_t *desc)
{
    emscripten_set_main_loop_arg(main_loop, world, 60, 1);
    return 0;
}

void loadAssets(flecs::world &ecs);
int main(void)
{
    printf("Start\n");
    gWorld = new flecs::world{};

    gWorld->import <flecs::monitor>();
    loadAssets(*gWorld);
    ld53::render::initRender(*gWorld);
    ld53::game::initGame(*gWorld);
    ld53::input::initInput(*gWorld);

    ecs_app_set_run_action(main_init);

    return gWorld->app()
        .enable_rest()
        .run();
}

void loadAssets(flecs::world &ecs)
{
    auto testImg = ecs.entity<ld53::assets::Test>().emplace<ld53::render::ImageAsset>("test.png");
}

namespace ld53
{
    std::string findLoc()
    {
        auto maybeUrl = emscripten::val::module_property("base_url");
        if (maybeUrl.isNull() || maybeUrl.isUndefined())
        {
            return "http://localhost:8000/data/";
        }
        return maybeUrl.as<std::string>();
    }
    std::string scriptLoc = findLoc();

    std::string
    locateFile(const char *path)
    {
        return scriptLoc + path;
    }
} // namespace ld53
