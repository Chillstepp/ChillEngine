#include "D3D12Content.h"


namespace ChillEngine::graphics::d3d12::content
{
    namespace submesh
    {
        /* Data contains:
         *  u32 element_size, u32 vertex_count,
         *  u32 index_count, u32 elements_type, u32 primitive_topology,
         *  u8 position[sizeof(f32) * 3 * vertex_count];
         *  u8 elements[sizeof(f32) * 3 * vertex_count];
         *  u8 indices[index_size * index_count];
         */
        id::id_type add(const u8*& data)
        {
            
        }

        void remove(id::id_type id)
        {
            
        }
    }
}