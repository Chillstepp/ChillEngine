#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12
{
    struct d3d12_frame_info;
}

namespace ChillEngine::graphics::d3d12::gpass
{
    constexpr DXGI_FORMAT       main_buffer_format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    constexpr DXGI_FORMAT       depth_buffer_format = DXGI_FORMAT_D32_FLOAT;
    
    struct opaque_root_parameter {
        enum parameter : u32 {
            per_frame_data,
            position_buffer,
            element_buffer,
            srv_indices,
            per_object_data,

            count
        };
    };
    
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

    const d3d12_render_texture& main_buffer();

    const d3d12_depth_buffer& depth_buffer();

}


