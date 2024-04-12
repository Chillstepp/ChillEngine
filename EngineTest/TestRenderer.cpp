#include "Test.h"
#include "../Engine/Graphics/Direct3D12/D3D12Core.h"
#if TEST_RENDERER
#include "TestRenderer.h"
#include "../Engine/Platform/PlatformTypes.h"
#include "../Engine/Platform/Platform.h"
#include "../Engine/Graphics/Render.h"
#include "ShaderCompilation.h"


using namespace ChillEngine;

graphics::render_surface _surface[4];
time_it timer{};

void destroy_render_surface(graphics::render_surface& surface);


LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        {
            bool all_closed{ true };
            for (u32 i{ 0 }; i < _countof(_surface); ++i)
            {
                if(_surface[i].window.is_valid())
                {
                    if (_surface[i].window.is_closed())
                    {
                        destroy_render_surface(_surface[i]);
                    }
                    else
                    {
                        all_closed = false;
                    }                    
                }

            }
            if (all_closed)
            {
                PostQuitMessage(0);
                return 0;
            }
        }
        break;
    case WM_SYSCHAR:
        //alt + enter: full screen
        if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
        {
            platform::window win{ platform::window_id{(id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
            win.set_fullscreen(!win.is_fullscreen());
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if(wparam == VK_ESCAPE)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void create_render_surface(graphics::render_surface& surface, platform::window_init_info info)
{
    surface.window = platform::create_window(&info);
    surface.surface = graphics::create_surface(surface.window);
}

void destroy_render_surface(graphics::render_surface& surface)
{
    graphics::render_surface temp {surface};
    surface = {};
    if(temp.surface.is_valid()) graphics::remove_surface(temp.surface.get_id());
    if(temp.window.is_valid()) platform::remove_window(temp.window.get_id());
}

bool engine_test::initialize()
{
    while (!compile_shaders())
    {
        //pop up a message box that allow the user to retry compilation
        //if not click retry, just return false
        if(MessageBox(nullptr, L"Failed To Compile Engine Shader", L"Shader Compilation Error", MB_RETRYCANCEL) != IDRETRY)
        {
            return false;
        }
    }
    
    bool result = graphics::initialize(graphics::graphics_platform::direct3d12);
    if(!result) return false;
    
    platform::window_init_info info[]
    {
        {&win_proc, nullptr, L"Test window 1", 100, 100, 400, 800},
        {&win_proc, nullptr, L"Test window 2", 150, 150, 800, 400},
        {&win_proc, nullptr, L"Test window 3", 200, 200, 400, 400},
        {&win_proc, nullptr, L"Test window 4", 250, 250, 800, 600}
    };
    static_assert(_countof(info) == _countof(_surface));

    for (u32 i = 0; i < _countof(_surface); ++i)
        create_render_surface(_surface[i], info[i]);
    return true;
}

void engine_test::run()
{
    timer.begin();
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    for(u32 i = 0; i < _countof(_surface); ++i)
    {
        if(_surface[i].surface.is_valid())
        {
            _surface[i].surface.render();
        }
    }
    timer.end();
}

void engine_test::shutdown()
{
    for (u32 i{ 0 }; i < _countof(_surface); ++i)
    {
        destroy_render_surface(_surface[i]);
    }
    graphics::shutdown();
}
#endif
