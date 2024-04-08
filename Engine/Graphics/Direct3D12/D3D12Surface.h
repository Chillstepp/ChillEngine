#pragma once

#include "D3D12Common.h"

namespace ChillEngine::graphics::d3d12
{
    class d3d12_surface
    {
    public:
        explicit d3d12_surface(platform::window window) : _window(window)
        {
            assert(_window.handle());
            
        }
        ~d3d12_surface() {release();}

        /**
         * \brief 
         * \param factory 
         * \param command_queue 
         * \param format: what format to render.
         */
        void create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* command_queue, DXGI_FORMAT format);
        void present() const;
        void resize();

        u32 width() const {}
        u32 height() const {}
    private:
        void release();

        IDXGISwapChain4* _swap_chain = nullptr;
        platform::window _window;
        u32              _current_backbuffer_index = 0;
    };
}
