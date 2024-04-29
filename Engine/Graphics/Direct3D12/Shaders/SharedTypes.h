#pragma once


#include "../D3D12Common.h"

namespace ChillEngine::graphics::d3d12::hlsl
{
    using float4x4 = math::m4x4a;
    using float4 = math::v4;
    using float3 = math::v3;
    using float2 = math::v2;
    using uint = u32;
    using uint4 = math::u32v4;
    using uint3 = math::u32v3;
    using uint2 = math::u32v2;
    
    #include "CommonTypes.hlsli"
}
