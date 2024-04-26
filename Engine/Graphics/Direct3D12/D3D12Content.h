#pragma once

#include "D3D12Common.h"


namespace ChillEngine::graphics::d3d12::content
{

    bool initialize();
    void shutdown();
    
    namespace submesh
    {
        struct views_cache
        {
            D3D12_GPU_VIRTUAL_ADDRESS *const posistion_buffers;
            D3D12_GPU_VIRTUAL_ADDRESS *const element_buffers;
            D3D12_INDEX_BUFFER_VIEW   *const index_buffer_views;
            D3D_PRIMITIVE_TOPOLOGY    *const primitive_topologies;
            u32                       *const elements_types;      
        };
        id::id_type add(const u8*& data);
        void remove(id::id_type id);
        void get_views(const id::id_type *const gpu_ids, u32 id_count, const views_cache& cache);
    }

    namespace texture
    {
        id::id_type add(const u8* const);
        void remove(id::id_type id);
        void get_descriptor_indices(const id::id_type *const texture_ids, u32 id_count, u32 *const indices);
    }

    namespace material
    {
        struct materials_cache
        {
            ID3D12RootSignature* *const root_signatures;
            material_type::type *const material_types;
        };
        id::id_type add(material_init_info info);
        void remove(id::id_type id);

        void get_materials(const id::id_type *const material_ids, u32 material_count, const materials_cache& cache);
    }
}
