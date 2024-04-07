#include "D3D12Core.h"

constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

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
        
        ID3D12Device8*  main_device = nullptr;
        IDXGIFactory7*  dxgi_factory = nullptr;
        d3d12Command   gfx_command;

        bool failed_init()
        {
            shutdown();
            return false;
        }
        
        
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
            DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
            debug_interface->EnableDebugLayer();
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

        new (&gfx_command) d3d12Command(main_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        if(!gfx_command.command_queue()) return failed_init();
        
        NAME_D3D12_OBJECT(main_device, L"MAIN D3D12 DEVICE");
#ifdef _DEBUG
        {
            ComPtr<ID3D12InfoQueue> info_queue;
            DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        }
#endif
        return true;
    }

    void shutdown()
    {
        gfx_command.release();
        release(dxgi_factory);
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

    void render()
    {
        //1.wait for GPU to finish with the command allocator and reset the allocator once the GPU is done with it.
        //This frees the memory that used to store commands.
        gfx_command.begin_frame();
        

        //2.Record commands to command list
        ID3D12GraphicsCommandList6* cmd_list = gfx_command.command_list();

        //3.Done recording commands. ExecuteCommands now.
        //signal and increment the fence for next frame.
        gfx_command.end_frame();
    }


}
