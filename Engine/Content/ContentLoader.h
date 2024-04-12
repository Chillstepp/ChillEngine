#pragma once

#include "../Common/CommonHeaders.h"

namespace ChillEngine::content {

    
    bool load_engine_shaders(std::unique_ptr<u8[]>& shaders, u64& size);
}
