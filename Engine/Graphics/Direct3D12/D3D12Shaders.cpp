#include "D3D12Shaders.h"

#include "../../Content/ContentLoader.h"


namespace ChillEngine::graphics::d3d12::shaders
{
    namespace
    {
        struct compiled_shader
        {
            u64 size;
            const u8* byte_code;
        };
        using compiled_shader_ptr = compiled_shader const *;
        compiled_shader_ptr engine_shaders[engine_shader::count] {};
        
        //a chunk of memory which contains all compiled engine shaders
        //The blob is an array of shader byte code which contains a u64 size and an array of bytes
        std::unique_ptr<u8[]> shaders_blob{};
        
        bool load_engine_shaders()
        {
            assert(!shaders_blob);
            u64 size{ 0 };
            bool result{ content::load_engine_shaders(shaders_blob, size) };
            assert(shaders_blob && size);

            u64 offset{ 0 };
            u32 index{ 0 };
            while (offset < size && result)
            {
                assert(index < engine_shader::count);
                //对第index个engine shader进行操作
                compiled_shader_ptr& shader = engine_shaders[index];
                assert(!shader);
                result &= index < engine_shader::count && !shader;
                if (!result) break;
                shader = reinterpret_cast<const compiled_shader_ptr>(&shaders_blob[offset]);
                offset += sizeof(u64) + shader->size;
                ++index;
            }
            assert(offset == size && index == engine_shader::count);

            return result;
        }

        
    }

    bool initialize()
    {
        return load_engine_shaders();
    }

    void shutdown()
    {
        for(u32 i = 0; i < engine_shader::count; ++i)
        {
            engine_shaders[i] = {};
        }
        shaders_blob.reset();
    }

    D3D12_SHADER_BYTECODE get_engine_shader(engine_shader::id id)
    {
        assert(id < engine_shader::count);
        const compiled_shader_ptr shader = engine_shaders[id];
        assert(shader && shader->size);
        return { &shader->byte_code, shader->size };
    }

}
