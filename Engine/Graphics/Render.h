
#pragma once
#include "../Common/CommonHeaders.h"
#include "../Platform/Window.h"

namespace ChillEngine::graphics {

    class surface
    {};

    struct render_surface
    {
        platform::window window{};
        surface surface{};
    };

    enum class graphics_platform : u32
    {
        direct3d12=0,
        vulkan = 1
    };
    
    bool initialize(graphics_platform platform);
    void shutdown();
    void render();


}
