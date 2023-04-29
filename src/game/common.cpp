
#include "common.h"

namespace ld53::game
{
    void initGame(flecs::world &ecs)
    {
        ecs.component<Position>()
            .member<int>("x")
            .member<int>("y");
    }
} // namespace ld53::game