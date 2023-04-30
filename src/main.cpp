
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
  ecs.entity<Tutorial>().emplace<ld53::render::ImageAsset>("tutorial.png");
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

  ecs.entity<Tileset::Wall>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(4, 2)
      .add(TileType::Solid);
  ecs.entity<Tileset::WallBottom>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(4, 3)
      .add(TileType::None);
  ecs.entity<Tileset::Gate>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(5, 2)
      .add(TileType::Solid);
  ecs.entity<Tileset::GateOpened>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(5, 3)
      .add(TileType::Solid);

  ecs.entity<Tileset::Mail>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(0, 2)
      .emplace<ld53::render::AnimatedTile>(2, 2.0f);
  ecs.entity<Tileset::Mailbox>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(0, 3);
  ecs.entity<Tileset::MailboxFull>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(0, 4);
  ecs.entity<Tileset::ButtonPlate>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(2, 2);
  ecs.entity<Tileset::ButtonPlatePressed>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(3, 2);
  ecs.entity<Tileset::Box>().is_a<Tileset>().emplace<ld53::render::ImageTile>(
      2, 3);

  ecs.entity<Tileset::WireTB>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(3, 0)
      .add(TileType::None);
  ecs.entity<Tileset::WireLR>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(3, 1)
      .add(TileType::None);
  ecs.entity<Tileset::WireBR>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(4, 0)
      .add(TileType::None);
  ecs.entity<Tileset::WireTL>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(5, 0)
      .add(TileType::None);
  ecs.entity<Tileset::WireBL>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(4, 1)
      .add(TileType::None);
  ecs.entity<Tileset::WireTR>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(5, 1)
      .add(TileType::None);

  ecs.entity<Tileset::PlayerIdleDown>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 0);
  ecs.entity<Tileset::PlayerWalkDown>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 0)
      .emplace<ld53::render::AnimatedTile>(4, 8.0f);
  ecs.entity<Tileset::PlayerIdleUp>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 1);
  ecs.entity<Tileset::PlayerWalkUp>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 1)
      .emplace<ld53::render::AnimatedTile>(4, 8.0f);
  ecs.entity<Tileset::PlayerIdleLeft>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 2);
  ecs.entity<Tileset::PlayerWalkLeft>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 2)
      .emplace<ld53::render::AnimatedTile>(4, 8.0f);
  ecs.entity<Tileset::PlayerIdleRight>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 3);
  ecs.entity<Tileset::PlayerWalkRight>()
      .is_a<Tileset>()
      .emplace<ld53::render::ImageTile>(16, 3)
      .emplace<ld53::render::AnimatedTile>(4, 8.0f);
}

namespace ld53 {
std::string findLoc() {
  auto params =
      emscripten::val::global("URLSearchParams")
          .new_(emscripten::val::global("document")["location"]["search"]);
  auto wasmUrl = params.call<emscripten::val>("get", emscripten::val("wasm"));
  if (!wasmUrl.isString()) {
    printf("Missing wasm url\n");
    return "./data/";
  }
  auto wasm = wasmUrl.as<std::string>();
  auto pos = wasm.find_last_of('/');
  auto url = wasm.substr(0, pos) + "/../data/";
  printf("URL: %s\n", url.c_str());
  return url;
}

std::string locateFile(const char *path) {
  static std::string scriptLoc = findLoc();
  return scriptLoc + path;
}
} // namespace ld53
