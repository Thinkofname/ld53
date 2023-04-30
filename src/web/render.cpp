
#include "render.h"

#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

#include "assets.h"
#include "game/common.h"
#include "game/room.h"
#include "main.h"

namespace ld53::render {
constexpr int VIRTUAL_WIDTH = 320;
constexpr int VIRTUAL_HEIGHT = 240;

struct Renderer {
  emscripten::val canvas;
  emscripten::val backingCtx;
  emscripten::val virtualCanvas;
  emscripten::val ctx;

  int width, height;
};

struct HTMLImage {
  struct IsLoaded {};
  emscripten::val image;
};

struct RenderRoom {
  emscripten::val canvas;
};

void initRenderer(flecs::iter &it) {
  printf("Starting renderer\n");
  auto document = emscripten::val::global("document");
  auto canvas = document.call<emscripten::val>("getElementById",
                                               emscripten::val("canvas"));
  auto virtualCanvas = document.call<emscripten::val>(
      "createElement", emscripten::val("canvas"));
  auto ctx = canvas.call<emscripten::val>("getContext", emscripten::val("2d"));
  auto virtualCtx =
      virtualCanvas.call<emscripten::val>("getContext", emscripten::val("2d"));

  canvas.set("width", 800);
  canvas.set("height", 600);

  it.world().emplace<Renderer>(canvas, ctx, virtualCanvas, virtualCtx, 800,
                               600);
}

void beginFrame(Renderer &renderer) {
  renderer.width = renderer.canvas["clientWidth"].as<int>();
  renderer.height = renderer.canvas["clientHeight"].as<int>();
  renderer.canvas.set("width", renderer.width);
  renderer.canvas.set("height", renderer.height);

  renderer.virtualCanvas.set("width", VIRTUAL_WIDTH);
  renderer.virtualCanvas.set("height", VIRTUAL_HEIGHT);

  auto &ctx = renderer.ctx;
  ctx.set("imageSmoothingEnabled", false);

  ctx.call<void>("save");
}
void endFrame(Renderer &renderer) {
  renderer.ctx.call<void>("restore");

  auto &ctx = renderer.backingCtx;
  ctx.set("imageSmoothingEnabled", false);

  // We need to make our game fit into the virtual screen and keep into
  // that space. Even if the canvas shape isn't right for it.

  auto targetAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;
  auto currentAspect = (float)renderer.width / (float)renderer.height;

  float scale = 1.0;
  if (currentAspect > targetAspect) {
    // Fit to height
    scale = (float)renderer.height / (float)VIRTUAL_HEIGHT;
  } else {
    // Fit to width
    scale = (float)renderer.width / (float)VIRTUAL_WIDTH;
  }
  float width = VIRTUAL_WIDTH * scale;
  float height = VIRTUAL_HEIGHT * scale;

  ctx.set("fillStyle", emscripten::val("#000000"));
  ctx.call<void>("fillRect", 0, 0, renderer.width, renderer.height);

  ctx.call<void>("drawImage", renderer.virtualCanvas,
                 (renderer.width - width) / 2, (renderer.height - height) / 2,
                 width, height);
}

void drawBox(Renderer &renderer, const game::Position &pos) {
  renderer.ctx.set("fillStyle", emscripten::val("red"));
  renderer.ctx.call<void>("fillRect", pos.x, pos.y, 16, 16);
}

void drawImage(Renderer &renderer, const game::Position &pos,
               const HTMLImage &img) {
  renderer.ctx.call<void>("drawImage", img.image, pos.x, pos.y);
}
void drawImageTile(Renderer &renderer, const game::Position &pos,
                   const HTMLImage &img, const ImageTile &tile) {
  renderer.ctx.call<void>("drawImage", img.image, tile.x * 16, tile.y * 16, 16,
                          16, pos.x, pos.y, 16, 16);
}
void drawImageAnimatedTile(flecs::entity e, Renderer &renderer,
                           const game::Position &pos, const HTMLImage &img,
                           const ImageTile &tile, const AnimatedTile &ani,
                           AnimatedTileState *state) {
  if (!state)
    state = e.get_mut<AnimatedTileState>();

  state->nextFrame -= e.delta_time() * ani.rate;
  if (state->nextFrame <= 0) {
    state->nextFrame += 1;
    state->frame = (state->frame + 1) % ani.frames;

    // Safety in case of lag spike/pause on brower tab
    if (state->nextFrame <= -5)
      state->nextFrame = 0;
  }

  renderer.ctx.call<void>("drawImage", img.image,
                          tile.x * 16 + state->frame * 16, tile.y * 16, 16, 16,
                          pos.x, pos.y, 16, 16);
}

void aniTest(flecs::entity e, const Renderer &renderer, game::Position &pos) {
  pos.x += 1;
  pos.x %= VIRTUAL_WIDTH;
}

void loadImages(flecs::entity e, const ImageAsset &asset) {
  auto document = emscripten::val::global("document");
  auto img =
      document.call<emscripten::val>("createElement", emscripten::val("img"));
  img.set("src", emscripten::val(locateFile(asset.path)));
  auto baseFunc = emscripten::val::module_property("on_image_load");
  auto func = baseFunc.call<emscripten::val>(
      "bind", emscripten::val::null(), (int)(e.remove_generation().raw_id()));
  img.call<void>("addEventListener", emscripten::val("load"), func);

  e.emplace<HTMLImage>(img);
}

void on_image_load(emscripten::val param, emscripten::val event) {
  int id = param.as<int>();
  printf("Image loaded for %d\n", id);
  auto entity = gWorld->get_alive(id);
  entity.add<HTMLImage::IsLoaded>();
}

void buildRoom(flecs::entity e, const game::Room &room) {
  printf("Building render room\n");
  auto ecs = e.world();
  auto document = emscripten::val::global("document");
  auto canvas = document.call<emscripten::val>("createElement",
                                               emscripten::val("canvas"));

  canvas.set("width", VIRTUAL_WIDTH);
  canvas.set("height", VIRTUAL_HEIGHT);

  auto wall = ecs.id<ld53::assets::Tileset::Wall>();
  auto wallBottom = ecs.id<ld53::assets::Tileset::WallBottom>();

  auto ctx = canvas.call<emscripten::val>("getContext", emscripten::val("2d"));
  for (int y = 0; y < game::ROOM_HEIGHT; y++) {
    for (int x = 0; x < game::ROOM_WIDTH; x++) {
      auto tile = e.world().entity(room.get_tile(x, y));
      if (!tile)
        continue;
      if (!tile.has<HTMLImage::IsLoaded>()) {
        // TODO: Handle this better?
        printf("Not all tiles loaded\n");
        return;
      }
      if (auto section = tile.get<ImageTile>()) {
        ctx.call<void>("drawImage", tile.get<HTMLImage>()->image,
                       section->x * 16, section->y * 16, 16, 16, x * 16, y * 16,
                       16, 16);
      } else {
        ctx.call<void>("drawImage", tile.get<HTMLImage>()->image, x * 16,
                       y * 16);
      }

      // Lighting for walls
      if (x > 0) {
        auto side = room.get_tile(x - 1, y);
        if (side == wall && tile != wall) {
          bool top = tile == wallBottom ||
                     (y > 0 && room.get_tile(x - 1, y - 1) != wall);
          ctx.call<void>("drawImage", tile.get<HTMLImage>()->image, 6 * 16,
                         (top ? 0 : 1) * 16, 16, 16, x * 16, y * 16, 16, 16);
        } else if (side == wallBottom && (tile != wall && tile != wallBottom)) {
          ctx.call<void>("drawImage", tile.get<HTMLImage>()->image, 6 * 16,
                         2 * 16, 16, 16, x * 16, y * 16, 16, 16);
        }
      }
    }
  }

  e.emplace<RenderRoom>(canvas);
}

void drawRoom(Renderer &renderer, const game::Position &pos,
              const RenderRoom &room) {
  renderer.ctx.call<void>("drawImage", room.canvas, pos.x, pos.y);
}
EMSCRIPTEN_BINDINGS(ld53) {
  emscripten::function("on_image_load", on_image_load);
}

void initRender(flecs::world &ecs) {
  ecs.component<Renderer>();
  ecs.component<ImageAsset>().member<const char *>("path");
  ecs.component<HTMLImage>();
  ecs.component<HTMLImage::IsLoaded>();
  ecs.component<Image>().add(flecs::Exclusive).add(flecs::Traversable);
  ecs.component<DependsOn>().add(flecs::Traversable);
  ecs.component<ImageTile>().member<int>("x").member<int>("y");
  ecs.component<AnimatedTile>().member<int>("frames").member<float>("rate");
  ecs.component<AnimatedTileState>().member<int>("frame").member<float>(
      "nextFrame");

  ecs.component<Depth>().add(flecs::Exclusive);
  ecs.component<Depth::Background>();
  ecs.component<Depth::Movable>();
  ecs.component<Depth::Player>();

  printf("Init renderer\n");
  ecs.system<>("initRenderer")
      .kind(flecs::OnStart)
      .write<Renderer>()
      .iter(initRenderer);
  ecs.system<Renderer>("beginFrame").kind(flecs::PreStore).each(beginFrame);
  ecs.system<Renderer>("endFrame").kind(flecs::PostFrame).each(endFrame);

  ecs.system<Renderer, const game::Position, const RenderRoom>("drawRoom")
      .kind(flecs::OnStore)
      .term_at(1)
      .singleton()
      .term_at(2)
      .second<game::World>()
      .each(drawRoom);

  ecs.system<const ImageAsset>("loadImages")
      .kind(flecs::PreFrame)
      .term_at(1)
      .self()
      .without<HTMLImage>()
      .write<HTMLImage>()
      .each(loadImages);
  ecs.system<Renderer, const game::Position, const HTMLImage>("drawImage")
      .kind(flecs::OnStore)
      .term_at(1)
      .singleton()
      .term_at(2)
      .second<game::World>()
      .term_at(3)
      .up<Image>()
      .with<HTMLImage::IsLoaded>()
      .up<Image>()
      .without<ImageTile>()
      .self()
      .up<Image>()
      .each(drawImage);
  ecs.system<Renderer, const game::Position, const HTMLImage, const ImageTile>(
         "drawImageTile")
      .kind(flecs::OnStore)
      .term_at(1)
      .singleton()
      .term_at(2)
      .second<game::World>()
      .term_at(3)
      .up<Image>()
      .with<HTMLImage::IsLoaded>()
      .up<Image>()
      .term_at(4)
      .self()
      .up<Image>()
      .without<AnimatedTile>()
      .self()
      .up<Image>()
      .group_by<Depth>()
      .each(drawImageTile);
  ecs.system<Renderer, const game::Position, const HTMLImage, const ImageTile,
             const AnimatedTile, AnimatedTileState *>("drawImageAnimatedTile")
      .kind(flecs::OnStore)
      .term_at(1)
      .singleton()
      .term_at(2)
      .second<game::World>()
      .term_at(3)
      .up<Image>()
      .with<HTMLImage::IsLoaded>()
      .up<Image>()
      .term_at(4)
      .self()
      .up<Image>()
      .term_at(5)
      .self()
      .up<Image>()
      .each(drawImageAnimatedTile);

  ecs.system<Renderer, const game::Position, const HTMLImage>("drawMailIcon")
      .kind(flecs::OnStore)
      .term_at(1)
      .singleton()
      .term_at(2)
      .second<game::World>()
      .term_at(3)
      .up<Image>()
      .with<HTMLImage::IsLoaded>()
      .up<Image>()
      .with<game::Holding>(flecs::Any)
      .each([](Renderer &renderer, const game::Position &pos,
               const HTMLImage &image) {
        renderer.ctx.call<void>("drawImage", image.image, 16, 3 * 16, 16, 16,
                                pos.x, pos.y - 8, 16, 16);
      });

  ecs.system<const game::Room>("buildRoomRender")
      .without<RenderRoom>()
      .write<RenderRoom>()
      .with<HTMLImage::IsLoaded>()
      .up<DependsOn>()
      .each(buildRoom);
  ecs.system<const game::Room>("buildRoomRenderDirty")
      .with<game::Room::IsDirty>()
      .write<RenderRoom>()
      .with<HTMLImage::IsLoaded>()
      .up<DependsOn>()
      .each(buildRoom);
}

} // namespace ld53::render