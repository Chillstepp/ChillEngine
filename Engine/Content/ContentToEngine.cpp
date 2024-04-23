#include "ContentToEngine.h"

namespace ChillEngine::content
{
    namespace
    {
        /* Data contains:
         * struct{
         *      u32 lod_count,
         *      struct{
         *          float lod_threshold,
         *          u32 submesh_count,
         *          u32 size_of_submeshes,
         *          struct{
         *              u32 element_size,
         *              u32 vertex_count,
         *              u32 index_count,
         *              u32 elements_type,
         *              u32 primitive_topology,
         *              u8 position[sizeof(f32) * 3 * vertex_count]; // sizeof(position) must be a multiple of 4 bytes.Pad if needed.
         *              u8 elements[sizeof(f32) * 3 * vertex_count];// sizeof(elements) must be a multiple of 4 bytes.Pad if needed.
         *              u8 indices[index_size * index_count];
         *          } submeshes[submesh_count]
         *      }mesh_lods[lod_count];
         *  }geometry;
        */

        /*  Output format:
         *  
         * 
         */

        id::id_type create_geometry_resource(const void* data)
        {
            
        }

        
    }

    

    id::id_type create_resource(const void* const data, asset_type::type type)
    {
        assert(data);
        id::id_type id = id::invalid_id;

        switch (type)
        {
        case asset_type::animation: break;
        case asset_type::material: break;
        case asset_type::mesh:  id = create_geometry_resource(data);  break;
        case asset_type::texture: break;
        }
        assert(id::is_valid(id));
        return id;
    }

    void destroy_resource(id::id_type id, asset_type::type type)
    {
        assert(id::is_valid(id));
        
        switch (type)
        {
        case asset_type::animation: break;
        case asset_type::material: break;
        case asset_type::mesh:  destroy_geometry_resource(id);  break;
        case asset_type::texture: break;
        default:
            assert(false);
            break;
        }
    }
}
