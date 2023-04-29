cmake_minimum_required(VERSION 3.14)
include(FetchContent)
include(CheckIPOSupported)
project(ld53)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(EMSCRIPTEN_KEEPALIVE=__attribute__\(\(used\)\))

FetchContent_Declare(
        flecs
        URL https://github.com/SanderMertens/flecs/archive/e7c40b90f8d68b3fe22933dafeaf5bdb6b7a6f4e.tar.gz # master
)
FetchContent_MakeAvailable(flecs)

include_directories(src)

add_executable(ld53 src/main.cpp src/main.h
        src/web/render.cpp src/web/render.h
        src/web/input.cpp src/web/input.h
        src/game/common.cpp src/game/common.h
)
target_link_libraries(ld53 flecs_static embind)

set_target_properties(ld53 PROPERTIES LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=cwrap -s MODULARIZE=1 -s EXPORT_NAME=\"ld53\" -s INITIAL_MEMORY=256MB -s STACK_SIZE=256kb")

set_target_properties(ld53 PROPERTIES LINK_FLAGS_DEBUG "-O0 -g -gsource-map --source-map-base=http://localhost:8000/build/")
set_target_properties(ld53 PROPERTIES LINK_FLAGS_RELEASE "-O3")