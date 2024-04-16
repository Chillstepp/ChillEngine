#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12::fx
{
    bool initialize();
    void shutdown();

    void post_process(ID3D12GraphicsCommandList6* cmd_list, D3D12_CPU_DESCRIPTOR_HANDLE target_rtv);
}
