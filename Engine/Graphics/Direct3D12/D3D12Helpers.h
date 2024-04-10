#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12::d3dx
{
    constexpr struct
    {
        const D3D12_HEAP_PROPERTIES default_heap
        {
            D3D12_HEAP_TYPE_DEFAULT,// D3D12_HEAP_TYPE Type;
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,// D3D12_CPU_PAGE_PROPERTY CPUPageProperty;
            D3D12_MEMORY_POOL_UNKNOWN, // D3D12_MEMORY_POOL MemoryPoolPreference;
            0,// UINT CreationNodeMask
            0// UINT VisibleNodeMask
        };
    }heap_properties;
    
}
