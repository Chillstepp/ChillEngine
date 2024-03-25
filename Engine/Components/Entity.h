#pragma once

#include "ComponentsCommonHeaders.h"

namespace ChillEngine
{

    struct entity_info
    {
    
    };
    
    u32 create_game_entity(const entity_info& info);
    void remove_game_entity(u32 id);
}
