#include "D3D12Content.h"


namespace ChillEngine::graphics::d3d12::content
{
    namespace
    {
        struct position_view
        {
            D3D12_VERTEX_BUFFER_VIEW        position_buffer_view{};
            D3D12_INDEX_BUFFER_VIEW         index_buffer_view{};
        };

        struct elements_view
        {
            D3D12_VERTEX_BUFFER_VIEW        element_buffer_view{};
            u32                             element_type{};
            D3D_PRIMITIVE_TOPOLOGY          primitive_topology;
        };
    }
    
    namespace submesh
    {
        /* Data contains:
         *  u32 element_size,
         *  u32 vertex_count,
         *  u32 index_count,
         *  u32 elements_type,
         *  u32 primitive_topology,
         *  u8 position[sizeof(f32) * 3 * vertex_count]; // sizeof(position) must be a multiple of 4 bytes.Pad if needed.
         *  u8 elements[sizeof(f32) * 3 * vertex_count];// sizeof(elements) must be a multiple of 4 bytes.Pad if needed.
         *  u8 indices[index_size * index_count];
         */


        /*
         * Remarks:
         *  - Advances the data pointer
         *  - Position and element buffers should be padded to a multiple of 4 bytes in length
         *    which is defined as D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE
         */
        id::id_type add(const u8*& data)
        {
            utl::blob_stream_reader blob{(const u8*)data};
            const u32 element_size = blob.read<u32>();
            const u32 vertex_count = blob.read<u32>();
            const u32 index_count = blob.read<u32>();
            const u32 element_type = blob.read<u32>();
            const u32 primitive_topology = blob.read<u32>();

            //
            const u32 index_size = vertex_count < (1<<16) ? sizeof(u16) : sizeof(u32);
            
            const u32 position_buffer_size = sizeof(math::v3) * vertex_count;
            const u32 element_buffer_size = element_size * vertex_count;
            const u32 indices_buffer_size = index_size * index_count;

            
            constexpr u32 alignment = D3D12_STANDARD_MAXIMUM_ELEMENT_ALIGNMENT_BYTE_MULTIPLE;
            const u32 aligned_position_buffer_size = static_cast<u32>(math::align_size_up<alignment>(position_buffer_size));
            const u32 aligned_element_buffer_size = static_cast<u32>(math::align_size_up<alignment>(element_buffer_size));
            const u32 total_buffer_size = aligned_position_buffer_size + aligned_element_buffer_size + indices_buffer_size;
            ID3D12Resource* resource = d3dx::create_buffer(blob.position(), total_buffer_size);
        }

        void remove(id::id_type id)
        {
            
        }
    }
}