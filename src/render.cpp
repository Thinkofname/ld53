
#include "render.h"
#include <emscripten/val.h>
#include <emscripten/html5.h>

namespace ld53::render
{

    struct Renderer
    {
        emscripten::val canvas;
        emscripten::val ctx;

        int width, height;
    };

    struct Position {
        int x{0}, y{0};
    };

    void initRenderer(flecs::iter &it)
    {
        printf("Starting renderer\n");
        auto document = emscripten::val::global("document");
        auto canvas = document.call<emscripten::val>("getElementById", emscripten::val("canvas"));
        auto ctx = canvas.call<emscripten::val>("getContext", emscripten::val("2d"));

        canvas.set("width", 800);
        canvas.set("height", 600);

        it.world().emplace<Renderer>(canvas, ctx, 800, 600);
    }

    void beginFrame(Renderer& renderer)
    {
        renderer.width = renderer.canvas["clientWidth"].as<int>();
        renderer.height = renderer.canvas["clientHeight"].as<int>();
        renderer.canvas.set("width", renderer.width);
        renderer.canvas.set("height", renderer.height);
    }

    void drawBox(Renderer& renderer, const Position& pos)
    {
        renderer.ctx.set("fillStyle", emscripten::val("red"));
        renderer.ctx.call<void>("fillRect", pos.x, pos.y, 25, 25);
    }

    void aniTest(flecs::entity e, const Renderer& renderer, Position& pos)
    {
        pos.x += 1;
        pos.x %= renderer.width;
    }

    void initRender(flecs::world &ecs)
    {
        ecs.component<Renderer>();
        ecs.component<Position>()
            .member<int>("x")
            .member<int>("y");

        printf("Init renderer\n");
        ecs.system<>("initRenderer")
            .kind(flecs::OnStart)
            .write<Renderer>()
            .iter(initRenderer);
        ecs.system<Renderer>("beginFrame")
            .kind(flecs::PreStore)
            .each(beginFrame);
        ecs.system<Renderer, const Position>("drawBox")
            .kind(flecs::OnStore)
            .term_at(1).singleton()
            .each(drawBox);

        ecs.system<const Renderer, Position>("aniTest")
            .kind(flecs::OnUpdate)
            .term_at(1).singleton()
            .each(aniTest);

        ecs.entity().emplace<Position>(5, 23);
        ecs.entity().emplace<Position>(64, 100);
    }

} // namespace ld53::render