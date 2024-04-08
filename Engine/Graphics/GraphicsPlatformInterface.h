#pragma once

namespace ChillEngine::graphics
{
    struct platform_interface
    {
        bool (*initialize)(void);
        void (*shutdown)(void);
        void (*render)(void);

        struct{
            surface(*create) (platform::window);
            void(*remove)(surface_id);
            void(*resize)(surface_id, u32, u32);
            u32(*width)(surface_id);
            u32(*height)(surface_id);
            u32(*render)(surface_id);
        } surface;
    };
    
}
