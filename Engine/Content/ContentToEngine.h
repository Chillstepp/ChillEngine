#pragma once

#include "../Common/CommonHeaders.h"

namespace ChillEngine::content
{
    struct asset_type
    {
        enum type : u32
        {
            unknown = 0,
            animation,
            material,
            mesh,
            texture,

            count
        };  
    };

    struct compiled_shader
    {
        static constexpr u32 hash_length = 16;
        constexpr u64 byte_code_size() const {return _byte_code_size;}
        constexpr const u8 *const hash() const {return _hash;}
        constexpr const u8 *const byte_code() const {return &_byte_code;}
    private:
        u64         _byte_code_size;
        u8          _hash[hash_length];
        u8          _byte_code;
    };
    using compiled_shader_ptr = compiled_shader const *;

    id::id_type create_resource(const void *const data, asset_type:: type);
    void destroy_resource(id::id_type id, asset_type::type type);

    id::id_type add_shader(const u8* data);
    void remove_shader(id::id_type id);
    compiled_shader_ptr get_shader(id::id_type id);
    
}
