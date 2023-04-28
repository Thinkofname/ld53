
#include <flecs.h>
#include <emscripten.h>

#include "render.h"

void main_loop(void* ecsRaw) {
    flecs::world ecs{static_cast<flecs::world_t*>(ecsRaw)};
    ecs.progress();
}

int main_init(ecs_world_t *world, ecs_app_desc_t* desc) {
    emscripten_set_main_loop_arg(main_loop, world, 60, 1);
    return 0;
}

int main(void) {
    printf("Start\n");
    auto ecs= new flecs::world{};

    ecs->import<flecs::monitor>();
    ld53::render::initRender(*ecs);

    ecs_app_set_run_action(main_init);

    return ecs->app()
        .enable_rest()
        .run();
}
