#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12
{
    struct d3d12_frame_info;
}

namespace ChillEngine::graphics::d3d12::gpass
{
    bool initialize();
    void shutdown();

    void set_size(math::u32v2 size);
    void depth_prepass(ID3D12GraphicsCommandList6* cmd_list, const d3d12_frame_info& info);
    void render(ID3D12GraphicsCommandList6* cmd_list, const d3d12_frame_info& info);

    void add_transitions_for_depth_prepass(d3dx::d3d12_resource_barrier& barriers);
    void add_transitions_for_gpass(d3dx::d3d12_resource_barrier& barriers);
    void add_transitions_for_post_process(d3dx::d3d12_resource_barrier& barriers);

    void set_render_targets_for_depth_prepass(ID3D12GraphicsCommandList6* cmd_list);
    void set_render_targets_for_gpass(ID3D12GraphicsCommandList6* cmd_list);
}


