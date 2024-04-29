#include "D3D12Gpass.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"
#include "D3D12Content.h"

namespace ChillEngine::graphics::d3d12::gpass
{
    namespace
    {
        struct gpass_root_param_indices
        {
            enum : u32 {
                root_constants,
                count
            };
        };
        constexpr math::u32v2       initial_dimensions{100, 100};
        
        d3d12_render_texture        gpass_main_buffer{};
        d3d12_depth_buffer          gpass_depth_buffer{};
        math::u32v2                 dimensions{ initial_dimensions };

        ID3D12RootSignature*        gpass_root_sig = nullptr;
        ID3D12PipelineState*        gpass_pso = nullptr;
#if _DEBUG
        constexpr f32               clear_value[4]{0.5f, 0.5f, 0.5f, 1.0f};
#else
        constexpr f32               clear_value[4]{};
#endif


        struct gpass_cache
        {
            utl::vector<id::id_type>    d3d12_render_item_ids;

            // NOTE: when adding new arrays, make sure to update resize() and struct_size.
            id::id_type*                entity_ids{ nullptr };
            id::id_type*                submesh_gpu_ids{ nullptr };
            id::id_type*                material_ids{ nullptr };
            ID3D12PipelineState**       gpass_pipeline_states{ nullptr };
            ID3D12PipelineState**       depth_pipeline_states{ nullptr };
            ID3D12RootSignature**       root_signatures{ nullptr };
            material_type::type*        material_types{ nullptr };
            D3D12_GPU_VIRTUAL_ADDRESS*  position_buffers{ nullptr };
            D3D12_GPU_VIRTUAL_ADDRESS*  element_buffers{ nullptr };
            D3D12_INDEX_BUFFER_VIEW*    index_buffer_views{ nullptr };
            D3D_PRIMITIVE_TOPOLOGY*     primitive_topologies{ nullptr };
            u32*                        elements_types{ nullptr };
            D3D12_GPU_VIRTUAL_ADDRESS*  per_object_data{ nullptr };

            constexpr content::render_item::items_cache items_cache() const
            {
                return{
                    entity_ids,
                    submesh_gpu_ids,
                    material_ids,
                    gpass_pipeline_states,
                    depth_pipeline_states
                };
            }

            constexpr content::submesh::views_cache views_cache() const
            {
                return{
                    position_buffers,
                    element_buffers,
                    index_buffer_views,
                    primitive_topologies,
                    elements_types
                };
            }

            constexpr content::material::materials_cache materials_cache() const
            {
                return{
                    root_signatures,
                    material_types
                };
            }

            constexpr u32 size() const
            {
                return (u32)d3d12_render_item_ids.size();
            }

            constexpr void clear()
            {
                d3d12_render_item_ids.clear();
            }

            constexpr void resize()
            {
                const u64 items_count{ d3d12_render_item_ids.size() };
                const u64 new_buffer_size{ items_count * struct_size };
                const u64 old_buffer_size{ _buffer.size() };
                if (new_buffer_size > old_buffer_size)
                {
                    _buffer.resize(new_buffer_size);
                }

                if (new_buffer_size != old_buffer_size)
                {
                    entity_ids = (id::id_type*)_buffer.data();
                    submesh_gpu_ids = (id::id_type*)(&entity_ids[items_count]);
                    material_ids = (id::id_type*)(&submesh_gpu_ids[items_count]);
                    gpass_pipeline_states = (ID3D12PipelineState**)(&material_ids[items_count]);
                    depth_pipeline_states = (ID3D12PipelineState**)(&gpass_pipeline_states[items_count]);
                    root_signatures = (ID3D12RootSignature**)(&depth_pipeline_states[items_count]);
                    material_types = (material_type::type*)(&root_signatures[items_count]);
                    position_buffers = (D3D12_GPU_VIRTUAL_ADDRESS*)(&material_types[items_count]);
                    element_buffers = (D3D12_GPU_VIRTUAL_ADDRESS*)(&position_buffers[items_count]);
                    index_buffer_views = (D3D12_INDEX_BUFFER_VIEW*)(&element_buffers[items_count]);
                    primitive_topologies = (D3D_PRIMITIVE_TOPOLOGY*)(&index_buffer_views[items_count]);
                    elements_types = (u32*)(&primitive_topologies[items_count]);
                    per_object_data = (D3D12_GPU_VIRTUAL_ADDRESS*)(&elements_types[items_count]);
                }
            }

        private:
            constexpr static u32 struct_size{
                sizeof(id::id_type) +                   // entity_ids
                sizeof(id::id_type) +                   // submesh_gpu_ids
                sizeof(id::id_type) +                   // material_ids
                sizeof(ID3D12PipelineState *) +         // gpass_pipeline_states
                sizeof(ID3D12PipelineState *) +         // depth_pipeline_states
                sizeof(ID3D12RootSignature*) +          // root_signatures
                sizeof(material_type::type) +           // material_types
                sizeof(D3D12_GPU_VIRTUAL_ADDRESS) +     // position_buffers
                sizeof(D3D12_GPU_VIRTUAL_ADDRESS) +     // element_buffers
                sizeof(D3D12_INDEX_BUFFER_VIEW) +       // index_buffer_views
                sizeof(D3D_PRIMITIVE_TOPOLOGY) +        // primitive_topologies
                sizeof(u32) +                           // elements_types
                sizeof(D3D12_GPU_VIRTUAL_ADDRESS)       // per_object_data
            };

            utl::vector<u8> _buffer;
        } frame_cache;
        
        bool create_buffers(math::u32v2 size)
        {
            assert(size.x > 0 && size.y > 0);
            gpass_main_buffer.release();
            gpass_depth_buffer.release();

            D3D12_RESOURCE_DESC desc{};
            desc.Alignment = 0;
            desc.DepthOrArraySize = 1;
            desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            desc.Format = main_buffer_format;
            desc.Width = size.x;
            desc.Height = size.y;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.MipLevels = 0;
            desc.SampleDesc = {1, 0};

            //create the main buffer
            {
                d3d12_texture_init_info info{};
                info.desc = &desc;
                info.initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;//像素着色器通过着色器资源视图读取资源之前，资源必须处于此状态。这是只读状态
                info.clear_value.Format = desc.Format;
                memcpy(&info.clear_value.Color, &clear_value[0], sizeof(clear_value));
                gpass_main_buffer = d3d12_render_texture(info);
            }
            
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            desc.Format = depth_buffer_format;
            desc.MipLevels = 1;
            
            //crete depth buffer
            {
                d3d12_texture_init_info info{};
                info.desc = &desc;
                info.initial_state = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                info.clear_value.Format = desc.Format;
                info.clear_value.DepthStencil.Depth = 0.0f;
                info.clear_value.DepthStencil.Stencil = 0;

                gpass_depth_buffer = d3d12_depth_buffer(info);
                NAME_D3D12_OBJECT(gpass_main_buffer.resource(), L"GPass Main Buffer");
                NAME_D3D12_OBJECT(gpass_depth_buffer.resource(), L"GPass Depth Buffer");
            }
            
            
            return gpass_main_buffer.resource() && gpass_depth_buffer.resource();
        }

        bool create_gpass_pso_and_root_signature()
        {
            assert(!gpass_root_sig && !gpass_pso);

            // Create GPass root signature
            using idx = gpass_root_param_indices;
            d3dx::d3d12_root_parameter parameters[idx::count]{};
            parameters[0].as_constants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);
            d3dx::d3d12_root_signature_desc root_signature{ &parameters[0], idx::count };
            root_signature.Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            gpass_root_sig = root_signature.create();
            assert(gpass_root_sig);
            NAME_D3D12_OBJECT(gpass_root_sig, L"GPass Root Signature");

            // Create GPass PSO
            struct {
                d3dx::d3d12_pipeline_state_subobject_root_signature         root_signature{ gpass_root_sig };
                d3dx::d3d12_pipeline_state_subobject_vs                     vs{ shaders::get_engine_shader(shaders::engine_shader::fullscreen_triangle_vs) };
                d3dx::d3d12_pipeline_state_subobject_ps                     ps{ shaders::get_engine_shader(shaders::engine_shader::fill_color_ps) };
                d3dx::d3d12_pipeline_state_subobject_primitive_topology     primitive_topology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
                d3dx::d3d12_pipeline_state_subobject_render_target_formats  render_target_formats;
                d3dx::d3d12_pipeline_state_subobject_depth_stencil_format   depth_stencil_format{ depth_buffer_format };
                d3dx::d3d12_pipeline_state_subobject_rasterizer             rasterizer{ d3dx::rasterizer_state.no_cull };
                d3dx::d3d12_pipeline_state_subobject_depth_stencil1         depth{ d3dx::depth_state.disabled };
            } stream;

            D3D12_RT_FORMAT_ARRAY rtf_array{};
            rtf_array.NumRenderTargets = 1;
            rtf_array.RTFormats[0] = main_buffer_format;

            stream.render_target_formats = rtf_array;

            gpass_pso = d3dx::create_pipeline_state(&stream, sizeof(stream));
            NAME_D3D12_OBJECT(gpass_pso, L"GPass Pipeline State Object");

            return gpass_root_sig && gpass_pso;
        }

        
        void
        prepare_render_frame(const d3d12_frame_info& d3d12_info)
        {
            assert(d3d12_info.info && d3d12_info.camera);
            assert(d3d12_info.info->render_item_ids && d3d12_info.info->render_item_count);
            gpass_cache& cache{ frame_cache };
            cache.clear();

            using namespace content;
            render_item::get_d3d12_render_item_ids(*d3d12_info.info, cache.d3d12_render_item_ids);
            cache.resize();
            const u32 items_count{ cache.size() };
            const render_item::items_cache items_cache{ cache.items_cache() };
            render_item::get_items(cache.d3d12_render_item_ids.data(), items_count, items_cache);

            const submesh::views_cache views_cache{ cache.views_cache() };
            submesh::get_views(items_cache.submesh_gpu_ids, items_count, views_cache);

            const material::materials_cache materials_cache{ cache.materials_cache() };
            material::get_materials(items_cache.material_ids, items_count, materials_cache);
        }
    }


    
    bool initialize()
    {
        return create_buffers(initial_dimensions) && create_gpass_pso_and_root_signature();
    }

    void shutdown()
    {
        gpass_main_buffer.release();
        gpass_depth_buffer.release();
        dimensions = initial_dimensions;

        core::release(gpass_root_sig);
        core::release(gpass_pso);
    }

    void set_size(math::u32v2 size)
    {
        math::u32v2& d = dimensions;
        if(size.x > d.x || size.y >d.y)
        {
            d = {std::max(size.x, d.x), std::max(size.y, d.y)};
            create_buffers(d);
        }
    }

    void depth_prepass(ID3D12GraphicsCommandList6* cmd_list, const d3d12_frame_info& d3d12_info)
    {
        prepare_render_frame(d3d12_info);
    }
    void render(ID3D12GraphicsCommandList6* cmd_list, const d3d12_frame_info& info)
    {
        
        cmd_list->SetGraphicsRootSignature(gpass_root_sig);
        cmd_list->SetPipelineState(gpass_pso);

        static u32 frame{ 0 };
        ++frame;
        struct
        {
            f32 width;
            f32 height;
            u32 frame;
        }constants {(f32)info.surface_width, (f32)info.surface_height, frame};

        using idx = gpass_root_param_indices;
        //把当前是第几帧这个数据加入到我们刚刚创建的gpass_root_sig的第一个参数
        cmd_list->SetGraphicsRoot32BitConstants(idx::root_constants, 3, &constants, 0);

        //Interpret the vertex data as a list of triangles.
        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd_list->DrawInstanced(3, 1, 0, 0);
    }

    //我们这里要写入深度了，要加上状态
    void add_transitions_for_depth_prepass(d3dx::d3d12_resource_barrier& barriers)
    {
        //use split barriers
        barriers.add(gpass_main_buffer.resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
        
        barriers.add(gpass_depth_buffer.resource(),
            D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE);
        
    }
    //深度buffer状态恢复回去，mainbuffer从只读，转为写。
    void add_transitions_for_gpass(d3dx::d3d12_resource_barrier& barriers)
    {
        barriers.add(gpass_main_buffer.resource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);
        barriers.add(gpass_depth_buffer.resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    void add_transitions_for_post_process(d3dx::d3d12_resource_barrier& barriers)
    {
        barriers.add(gpass_main_buffer.resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    //渲染深度
    void set_render_targets_for_depth_prepass(ID3D12GraphicsCommandList6* cmd_list)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = gpass_depth_buffer.dsv();
        cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
        cmd_list->OMSetRenderTargets(0, nullptr, 0, &dsv);
    }
    void set_render_targets_for_gpass(ID3D12GraphicsCommandList6* cmd_list)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = gpass_main_buffer.rtv(0);
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv = gpass_depth_buffer.dsv();

        cmd_list->ClearRenderTargetView(rtv, clear_value, 0, nullptr);
        cmd_list->OMSetRenderTargets(1, &rtv, 0, &dsv);
    }

    const d3d12_render_texture& main_buffer()
    {
        return gpass_main_buffer;
    }

    const d3d12_depth_buffer& depth_buffer()
    {
        return gpass_depth_buffer;
    }
    
}
