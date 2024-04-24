#include <filesystem>

#include "ShaderCompilation.h"
#include "../Engine/Common/CommonHeaders.h"
#include "../Engine/Content/ContentToEngine.h"

using namespace ChillEngine;

bool read_file(std::filesystem::path, std::unique_ptr<u8[]>&, u64&);

namespace
{

    id::id_type model_id{id::invalid_id};
    id::id_type vs_id{id::invalid_id};
    id::id_type ps_id{id::invalid_id};

    std::unordered_map<id::id_type, id::id_type> render_item_entity_map;

    void
    load_model()
    {
        std::unique_ptr<u8[]> model;
        u64 size{ 0 };
        read_file("..\\EngineTest\\model.model", model, size);

        model_id = content::create_resource(model.get(), content::asset_type::mesh);
        assert(id::is_valid(model_id));
    }

    void load_shaders()
    {
        // Let's say our material uses a vertex shader and a pixel shader.
        shader_file_info info{};
        info.file_name = "TestShader.hlsl";
        info.function = "TestShaderVS";
        info.type = shader_type::vertex;

        const char* shader_path{ "..\\enginetest\\" };

        auto vertex_shader = compile_shader(info, shader_path);
        assert(vertex_shader.get());

        info.function = "TestShaderPS";
        info.type = shader_type::pixel;

        auto pixel_shader = compile_shader(info, shader_path);
        assert(pixel_shader.get());

        vs_id = content::add_shader(vertex_shader.get());
        ps_id = content::add_shader(pixel_shader.get());
    }

}


id::id_type create_render_item(id::id_type entity_id)
{
    //load a model, pretend it belongs to entity_id;
    auto _1 = std::thread([]{load_model();});
    // load a material:
    // 1. load textures
    // 2. load shaders for that material
    auto _2 = std::thread([]{load_shaders();});

    _1.join();
    _2.join();

    // add a render item using the model and its material.
    return id::invalid_id;
}

void destroy_render_item(id::id_type item_id)
{
    // remove the render item from engine

    // remove material

    // remove shaders and textures

    // remove model
}