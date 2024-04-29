#if !defined(CHILLENGINE_COMMON_HLSLI) && !defined(__cplusplus)
#define CHILLENGINE_COMMON_HLSLI
#error Do not include this header directly in shader.Only include this file via Common.hlsli
#endif

struct GlobalShaderData
{
    float4x4 View;
    float4x4 Projection;
    float4x4 InvProjection;
    float4x4 ViewProjection;
    float4x4 InvViewProjection;

    float3   CameraPosition;
    float    ViewWidth;

    float3   CameraDirection;
    float    ViewHeight;

    float DeltaTime;
};

struct PerObjectData
{
    float4x4 World;
    float4x4 InvWorld;
    float4x4 WorldViewProjection;
};

#ifdef __cpluscplus
static_assert((sizeof(PerObjectData) % 16) == 0, "Make sure perobjectdata is formatted in 16-byte chunks without any implicit padding.")
#endif