#include "D3D12Interface.h"

#include "D3D12Core.h"
#include "../GraphicsPlatformInterface.h"
#include "../../Common/CommonHeaders.h"

namespace ChillEngine::graphics::d3d12
{
    void get_platform_interface(platform_interface& pi)
    {
        pi.platform = graphics_platform::direct3d12;
        
        pi.initialize = core::initialize;
        pi.shutdown = core::shutdown;
        
        pi.surface.create = core::create_surface;
        pi.surface.height = core::surface_height;
        pi.surface.remove = core::remove_surface;
        pi.surface.render = core::render_surface;
        pi.surface.width = core::surface_width;
        pi.surface.resize = core::resize_surface;
    }

}
