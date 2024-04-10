#pragma once


#include "../../Common/CommonHeaders.h"
#include "../../Graphics/Render.h"
#include "../../Utilities/Utilities.h"

#include <dxgi1_6.h> //ms把他从directX解耦出来，处理adapter，输出设备，交换链等
#include <d3d12.h> // 核心图形功能
#include <wrl.h> // d3d12 使用组件对象模型实现，提供了ComPtr访问组件


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace ChillEngine::graphics::d3d12
{
    constexpr u32 frame_buffer_count = 3;
}


// Assert that COM call to D3D API succeeded
#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)                                   \
if(FAILED(x)) {                                     \
    char line_number[32];                           \
    sprintf_s(line_number, "%u", __LINE__);         \
    OutputDebugStringA("Error in: ");               \
    OutputDebugStringA(__FILE__);                   \
    OutputDebugStringA("\nLine: ");                 \
    OutputDebugStringA(line_number);                \
    OutputDebugStringA("\n");                       \
    OutputDebugStringA(#x);                         \
    OutputDebugStringA("\n");                       \
    __debugbreak();                                 \
}
#endif // !DXCall
#else
#ifndef DXCall
#define DXCall(x) x
#endif // !DXCall
#endif // _DEBUG


#ifdef _DEBUG
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name);\
OutputDebugString(L"::D3D12 object Created");\
OutputDebugString(name);\
OutputDebugString(L"\n");

#define NAME_D3D12_OBJECT_Indexed(obj, idx, name)           \
{                                                           \
    wchar_t full_name[128];                                 \
    if(swprintf_s(full_name, L"%s[%u]", name, idx) > 0)     \
    {                                                       \
        obj->SetName(full_name);                            \
        OutputDebugString(L"::D3D12 object Created");       \
        OutputDebugString(full_name);                       \
        OutputDebugString(L"\n");                           \
    }                                                       \
}
#else
#define NAME_D3D12_OBJECT(obj, name)
#define NAME_D3D12_OBJECT_Indexed(obj, idx, name)
#endif
