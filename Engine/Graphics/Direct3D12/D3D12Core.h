#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12 { //forward declaration
    struct d3d12_frame_info
    {
        u32 surface_width;
        u32 surface_height;
    };
}

namespace ChillEngine::graphics::d3d12::core
{
    
    bool initialize();
    void shutdown();
    void render();

    template<typename T>
    constexpr void release(T*& resource)
    {
        if(resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    namespace detail
    {
        void deferred_release(IUnknown* resource);
        
    }
    
    template<typename T>
    constexpr void deferred_release(T*& resource)
    {
        if(resource)
        {
            detail::deferred_release(resource);
            resource = nullptr;
        }
        
    }
    
    descriptor_heap& rtv_heap();
    descriptor_heap& srv_heap();
    descriptor_heap& uav_heap();
    descriptor_heap& dsv_heap();
    ID3D12Device8 *const device();
    void set_deferred_releases_flag();
    u32 current_frame_index();

    surface create_surface(platform::window window);
    void remove_surface(surface_id);
    void resize_surface(surface_id, u32, u32);
    u32 surface_width(surface_id id);
    u32 surface_height(surface_id id);
    void render_surface(surface_id id);
    
}
