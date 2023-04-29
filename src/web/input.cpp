#include "input.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <optional>

#include "main.h"

namespace ld53::input {

std::optional<InputType> mapKey(std::string_view key) {
  InputType type;
  if (key == "KeyW" || key == "ArrowUp")
    type = InputType::Up;
  else if (key == "KeyS" || key == "ArrowDown")
    type = InputType::Down;
  else if (key == "KeyA" || key == "ArrowLeft")
    type = InputType::Left;
  else if (key == "KeyD" || key == "ArrowRight")
    type = InputType::Right;
  else
    return {};

  return {type};
}

bool event_keyup(emscripten::val event) {
  auto key = event["code"].as<std::string>();
  printf("Got event: %s\n", key.c_str());

  auto type = mapKey(key);
  if (!type)
    return false;

  gWorld->entity().emplace<InputData>(false, *type);
  return true;
}
bool event_keydown(emscripten::val event) {
  auto key = event["code"].as<std::string>();
  printf("Got event: %s\n", key.c_str());

  auto type = mapKey(key);
  if (!type)
    return false;

  gWorld->entity().emplace<InputData>(true, *type);
  return true;
}

// Fake sokol for flecs explorer support
void sokol_capture_keyboard_events(bool enable) {
  auto window = emscripten::val::global("window");
  const char *action = enable ? "addEventListener" : "removeEventListener";

  window.call<void>(action, emscripten::val("keyup"),
                    emscripten::val::module_property("event_keyup"));
  window.call<void>(action, emscripten::val("keydown"),
                    emscripten::val::module_property("event_keydown"));
}

EMSCRIPTEN_BINDINGS(ld53) {
  emscripten::function("sokol_capture_keyboard_events",
                       sokol_capture_keyboard_events);
  emscripten::function("event_keyup", event_keyup);
  emscripten::function("event_keydown", event_keydown);
}

void initInput(flecs::world &ecs) {
  sokol_capture_keyboard_events(true);
  flecs::enum_type<InputType>(ecs);
  ecs.component<InputType>()
      .constant("Up", (int32_t)InputType::Up)
      .constant("Down", (int32_t)InputType::Down)
      .constant("Left", (int32_t)InputType::Left)
      .constant("Right", (int32_t)InputType::Right);
  ecs.component<InputData>().member<bool>("pressed").member<InputType>("type");

  ecs.system<>("cleanupInputData")
      .with<InputData>()
      .inout_none()
      .kind(flecs::PostFrame)
      .each([](flecs::entity e) { e.destruct(); });
}
} // namespace ld53::input