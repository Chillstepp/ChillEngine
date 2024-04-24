#include "D3D12Content.h"

#include "D3D12Core.h"
#include "../../Content/ContentToEngine.h"

namespace ChillEngine::graphics::d3d12::content
{
    namespace
    {
        struct submesh_view
        {
            D3D12_VERTEX_BUFFER_VIEW        position_buffer_view{};
            D3D12_INDEX_BUFFER_VIEW         index_buffer_view{};
            D3D12_VERTEX_BUFFER_VIEW        element_buffer_view{};
            u32                             element_type{};
            D3D_PRIMITIVE_TOPOLOGY          primitive_topology;
        };

        utl::free_list<ID3D12Resource*>     submesh_buffers{};
        utl::free_list<submesh_view>        submesh_views{};
        std::mutex                          submesh_mutex{};


        D3D_PRIMITIVE_TOPOLOGY get_d3d_primitive_topology(primitive_topology::type type)
        {
            using namespace ChillEngine::content;
            assert(type < primitive_topology::count);

            switch (type)
            {
                case primitive_topology::point_list:     return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case primitive_topology::line_list:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case primitive_topology::line_strip:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case primitive_topology::triangle_list:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case primitive_topology::triangle_strip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            }

            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
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
            //now, gpu has data!
            ID3D12Resource* resource = d3dx::create_buffer(blob.position(), total_buffer_size);

            blob.skip(total_buffer_size);
            data = blob.position();

            submesh_view view{};
            view.position_buffer_view.BufferLocation = resource->GetGPUVirtualAddress();
            view.position_buffer_view.SizeInBytes = position_buffer_size;
            view.position_buffer_view.StrideInBytes = sizeof(math::v3);

            if(element_size)
            {
                view.element_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size;
                view.element_buffer_view.SizeInBytes = element_buffer_size;
                view.element_buffer_view.StrideInBytes = element_size;
            }
            
            view.index_buffer_view.BufferLocation = resource->GetGPUVirtualAddress() + aligned_position_buffer_size + aligned_element_buffer_size;
            view.index_buffer_view.SizeInBytes = indices_buffer_size;
            view.index_buffer_view.Format = (index_size == sizeof(u16)) ?  DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT ;
            
            view.element_type = element_type;
            view.primitive_topology = get_d3d_primitive_topology((primitive_topology::type)primitive_topology);

            std::lock_guard lock{submesh_mutex};
            submesh_buffers.add(resource);
            return submesh_views.add(view);
        }

        void remove(id::id_type id)
        {
            std::lock_guard lock{submesh_mutex};
            submesh_views.remove(id);

            core::deferred_release(submesh_buffers[id]);
            submesh_buffers.remove(id);
        }
    }
}