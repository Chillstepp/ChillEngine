#pragma once

#include "D3D12Common.h"
#include "D3D12Resources.h"


namespace ChillEngine::graphics::d3d12
{
    class d3d12_surface
    {
    public:
        constexpr static u32 buffer_count = 3;
        constexpr static DXGI_FORMAT default_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        explicit d3d12_surface(platform::window window) : _window(window)
        {
            assert(_window.handle());
            
        }
#if USE_STL_VECTOR
        DISABLE_COPY(d3d12_surface);
        d3d12_surface(d3d12_surface&& o) noexcept:
            _swap_chain(o._swap_chain),_window(o._window), _current_backbuffer_index(o._current_backbuffer_index),
            _viewport(o._viewport), _scissor_rect(o._scissor_rect), _allow_tearing(o._allow_tearing), _present_flag(o._present_flag)
        {
            for(u32 i = 0; i < buffer_count; ++i)
            {
                _render_target_data[i].resource = o._render_target_data[i].resource;
                _render_target_data[i].rtv = o._render_target_data[i].rtv;
            }
            o.reset();
        }

        d3d12_surface& operator = (d3d12_surface&& o) noexcept
        {
            assert(this != std::addressof(o));
            if(this != &o)
            {
                release();
                move(o);
            }
        }


        constexpr void reset()
        {
            _swap_chain = nullptr;
            for(u32 i = 0; i < buffer_count; ++i)
            {
                _render_target_data[i] = {};
            }
            _window = {};
            _current_backbuffer_index = 0;
            _viewport = {};
            _scissor_rect = {};
            _allow_tearing = 0;
            _present_flag = 0;
        }

        void move(d3d12_surface& o)
        {
            _swap_chain = o._swap_chain;
            for(u32 i = 0; i < buffer_count; ++i)
            {
                _render_target_data[i] = o._render_target_data[i];
            }
            _window = o._window;
            _current_backbuffer_index = o._current_backbuffer_index;
            _viewport = o._viewport;
            _scissor_rect = o._scissor_rect;
            _allow_tearing = o._allow_tearing;
            _present_flag = o._present_flag;
        }
#else
        DISABLE_COPY_AND_MOVE(d3d12_surface)
#endif
        
        ~d3d12_surface() {release();}

        /**
         * \brief 
         * \param factory 
         * \param command_queue 
         * \param format: what format to render.
         */
        void create_swap_chain(IDXGIFactory7* factory, ID3D12CommandQueue* command_queue, DXGI_FORMAT format = default_back_buffer_format);
        void present() const;
        void resize();

        constexpr u32 width() const {return (u32)_viewport.Width;}
        constexpr u32 height() const {return (u32)_viewport.Height;}
        constexpr ID3D12Resource* const back_buffer() const {return _render_target_data[_current_backbuffer_index].resource;}
        constexpr D3D12_CPU_DESCRIPTOR_HANDLE rtv() const {return _render_target_data[_current_backbuffer_index].rtv.cpu;}
        constexpr const D3D12_VIEWPORT& viewport() const {return _viewport;}
        constexpr const D3D12_RECT& scissor_rect() const {return _scissor_rect;}
    private:
        void release();
        //create render target view and put it in back buffer
        void finalize();

        struct render_target_data
        {
            ID3D12Resource* resource = nullptr;
            descriptor_handle rtv{};
        };

        IDXGISwapChain4*    _swap_chain = nullptr;
        render_target_data  _render_target_data[buffer_count]{};
        platform::window    _window{};
        DXGI_FORMAT         _format{default_back_buffer_format};//后备缓冲区的格式
        mutable  u32        _current_backbuffer_index = 0;
        D3D12_VIEWPORT      _viewport{};//视口是用于显示和渲染的区域，控制了渲染目标中哪部分内容会被渲染到屏幕上。
        D3D12_RECT          _scissor_rect{};// 通常用于指定渲染目标或纹理中的矩形区域，以控制渲染或处理的区域范围。
        u32                 _allow_tearing = 0;
        u32                 _present_flag = 0;
    };
}
