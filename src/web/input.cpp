#include "input.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

namespace ld53::input {

bool event_keyup(emscripten::val event) {
  printf("Got event\n");
  return true;
}
bool event_keydown(emscripten::val event) {
  printf("Got event\n");
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

void initInput(flecs::world &ecs) { sokol_capture_keyboard_events(true); }
} // namespace ld53::input