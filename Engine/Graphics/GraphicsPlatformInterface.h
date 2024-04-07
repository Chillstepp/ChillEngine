#pragma once

namespace ChillEngine::graphics
{
    struct platform_interface
    {
        bool (*initialize)(void);
        void (*shutdown)(void);
        void (*render)(void);
    };
}
