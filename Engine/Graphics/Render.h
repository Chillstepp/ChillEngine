
#pragma once
#include "../Common/CommonHeaders.h"
#include "../Platform/Window.h"

namespace ChillEngine::graphics {

    DEFINE_TYPE_ID(surface_id);
    class surface
    {
    public:
        constexpr explicit surface(surface_id id) : _id{ id } {}
        constexpr surface() = default;
        constexpr surface_id get_id() const { return _id; }
        constexpr bool is_valid() const { return id::is_valid(_id); }
        
        void resize(u32 width, u32 height) const;
        u32 width() const;
        u32 height() const;
        void render() const;
    private:
        surface_id _id{ id::invalid_id };
    };

    struct render_surface
    {
        platform::window window{};
        surface surface{};
    };

    enum class graphics_platform : u32
    {
        direct3d12=0,
        vulkan = 1
    };
        
    bool initialize(graphics_platform platform);
    void shutdown();

    surface create_surface(platform::window window);
    void remove_surface(surface_id id);

    //get the location of compiled shaders relative to the executable's path.
    //The path is for the graphics API
    const char* get_engine_shaders_path();

    const char* get_engine_shaders_path(graphics_platform platform);

}
