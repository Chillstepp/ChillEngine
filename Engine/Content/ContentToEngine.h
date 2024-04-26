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

    struct lod_offset
    {
        u16 offset;
        u16 count;
    };
    
    id::id_type create_resource(const void *const data, asset_type:: type);
    void destroy_resource(id::id_type id, asset_type::type type);

    id::id_type add_shader(const u8* data);
    void remove_shader(id::id_type id);
    compiled_shader_ptr get_shader(id::id_type id);

    void get_submesh_gpu_ids(id::id_type geometry_content_id, u32 id_count, id::id_type* gpu_ids);
    void get_lod_offset(const id::id_type *const geometry_ids, const f32 *const thresholds, u32 id_count, utl::vector<lod_offset>& offsets);
}
