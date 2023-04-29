
#include "render.h"

#include <emscripten/val.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>

#include "main.h"
#include "game/common.h"
#include "assets.h"

namespace ld53::render
{

    struct Renderer
    {
        emscripten::val canvas;
        emscripten::val ctx;

        int width, height;
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

    void beginFrame(Renderer &renderer)
    {
        renderer.width = renderer.canvas["clientWidth"].as<int>();
        renderer.height = renderer.canvas["clientHeight"].as<int>();
        renderer.canvas.set("width", renderer.width);
        renderer.canvas.set("height", renderer.height);
    }

    void drawBox(Renderer &renderer, const game::Position &pos)
    {
        renderer.ctx.set("fillStyle", emscripten::val("red"));
        renderer.ctx.call<void>("fillRect", pos.x, pos.y, 25, 25);
    }

    void drawImage(Renderer &renderer, const game::Position &pos, const HTMLImage &img)
    {
        renderer.ctx.call<void>("drawImage", img.image, pos.x, pos.y);
    }

    void aniTest(flecs::entity e, const Renderer &renderer, game::Position &pos)
    {
        pos.x += 1;
        pos.x %= renderer.width;
    }

    void loadImages(flecs::entity e, const ImageAsset &asset)
    {
        auto document = emscripten::val::global("document");
        auto img = document.call<emscripten::val>("createElement", emscripten::val("img"));
        img.set("src", emscripten::val(locateFile(asset.path)));
        auto baseFunc = emscripten::val::module_property("on_image_load");
        auto func = baseFunc.call<emscripten::val>("bind", emscripten::val::null(), (int)(e.remove_generation().raw_id()));
        img.call<void>("addEventListener", emscripten::val("load"), func);

        e.emplace<HTMLImage>(img);
    }

    void on_image_load(emscripten::val param, emscripten::val event)
    {
        int id = param.as<int>();
        printf("Image loaded for %d\n", id);
        auto entity = gWorld->get_alive(id);
        entity.add<HTMLImage::IsLoaded>();
    }

    EMSCRIPTEN_BINDINGS(ld53)
    {
        emscripten::function("on_image_load", on_image_load);
    }

    void initRender(flecs::world &ecs)
    {
        ecs.component<Renderer>();
        ecs.component<ImageAsset>()
            .member<const char *>("path");
        ecs.component<HTMLImage>();
        ecs.component<HTMLImage::IsLoaded>();
        ecs.component<Image>().add(flecs::Exclusive).add(flecs::Traversable);

        printf("Init renderer\n");
        ecs.system<>("initRenderer")
            .kind(flecs::OnStart)
            .write<Renderer>()
            .iter(initRenderer);
        ecs.system<Renderer>("beginFrame")
            .kind(flecs::PreStore)
            .each(beginFrame);
        ecs.system<Renderer, const game::Position>("drawBox")
            .kind(flecs::OnStore)
            .term_at(1)
            .singleton()
            .each(drawBox);

        ecs.system<const Renderer, game::Position>("aniTest")
            .kind(flecs::OnUpdate)
            .term_at(1)
            .singleton()
            .each(aniTest);

        ecs.system<const ImageAsset>("loadImages")
            .kind(flecs::PreFrame)
            .without<HTMLImage>()
            .write<HTMLImage>()
            .each(loadImages);
        ecs.system<Renderer, const game::Position, const HTMLImage>("drawImage")
            .kind(flecs::OnStore)
            .term_at(1)
            .singleton()
            .term_at(3)
            .up<Image>()
            .with<HTMLImage::IsLoaded>()
            .up<Image>()
            .each(drawImage);

        ecs.entity().emplace<game::Position>(5, 23);
        ecs.entity().emplace<game::Position>(64, 100);

        ecs.entity().emplace<game::Position>(250, 40).add<Image, ld53::assets::Test>();
    }

} // namespace ld53::render