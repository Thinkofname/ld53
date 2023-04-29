
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <flecs.h>
#include <string>

#include "assets.h"
#include "game/common.h"
#include "web/input.h"
#include "web/render.h"

flecs::world *gWorld = nullptr;

void main_loop(void *ecsRaw) {
  flecs::world ecs{static_cast<flecs::world_t *>(ecsRaw)};
  ecs.progress();
}

int main_init(ecs_world_t *world, ecs_app_desc_t *desc) {
  emscripten_set_main_loop_arg(main_loop, world, 60, 1);
  return 0;
}

void loadAssets(flecs::world &ecs);
int main(void) {
  printf("Start\n");
  gWorld = new flecs::world{};

  gWorld->import <flecs::monitor>();
  loadAssets(*gWorld);
  ld53::render::initRender(*gWorld);
  ld53::game::initGame(*gWorld);
  ld53::input::initInput(*gWorld);

  ecs_app_set_run_action(main_init);

  return gWorld->app().enable_rest().run();
}

void loadAssets(flecs::world &ecs) {
  using namespace ld53::assets;
  using namespace ld53::game;
  ecs.entity<Test>().emplace<ld53::render::ImageAsset>("test.png");
  ecs.entity<Tileset>().emplace<ld53::render::ImageAsset>("tileset.png");

  ecs.entity<Tileset::Grass>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(0, 0)
      .add(TileType::None);
  ecs.entity<Tileset::GrassWithStone>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(1, 0)
      .add(TileType::None);

  ecs.entity<Tileset::TreeTop>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(2, 0)
      .add(TileType::Solid);
  ecs.entity<Tileset::TreeBottom>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(2, 1)
      .add(TileType::Solid);
  ecs.entity<Tileset::TreeBoth>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(1, 1)
      .add(TileType::Solid);
}

namespace ld53 {
std::string findLoc() {
  auto maybeUrl = emscripten::val::module_property("base_url");
  if (!maybeUrl.isString()) {
    auto params =
        emscripten::val::global("URLSearchParams")
            .new_(emscripten::val::global("document")["location"]["search"]);
    auto wasmUrl = params.call<emscripten::val>("get", emscripten::val("wasm"));
    if (!wasmUrl.isString())
      return "./";
    auto wasm = wasmUrl.as<std::string>();
    auto pos = wasm.find_last_of('/');
    auto url = wasm.substr(0, pos) + "/../data/";
    printf("URL: %s\n", url.c_str());
    return url;
  }
  return maybeUrl.as<std::string>();
}

std::string locateFile(const char *path) {
  static std::string scriptLoc = findLoc();
  return scriptLoc + path;
}
} // namespace ld53
