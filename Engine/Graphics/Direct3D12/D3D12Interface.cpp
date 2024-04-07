#include "D3D12Interface.h"

#include "D3D12Core.h"
#include "../GraphicsPlatformInterface.h"
#include "../../Common/CommonHeaders.h"

namespace ChillEngine::graphics::d3d12
{
    void get_platform_interface(platform_interface& pi)
    {
        pi.initialize = core::initialize;
        pi.shutdown = core::shutdown;
    }

}
