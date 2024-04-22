#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12::upload
{
    class d3d12_upload_context
    {
    public:
        DISABLE_COPY_AND_MOVE(d3d12_upload_context);
        d3d12_upload_context(u32 aligned_size);
        ~d3d12_upload_context() {assert(_frame_index == u32_invalid_id); }
        void end_upload();

        constexpr ID3D12GraphicsCommandList6* const command_list() const {return _cmd_list;}
        constexpr ID3D12Resource* const upload_buffer() const { return _upload_buffer;}
        constexpr void *const cpu_address() const { return _cpu_address;}
   
    private:
        DEBUG_OP(d3d12_upload_context() = default);
        ID3D12GraphicsCommandList6*     _cmd_list = nullptr;
        ID3D12Resource*                 _upload_buffer = nullptr;
        void*                           _cpu_address = nullptr;
        u32                             _frame_index = u32_invalid_id;
    };

    bool initialize();
    void shutdown();
}
