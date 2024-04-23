#include "ContentToEngine.h"
#include "../Graphics/Render.h"

namespace ChillEngine::content
{
    namespace
    {
        
        class geometry_hierarchy_stream
        {
        public:
            struct lod_offset
            {
                u16 offset;
                u16 count;
            };

            DISABLE_COPY_AND_MOVE(geometry_hierarchy_stream);
            geometry_hierarchy_stream(u8 *const buffer, u32 lods = u32_invalid_id)
                : _buffer{ buffer }
            {
                assert(buffer && lods);
                if (lods != u32_invalid_id)
                {
                    *((u32*)buffer) = lods;
                }

                _lod_count = *((u32*)buffer);
                _thresholds = (f32*)(&buffer[sizeof(u32)]);
                _lod_offsets = (lod_offset*)(&_thresholds[_lod_count]);
                _gpu_ids = (id::id_type*)(&_lod_offsets[_lod_count]);
            }

            void gpu_ids(u32 lod, id::id_type*& ids, u32& id_count)
            {
                assert(lod < _lod_count);
                ids = (id::id_type*)(_buffer + _lod_offsets[lod].offset);
                id_count = _lod_offsets[lod].count;
            }

            u32 lod_from_threshold(f32 threshold)
            {
                assert(threshold > 0);

                for (u32 i{ _lod_count - 1 }; i > 0; --i)
                {
                    if (_thresholds[i] <= threshold) return i;
                }

                assert(false); // shouldn't ever get here.
                return 0;
            }

            [[nodiscard]] constexpr u32 lod_count() const { return _lod_count; }
            [[nodiscard]] constexpr f32* thresholds() const { return _thresholds; }
            [[nodiscard]] constexpr lod_offset* lod_offsets() const { return _lod_offsets; }
            [[nodiscard]] constexpr id::id_type* gpu_ids() const { return _gpu_ids; }

        private:
            u8 *const       _buffer;
            f32*            _thresholds;
            lod_offset*     _lod_offsets;
            id::id_type*    _gpu_ids;
            u32             _lod_count;
        };

        
        utl::free_list<u8*> geometry_hierarchies;
        std::mutex geometry_mutex;

        // NOTE: expects the same data as create_geometry_resource()
        //拿到geometry hierarchy的buffer大小: 
        u32 get_geometry_hierarchy_buffer_size(const void *const data)
        {
            assert(data);
            utl::blob_stream_reader blob{ (const u8*)data };
            const u32 lod_count{ blob.read<u32>() };
            assert(lod_count);
            // add size of  lod_count, thresholds and lod offsets to the size of hierarchy.
            u32 size = sizeof(u32) + (sizeof(f32) + sizeof(geometry_hierarchy_stream::lod_offset)) * lod_count;

            for (u32 lod_idx = 0; lod_idx < lod_count; ++lod_idx)
            {
                // skip threshold
                blob.skip(sizeof(f32));
                // add size of gpu_ids (sizeof(id::id_type) * submesh_count)
                size += sizeof(id::id_type) * blob.read<u32>();
                // skip submesh data and go to the next LOD
                blob.skip(blob.read<u32>());
            }

            return size;
        }

        

        // Creates a hierarchy stream for a geometry that has multiple LODs and/or multiple submeshes.
        // NOTE: expects the same data as create_geometry_resource()
        id::id_type create_mesh_hierarchy(const void *const data)
        {
            assert(data);
            const u32 size{ get_geometry_hierarchy_buffer_size(data) };
            u8 *const hierarchy_buffer{ (u8 *const)malloc(size) };

            utl::blob_stream_reader blob{ (const u8*)data };
            const u32 lod_count{ blob.read<u32>() };
            assert(lod_count);
            geometry_hierarchy_stream stream{ hierarchy_buffer, lod_count };
            u16 submesh_index{ 0 };
            id::id_type *const gpu_ids{ stream.gpu_ids() };

            for (u32 lod_idx{ 0 }; lod_idx < lod_count; ++lod_idx)
            {
                stream.thresholds()[lod_idx] = blob.read<f32>();
                const u32 id_count{ blob.read<u32>() };
                assert(id_count < (1 << 16));
                stream.lod_offsets()[lod_idx] = { submesh_index, (u16)id_count };
                blob.skip(sizeof(u32)); // skip over size_of_submeshes
                for (u32 id_idx{ 0 }; id_idx < id_count; ++id_idx)
                {
                    const u8* at{ blob.position() };
                    gpu_ids[submesh_index++] = graphics::add_submesh(at);
                    blob.skip((u32)(at - blob.position()));
                    assert(submesh_index < (1 << 16));
                }
            }

            assert([&]() {
                f32 previous_threshold{ stream.thresholds()[0] };
                for (u32 i{ 1 }; i < lod_count; ++i)
                {
                    if (stream.thresholds()[i] <= previous_threshold) return false;
                    previous_threshold = stream.thresholds()[i];
                }
                return true;
                   }());

            static_assert(alignof(void*) > 2, "We need the least significant bit for the single mesh marker.");
            std::lock_guard lock{ geometry_mutex };
            return geometry_hierarchies.add(hierarchy_buffer);
        }

        // Creates a single submesh gpu_id
        // NOTE: expects the same data as create_geometry_resource()
        id::id_type create_single_submesh(const void *const data)
        {
            assert(data);
            utl::blob_stream_reader blob{ (const u8*)data };
            // skip lod_count, lod_threshold, submesh_count and size_of_submeshes
            blob.skip(sizeof(u32) + sizeof(f32) + sizeof(u32) + sizeof(u32));
            const u8* at{ blob.position() };
            const id::id_type gpu_id{ graphics::add_submesh(at) };

            // create a fake pointer and put it in the geometry_hierarchies.
            static_assert(sizeof(uintptr_t) > sizeof(id::id_type));
            constexpr u8 shift_bits{ (sizeof(uintptr_t) - sizeof(id::id_type)) << 3 };
            u8 *const fake_pointer{ (u8 *const)((((uintptr_t)gpu_id) << shift_bits) | single_mesh_marker)};
            std::lock_guard lock{ geometry_mutex };
            return geometry_hierarchies.add(fake_pointer);
        }
        
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

        /*  Expect Output format: this is more cache-friendly.
         *  struct{
         *      u32 lod_count,
         *      f32 threshold[lod_count]
         *      struct {
         *          u16 offset,
         *          u16 count,
         *      }lod_offset[lod_count],
         *      id::id_type gpu_ids[total_number_of_submeshes];
         *  } geometry_hierarchy
         *
         * If geometry has a single LOD and submesh:
         *   (gpu_id << 32) | 0x01
         */

        id::id_type
        create_geometry_resource(const void *const data)
        {
            assert(data);
            return is_single_mesh(data) ? create_single_submesh(data) : create_mesh_hierarchy(data);
        }

        void
        destroy_geometry_resource(id::id_type id)
        {
            std::lock_guard lock{ geometry_mutex };
            u8 *const  pointer{ geometry_hierarchies[id] };
            if ((uintptr_t)pointer & single_mesh_marker)
            {
                graphics::remove_submesh(gpu_id_from_fake_pointer(pointer));
            }
            else
            {
                geometry_hierarchy_stream stream{ pointer };
                const u32 lod_count{ stream.lod_count() };
                u32 id_index{ 0 };
                for (u32 lod{ 0 }; lod < lod_count; ++lod)
                {
                    for (u32 i{ 0 }; i < stream.lod_offsets()[lod].count; ++i)
                    {
                        graphics::remove_submesh(stream.gpu_ids()[id_index++]);
                    }
                }

                free(pointer);
            }

            geometry_hierarchies.remove(id);
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