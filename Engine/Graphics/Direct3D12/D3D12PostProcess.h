#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12
{
    struct d3d12_frame_info;
}

namespace ChillEngine::graphics::d3d12::fx
{
    bool initialize();
    void shutdown();

    void post_process(ID3D12GraphicsCommandList6* cmd_list,
        const d3d12_frame_info& d3d12_info,
        D3D12_CPU_DESCRIPTOR_HANDLE target_rtv);
}
