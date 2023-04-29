#pragma

#include <flecs.h>

namespace ld53::game
{
    struct Position {
        int x{0}, y{0};
    };

    void initGame(flecs::world &ecs);
} // namespace ld53::game
