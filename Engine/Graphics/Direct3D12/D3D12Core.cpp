#include "D3D12Core.h"

#include "D3D12Resources.h"
#include "D3D12Surface.h"


using namespace Microsoft::WRL;

namespace ChillEngine::graphics::d3d12::core
{
    namespace
    {
        
        class d3d12Command
        {
        public:
            d3d12Command() = default;
            DISABLE_COPY_AND_MOVE(d3d12Command);
            explicit d3d12Command(ID3D12Device8 *const device, D3D12_COMMAND_LIST_TYPE type)
            {
                //Command Queue Create
                HRESULT hr = S_OK;
                D3D12_COMMAND_QUEUE_DESC desc{};
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                desc.NodeMask = 0;
                desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
                desc.Type = type;// 使用的command list 类型
                DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmd_queue)));
                if(FAILED(hr))
                {
                    goto _error;
                }
                NAME_D3D12_OBJECT(_cmd_queue, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"gfx command queue" :
                    type==D3D12_COMMAND_LIST_TYPE_COMPUTE? L"Compute command queue": L"Command Queue");

                //Command List Create
                //1.create command allocator
                for(u32 i = 0; i < frame_buffer_count; i++)
                {
                    command_frame& frame = _cmd_frames[i];
                    DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.cmd_allocator)));
                    if (FAILED(hr)) goto _error;
                    NAME_D3D12_OBJECT_Indexed(frame.cmd_allocator, i, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"gfx command allocator" :
                            type==D3D12_COMMAND_LIST_TYPE_COMPUTE? L"Compute command allocator": L"Command allocator");
                }
                //2.create command list
                DXCall(hr = device->CreateCommandList(0, type, _cmd_frames[0].cmd_allocator, nullptr, IID_PPV_ARGS(&_cmd_list)));
                if(FAILED(hr)) goto _error;
                //cmdList reset 前要 close
                DXCall(_cmd_list->Close());
                NAME_D3D12_OBJECT(_cmd_list, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"gfx command list" :
                    type==D3D12_COMMAND_LIST_TYPE_COMPUTE? L"Compute command list": L"Command list");
                
                DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
                if(FAILED(hr)) goto _error;
                NAME_D3D12_OBJECT(_fence, L"D3D12 Fence");

                _fence_event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
                assert(_fence_event);
                return;
                
                _error:
                    release();
            }

            ~d3d12Command()
            {
                assert(!_cmd_queue && !_cmd_list && !_fence);
            }
            
            
            //wait for the current frame to be signalled and reset the command list/allocator
            void begin_frame()
            {
                command_frame& frame = _cmd_frames[_frame_index];
                //wait gpu to finish this frame.
                frame.wait(_fence_event, _fence);
                
                //prepare for restart the frame
                DXCall(frame.cmd_allocator->Reset());
                DXCall(_cmd_list->Reset(frame.cmd_allocator, nullptr));
            }
            
            //signal the fence with the new fence value
            void end_frame()
            {
                //在execute command list前一定要 close commandList
                DXCall(_cmd_list->Close());
                ID3D12CommandList* const cmd_lists[] = {_cmd_list};
                _cmd_queue->ExecuteCommandLists(_countof(cmd_lists), &cmd_lists[0]);

                u64& fence_value = _fence_value;
                ++fence_value;
                _cmd_queue->Signal(_fence, _fence_value);
                command_frame& frame = _cmd_frames[_frame_index];
                frame.fence_value = fence_value;

                _frame_index = (_frame_index + 1) % frame_buffer_count;
            }

            //wait all frames finished
            void flush()
            {
                for(u32 i = 0; i < frame_buffer_count; ++i)
                {
                    _cmd_frames[i].wait(_fence_event, _fence);
                }
                _frame_index = 0;
            }
            void release()
            {
                //wait all frames finished
                flush();
                //release resource of this object
                core::release(_fence);
                _fence_value = 0;

                CloseHandle(_fence_event);
                _fence_event = nullptr;
                
                core::release(_cmd_queue);
                core::release(_cmd_list);

                for(u32 i = 0; i < frame_buffer_count; ++i)
                {
                    _cmd_frames[i].release();
                }
            }

            constexpr ID3D12CommandQueue* command_queue() const {return _cmd_queue;}
            constexpr ID3D12GraphicsCommandList6* command_list() const {return _cmd_list;}
            constexpr u32 frame_index() const { return _frame_index;}
            
        private:
            struct command_frame
            {
                ID3D12CommandAllocator* cmd_allocator = nullptr;
                u64                     fence_value = 0;
                //wait gpu to finish this frame: when fence_event is notified, which means gpu has finished this frame.
                void wait(HANDLE fence_event, ID3D12Fence1* fence)
                {
                    assert(fence && fence_event);
                    //If the current fence value is still less than "fence_value", then we know GPU has not finished executing the command lists
                    //Since it has not reached the  "_cmd_queue->Signal" command.
                    if(fence->GetCompletedValue() < fence_value)
                    {
                        DXCall(fence->SetEventOnCompletion(fence_value, fence_event));
                        WaitForSingleObject(fence_event, INFINITE);
                    }
                }
                void release()
                {
                    core::release(cmd_allocator);
                    fence_value = 0;
                }
            };
            ID3D12CommandQueue*         _cmd_queue = nullptr;
            ID3D12GraphicsCommandList6* _cmd_list = nullptr;
            ID3D12Fence1*               _fence = nullptr;
            u64                         _fence_value = 0;
            command_frame               _cmd_frames[frame_buffer_count] {};
            HANDLE                      _fence_event = nullptr;
            u32                         _frame_index = 0;
        };

        using surface_collection = utl::free_list<d3d12_surface>;
        ID3D12Device8*                  main_device = nullptr;
        IDXGIFactory7*                  dxgi_factory = nullptr;
        d3d12Command                    gfx_command;
        surface_collection              surfaces;
        descriptor_heap                 rtv_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//渲染目标视图资源 render target view
        descriptor_heap                 srv_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//着色器视图 shader resource view
        descriptor_heap                 uav_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);//无需访问视图 unordered access view
        descriptor_heap                 dsv_desc_heap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);//深度/模板视图资源  depth/stencil view
        
        utl::vector<IUnknown*>          deferred_releases[frame_buffer_count]{};
        u32                             deferred_release_flag[frame_buffer_count]{};
        std::mutex                      deferred_releases_mutex{};

        constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;
        constexpr DXGI_FORMAT render_target_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        bool failed_init()
        {
            shutdown();
            return false;
        }

        // Get the first most performing adapter that supports the minimum feature level.
        // NOTE: this function can be expanded in functionality with, for example, checking if any
        //       output devices (i.e. screens) are attached, enumerate the supported resolutions, provide
        //       a means for the user to choose which adapter to use in a multi-adapter setting, etc.
        IDXGIAdapter4* determine_main_adapter()
        {
            IDXGIAdapter4* adapter = nullptr;
            //获取性能最好的adapter
            for(u32 i = 0;
                dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
                ++i)
            {
                //pick first adapter that supports the minimum feature level.
                if(SUCCEEDED(D3D12CreateDevice(adapter, minimum_feature_level, __uuidof(ID3D12Device), nullptr)))
                {
                    return adapter;
                }
                release(adapter);
            
            }
            return nullptr;
        }

        D3D_FEATURE_LEVEL get_max_feature_level(IDXGIAdapter4* adapter)
        {
            constexpr D3D_FEATURE_LEVEL feature_levels[4]{
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_12_0,
                D3D_FEATURE_LEVEL_12_1,
            };

            D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_info{};
            feature_level_info.NumFeatureLevels = _countof(feature_levels);
            feature_level_info.pFeatureLevelsRequested = feature_levels;

            ComPtr<ID3D12Device> device;
            DXCall(D3D12CreateDevice(adapter, minimum_feature_level, IID_PPV_ARGS(&device)));
            DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level_info, sizeof(feature_level_info)));
            return feature_level_info.MaxSupportedFeatureLevel;
        }

        void __declspec(noinline) process_deferred_releases(u32 frame_idx)
        {
            std::lock_guard lock(deferred_releases_mutex);
            //NOTE: we clear this flag in the beginning.If we  clear it at the end , it will cause something wrong:
            //set_deferred_releases_flag() also set deferred_release_flag = 1, and it don't lock_guard because in x86 , access integer is atomic.
            //That will 
            deferred_release_flag[frame_idx] = 0;
            //release pending resources
            rtv_desc_heap.process_deferred_free(frame_idx);
            srv_desc_heap.process_deferred_free(frame_idx);
            uav_desc_heap.process_deferred_free(frame_idx);
            dsv_desc_heap.process_deferred_free(frame_idx);

            utl::vector<IUnknown*>& resources = deferred_releases[frame_idx];
            if(!resources.empty())
            {
                for(auto& resource: resources)
                {
                    release(resource);
                }
                resources.clear();
            }
        }
        
    }

    namespace detail
    {
        void deferred_release(IUnknown* resource)
        {
            const u32 frame_idx = current_frame_index();
            std::lock_guard lock(deferred_releases_mutex);
            deferred_releases[frame_idx].push_back(resource);
            set_deferred_releases_flag();
        }
    }

    bool initialize()
    {
        //determine which adapter (graphics card) to use.
        //determine what is the maximum feature level that is supporter
        //create a ID3D12Devive which is a virtual adapter

        if(main_device) shutdown();

        u32 dxgi_factory_flags = 0;
#ifdef _DEBUG
        // Enable debugging layer. Requires "Graphics Tools" optional feature
        {
            ComPtr<ID3D12Debug3> debug_interface;
            if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
            {
                debug_interface->EnableDebugLayer();
            }
            else
            {
                OutputDebugStringA("Warning: D3D12 Debug interface is not available.Requires 'Graphics Tools' optional feature.");
            }
            dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif
        HRESULT hr = S_OK;
        DXCall(hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory)));
        if(FAILED(hr)) return failed_init();

        //determine which adapter (graphics card) to use.
        ComPtr<IDXGIAdapter4> main_adapter;
        main_adapter.Attach(determine_main_adapter());
        if(!main_adapter) return failed_init();

        D3D_FEATURE_LEVEL max_feature_level = get_max_feature_level(main_adapter.Get());
        assert(max_feature_level >= minimum_feature_level);
        if(max_feature_level < minimum_feature_level) return failed_init();
        DXCall(hr = D3D12CreateDevice(main_adapter.Get(), max_feature_level, IID_PPV_ARGS(&main_device)));
        if(FAILED(hr)) return failed_init();

#ifdef _DEBUG
        {
            ComPtr<ID3D12InfoQueue> info_queue;
            DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        }
#endif
        
        bool result = true;
        result &= rtv_desc_heap.initialize(512, false);
        result &= dsv_desc_heap.initialize(512, false);
        result &= srv_desc_heap.initialize(4096, true);
        result &= uav_desc_heap.initialize(512, false);
        if(!result) return failed_init();
        
        new (&gfx_command) d3d12Command(main_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        if(!gfx_command.command_queue()) return failed_init();
        
        NAME_D3D12_OBJECT(main_device, L"MAIN D3D12 DEVICE");
        NAME_D3D12_OBJECT(rtv_desc_heap.heap(), L"RTV Descriptor Heap");
        NAME_D3D12_OBJECT(srv_desc_heap.heap(), L"SRV Descriptor Heap");
        NAME_D3D12_OBJECT(uav_desc_heap.heap(), L"UAV Descriptor Heap");
        NAME_D3D12_OBJECT(dsv_desc_heap.heap(), L"DSV Descriptor Heap");

        return true;
    }

    void shutdown()
    {
        gfx_command.release();
        //note: we don't call process_deferred_releases at the end because some resource(such as swap chains) can't be released before
        //their depending resources are released.
        for(u32 i = 0; i < frame_buffer_count; ++i)
        {
            process_deferred_releases(i);
        }
        release(dxgi_factory);
        
        rtv_desc_heap.release();
        dsv_desc_heap.release();
        srv_desc_heap.release();
        uav_desc_heap.release();

        //note:some types only use deferred release for their resources during
        //      shutdown/reset/clear. To finally release these resources we call process_deferred_releases once more.
        process_deferred_releases(0);

#ifdef _DEBUG
        {
            {
                ComPtr<ID3D12InfoQueue> info_queue;
                DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
            }

            ComPtr<ID3D12DebugDevice2> debug_device;
            DXCall(main_device->QueryInterface(IID_PPV_ARGS(&debug_device)));
            release(main_device);
            DXCall(debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY|D3D12_RLDO_DETAIL|D3D12_RLDO_IGNORE_INTERNAL));
            
        }
#endif
        release(main_device);
    }

    // void render()
    // {
    //     //1.wait for GPU to finish with the command allocator and reset the allocator once the GPU is done with it.
    //     //This frees the memory that used to store commands.
    //     gfx_command.begin_frame();
    //
    //     const u32 frame_idx = current_frame_index();
    //     if(deferred_release_flag[frame_idx])
    //     {
    //         process_deferred_releases(frame_idx);
    //     }
    //
    //     //2.Record commands to command list
    //     ID3D12GraphicsCommandList6* cmd_list = gfx_command.command_list();
    //
    //     //3.Done recording commands. ExecuteCommands now.
    //     //signal and increment the fence for next frame.
    //     gfx_command.end_frame();
    // }

    ID3D12Device *const device()
    {
        return main_device;
    }

    void set_deferred_releases_flag()
    {
        deferred_release_flag[current_frame_index()] = 1;
    }

    u32 current_frame_index()
    {
        return gfx_command.frame_index();
    }

    surface create_surface(platform::window window)
    {
        //be careful of this: window will be used construct a temporary d3d12_surface and then move it to back of the vector.
        //and because it is a non-trivial class , the default move construct is wrong
        u32 index = surfaces.add(window);
        surface_id id((u32)index);
        surfaces[id].create_swap_chain(dxgi_factory, gfx_command.command_queue(), render_target_format);
        return surface{id};
    }

    void remove_surface(surface_id id)
    {
        gfx_command.flush();
        
        surfaces.remove(id);
    }

    void resize_surface(surface_id id, u32, u32)
    {
        gfx_command.flush();
        surfaces[id].resize();
    }

    u32 surface_width(surface_id id)
    {
        return surfaces[id].width();
    }

    u32 surface_height(surface_id id)
    {
        return surfaces[id].height();
    }

    void render_surface(surface_id id)
    {
        //1.wait for GPU to finish with the command allocator and reset the allocator once the GPU is done with it.
        //This frees the memory that used to store commands.
        gfx_command.begin_frame();
        ID3D12GraphicsCommandList6* cmd_list = gfx_command.command_list();
        const u32 frame_idx = current_frame_index();
        if(deferred_release_flag[frame_idx])
        {
            process_deferred_releases(frame_idx);
        }

        const d3d12_surface& surface = surfaces[id];

        
        //presenting swap chain buffers happens in lockstep with frame buffers.
        surface.present();

        //2.record commands
        
        //3.Done recording commands. ExecuteCommands now.
        //signal and increment the fence for next frame.
        gfx_command.end_frame();
    }

    descriptor_heap& rtv_heap()
    {
        return rtv_desc_heap;
    }
    descriptor_heap& srv_heap()
    {
        return srv_desc_heap;
    }
    descriptor_heap& uav_heap()
    {
        return uav_desc_heap;
    }
    descriptor_heap& dsv_heap()
    {
        return dsv_desc_heap;
    }
    DXGI_FORMAT get_default_render_target_format()
    {
        return render_target_format;
    }
}
