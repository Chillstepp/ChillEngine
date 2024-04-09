#include "D3D12Surface.h"

#include "D3D12Core.h"
#include "D3D12Resources.h"

namespace ChillEngine::graphics::d3d12
{
    namespace
    {
        //后备缓冲区不支持SRGB纹理格式，随意我们需要转到非srgb随后在view中以srgb再读出来
        constexpr DXGI_FORMAT to_non_srgb(DXGI_FORMAT format)
        {
            if(format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;
            return format;
        }
    }
    
    void d3d12_surface::create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* command_queue, DXGI_FORMAT format)
    {
        assert(factory && command_queue);
        release();

        if(SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &_allow_tearing, sizeof(u32))) && _allow_tearing)
        {
            _present_flag = DXGI_PRESENT_ALLOW_TEARING;
        }

        //diable tearing now.
        _allow_tearing = _present_flag = 0;

        DXGI_SWAP_CHAIN_DESC1 desc{};
        //RGBA中的A透明度
        desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.BufferCount = frame_buffer_count;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.Flags = _allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        desc.Format = to_non_srgb(format);
        desc.Height = _window.height();
        desc.Width = _window.width();
        //不需要任何的AA 
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        /*DXGI_SCALING 枚举定义了交换链的缩放模式，可以根据实际需求来选择合适的缩放行为。常见的 DXGI_SCALING 值包括：
        DXGI_SCALING_STRETCH：拉伸模式，即将后台缓冲区的内容拉伸到与前台缓冲区大小相匹配。
        DXGI_SCALING_NONE：非缩放模式，后台缓冲区和前台缓冲区大小不匹配时，后台缓冲区的内容会被裁剪或填充。
        DXGI_SCALING_ASPECT_RATIO_STRETCH：按纵横比拉伸模式，保持内容纵横比的前提下将内容拉伸到与前台缓冲区大小相匹配。
        DXGI_SCALING 主要用于指定交换链的缩放行为，在不同的应用场景中可以选择不同的缩放模式来达到最佳的显示效果和性能。选择合适的 DXGI_SCALING 类型可
        以确保图形渲染内容能够正确地显示在不同大小的显示器上，并提供最佳的用户体验。*/
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//我们不用上一帧的数据，直接discard
        desc.Stereo = false;//立体视觉(左右眼不同)

        IDXGISwapChain1* swap_chain;
        HWND hwnd((HWND)_window.handle());
        DXCall(factory->CreateSwapChainForHwnd(command_queue, hwnd, &desc, nullptr, nullptr, &swap_chain));
        DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
        DXCall(swap_chain->QueryInterface(IID_PPV_ARGS(&_swap_chain)));
        core::release(swap_chain);

        _current_backbuffer_index = _swap_chain->GetCurrentBackBufferIndex();

        //用rtv来分配缓冲区
        for(u32 i = 0; i < frame_buffer_count; ++i)
        {
            _render_target_data[i].rtv = core::rtv_heap().allocate();
        }

        finalize();
    }

    void d3d12_surface::present() const
    {
        assert(_swap_chain);
        DXCall(_swap_chain->Present(0, _present_flag)); //不垂直同步， 无任何呈现标志参数
        _current_backbuffer_index = _swap_chain->GetCurrentBackBufferIndex();
    }

    void d3d12_surface::resize()
    {
        
    }
    

    void d3d12_surface::finalize()
    {
        //create rtv for back_buffers
        for(u32 i = 0; i < frame_buffer_count; ++i)
        {
            render_target_data& data = _render_target_data[i];
            assert(!data.resource);
            //用于获取交换链中的后台缓冲区
            DXCall(_swap_chain->GetBuffer(i, IID_PPV_ARGS(&data.resource)));
            D3D12_RENDER_TARGET_VIEW_DESC desc{};
            desc.Format = core::get_default_render_target_format();
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            core::device()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
        }
        DXGI_SWAP_CHAIN_DESC desc{};
        DXCall(_swap_chain->GetDesc(&desc));
        const u32 width = desc.BufferDesc.Width;
        const u32 height = desc.BufferDesc.Height;
        assert(_window.width() == width && _window.height()==height);

        //set viewport and scissor rect
        _viewport.TopLeftX = 0.0f;
        _viewport.TopLeftY = 0.0f;
        _viewport.Width = static_cast<float>(width);
        _viewport.Height = static_cast<float>(height);
        _viewport.MinDepth = 0.0f;
        _viewport.MaxDepth = 1.0f;

        _scissor_rect = D3D12_RECT{ 0, 0, (LONG)width, (LONG)height };
    }

    void d3d12_surface::release()
    {
        for(u32 i = 0; i < frame_buffer_count; ++i)
        {
            render_target_data& data = _render_target_data[i];
            core::release(data.resource);
            core::rtv_heap().free(data.rtv);
        }
        core::release(_swap_chain);
    }
    

}
