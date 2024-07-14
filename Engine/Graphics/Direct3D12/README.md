



[TOC]

- [DX12-Wrapper Doc](#dx12-wrapper-doc)
  * [自定义数据结构部分](#---------)
  * [`utl::free_list`](#-utl--free-list-)
  * [`utl::vector`](#-utl--vector-)
  * [`ChillEngine::graphics::d3d12::core::d3d12Command`](#-chillengine--graphics--d3d12--core--d3d12command-)
  * [`ChillEngine::graphics::d3d12::descriptor_heap`](#-chillengine--graphics--d3d12--descriptor-heap-)
    + [内容概要](#----)
    + [封装](#--)
  * [资源Texture封装`ChillEngine::graphics::d3d12::d3d12_texture`](#--texture---chillengine--graphics--d3d12--d3d12-texture-)
  * [资源RenderTexture封装`ChillEngine::graphics::d3d12::d3d12_render_texture`](#--rendertexture---chillengine--graphics--d3d12--d3d12-render-texture-)
  * [资源DepthBuffer封装`ChillEngine::graphics::d3d12::d3d12_depth_buffer`](#--depthbuffer---chillengine--graphics--d3d12--d3d12-depth-buffer-)
  * [资源Buffer封装`ChillEngine::graphics::d3d12::d3d12_buffer`](#--buffer---chillengine--graphics--d3d12--d3d12-buffer-)
  * [资源ConstantBuffer封装`ChillEngine::graphics::d3d12::d3d12_texture`](#--constantbuffer---chillengine--graphics--d3d12--d3d12-texture-)
  * [上传数据工具类`ChillEngine::graphics::d3d12::upload::d3d12_upload_context`](#--------chillengine--graphics--d3d12--upload--d3d12-upload-context-)
  * [`ChillEngine::graphics::d3d12::d3d12_surface`](#-chillengine--graphics--d3d12--d3d12-surface-)
  * [相机的封装`ChillEngine::graphics::d3d12::camera::d3d12_camera`](#------chillengine--graphics--d3d12--camera--d3d12-camera-)
  * [ResourceBarrier的封装](#resourcebarrier---)
    + [`d3d12_resource_barrier`](#-d3d12-resource-barrier-)
    + [Split barriers](#split-barriers)
  * [RootSignature](#rootsignature)
    + [使用](#--)
    + [DynamicResource](#dynamicresource)
  * [PSO](#pso)
  * [Content资源管理模块`ChillEngine::graphics::d3d12::content`](#content-------chillengine--graphics--d3d12--content-)
    + [概念与数据组织](#-------)
    + [Material](#material)
      - [添加流程](#----)
      - [d3d12_material_stream](#d3d12-material-stream)
    + [submesh](#submesh)
      - [添加流程](#-----1)
      - [utl::blob_stream_reader](#utl--blob-stream-reader)
    + [render_item](#render-item)
      - [利用`geometry_hierarchy_stream`添加geometry](#---geometry-hierarchy-stream---geometry)
      - [Render Item](#render-item)
  * [Shader模块](#shader--)
  * [后处理模块](#-----)
  * [图形管线模块](#------)

<small><i><a href='http://ecotrust-canada.github.io/markdown-toc/'>Table of contents generated with markdown-toc</a></i></small>

# DX12-Wrapper Doc

引擎中DX12部分的API文档，设计的思想是不同图形API接口暴露在`GraphicsPlatformInterface.h`，目前只含有D3D12。

```c++
//Render.cpp
bool set_platform_interface(graphics_platform platform, platform_interface& pi)
{
    switch (platform)
    {
        case graphics_platform::direct3d12:
            d3d12::get_platform_interface(pi);//引擎接口替换为d3d12图形接口
            break;
        default:
            return false;
    }

    return true;
}
```

## 自定义数据结构部分

## `utl::free_list`

​	相当于一个内存池，用`utl::vector`做底层容器初始化时reserve开辟一片大内存

free_list添加元素时：

- 如果`utl::vector`底层容器容量够，则直接emplace_back。

- 如果next_free_index有赋值说明前面有人释放了资源，这块内存可以复用，此时直接复用内存，从next_free_index获取空闲空间，利用placement new加入。

free_list删除元素时：

- 会利用元素内存前32bit标识next_free_index为当前内存空闲处
- 随后_next_free_index转到当前元素内存处。



一个示例图如下，可以清楚解释我们是如何利用空闲区与的前四位做空闲链表的。

![image-20240710153216972](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240710153216972.png)

## `utl::vector`

​	对于`free_list`如果使用`std::vector`可能会有一些风险，原因在于每次从free_list做remove元素操作时会手动调用元素的析构函数以保证元素资源的释放，而对于`std::vector`在其本身析构后还会对每个元素进行析构，这也是把`std::vector`作为内存池子的弊端，因此我们写一个`utl::vector<ElementType,Destruct>`, 其中表示`Destruct`表示`utl::vector`析构后是否析构其包含的每个元素。元素中析构的具体行为交给上层`free_list`来完成。

## `ChillEngine::graphics::d3d12::core::d3d12Command`

这个类中封装了CommandQueue/CommandList/CommandFrame等信息，还通过Fence保证了GPU和CPU间的同步。



- 我们禁用了Copy & Move相关函数，防止内部资源泄露/转移，保证每一个d3d12Command都在操作者的可见管理下。

- 其包含数据成员：

  ```c++
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
  ID3D12CommandQueue*         _cmd_queue = nullptr;//CommandQueue
  ID3D12GraphicsCommandList6* _cmd_list = nullptr;//CommandList
  ID3D12Fence1*               _fence = nullptr;//Fence
  u64                         _fence_value = 0;//Fence Value
  command_frame               _cmd_frames[frame_buffer_count] {};//一个Circle Buffer
  HANDLE                      _fence_event = nullptr;//用于阻塞CPU逻辑，等待GPU完成
  u32                         _frame_index = 0;//当前帧在command_frame的下标
  ```

- 构造函数中需要做几件事：

  - 签名

    ```c++
    explicit d3d12Command(ID3D12Device8 *const device, D3D12_COMMAND_LIST_TYPE type)
    ```

  - 创建CommandQueue: 根据`D3D12_COMMAND_LIST_TYPE`创建CommandQueue。

  - 创建CommandList: 创建完需要Close，因为后续CommandList每帧会调用Reset，否则会在第一次Reset时报错：

    > spurious error "544 ID3D12GraphicsCommandList::Reset: Reset fails because the command list was not closed." 

  - 创建Fence:

    ```c++
    DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
    if(FAILED(hr)) goto _error;
    NAME_D3D12_OBJECT(_fence, L"D3D12 Fence");
    ```

  - 创建Windows的事件对象`fence_event` ：用于CPU等待GPU帧时通过`WaitForSingleObject(fence_event, INFINITE);`阻塞，在fence->GetCompletedValue() == fence_value时解除阻塞。从而实现利用Fence + fence_event实现CPU/GPU同步。

- 函数部分

  - `BeginFrame()` 等待GPU完成当前帧，reset CommandList和CommandAllocator
  - `EndFrame()`CommandQueue执行ExecuteCommandLists提交命令。fence_value自增。
  - `Flush()` 等待所有帧完成，具体逻辑是遍历环形缓冲区`_cmd_frames`对每个进行wait调用以确定此时所有帧GPU逻辑已完成
  - `release()`意味着释放该d3d12Command对象，具体时先Flush(), fence_value归0，关闭fence_event, 释放CommnadQueue和CommandList，对command_frame调用Release()。





## `ChillEngine::graphics::d3d12::descriptor_heap`

这部分我们主要封装了描述符和描述符堆相关逻辑。

### 内容概要

​	DX12 使用`ID3D12Resource` 作为资源的基类，个人认为这种设计思路原因有是GPU 资源本质上是一块块存储数据的缓冲区，这样可以更贴近底层，并其需要使用者自己去提供资源管理的方法。不管是一块buffer，还是一张texture。用ID3D12Resource来统一表示GPU资源，**GPU资源并不直接绑定到渲染流水线上，而是通过名为描述符（Descriptor）的中间对象来绑定。**

资源描述符有以下：

- Constant buffer view (CBV) 【用于一般数据提交】
- Unordered access view (UAV) 【用于Computer Shader可写入的纹理】
- Shader resource view (SRV) 【用于提交给shader的纹理】
- Samplers 【提交给shader的所使用的纹理采样器】

- Render Target View (RTV) 【渲染目标纹理】
- Depth Stencil View (DSV) 【渲染目标深度/模板共享纹理】
- Index Buffer View (IBV) 【Mesh中的索引数据】
- Vertex Buffer View (VBV) 【Mesh中的顶点数据】
- Stream Output View (SOV) 【Stream Output输出数据】



描述符堆分为：着色器可见描述符堆（Shader Visible Descriptor Heap）和 着色器不可见描述符堆（Shader Invisible Descriptor Heap）

**着色器可见描述符堆（Shader Visible Descriptor Heap）：**

1. **可见性**：这类描述符堆是 GPU 着色器可以直接访问的。这意味着在绘制或计算过程中，着色器可以通过根签名（Root Signature）直接引用这些描述符。
2. **性能**：由于 GPU 可以直接访问这些描述符堆，访问速度较快，但它们的数量和大小受到严格的限制。每种类型的着色器可见描述符堆（CBV_SRV_UAV、Sampler）只能各有一个，并且这些堆的大小通常会有较低的上限。
3. **用途**：适合用于在绘制过程中需要频繁访问或更改的资源，例如纹理、常量缓冲区和着色器资源视图（SRV）。

**着色器不可见描述符堆（Shader Invisible Descriptor Heap）：**

1. **可见性**：这类描述符堆是 GPU 着色器无法直接访问的。它们仅供 CPU 使用，通常用于存储需要在 CPU 和 GPU 之间传递的描述符。
2. **性能**：由于这些堆不可直接被 shader 访问，因此在 CPU 上创建和管理这些堆时没有那么多的限制，可以更灵活地调整大小和数量。
3. **用途**：适用于需要频繁更新但不需要在每帧都由 shader 直接访问的描述符，例如在初始化或加载资源阶段，或者需要在后台处理的资源。





| 描述符类型    | 着色器可见，CPU 只写 | 非着色器可见，CPU 读/写 |
| ------------- | -------------------- | ----------------------- |
| CBV, SRV, UAV | 是                   | 是                      |
| Sampler       | 是                   | 是                      |
| RTV           | 否                   | 是                      |
| DSV           | 否                   | 是                      |

**CBV（Constant Buffer View）、SRV（Shader Resource View）、UAV（Unordered Access View）**

- **着色器可见，CPU 只写**：这些描述符可以放在着色器可见的描述符堆中，供 GPU 着色器访问。由于 CPU 只需在初始化或更新资源时写入描述符，因此是“只写”。
- **非着色器可见，CPU 读/写**：这些描述符也可以放在不可见的描述符堆中，供 CPU 读/写。CPU 可以灵活地创建和更新这些描述符，而不必担心 GPU 的访问限制。

**Sampler**

- **着色器可见，CPU 只写**：采样器描述符也可以放在着色器可见的描述符堆中，供 GPU 着色器访问。
- **非着色器可见，CPU 读/写**：采样器描述符同样可以放在不可见的描述符堆中，供 CPU 读/写管理。

**RTV（Render Target View）**

- **非着色器可见，CPU 读/写**：RTV 描述符通常放在不可见的描述符堆中，供 CPU 管理。CPU 可以在渲染过程中灵活地设置和更新渲染目标，但这些描述符不需要在着色器中直接访问。
- **不在着色器可见描述符堆中**：因为 RTV 主要用于设置渲染目标，不需要在着色器中频繁访问，所以不需要放在着色器可见的描述符堆中。

**DSV（Depth Stencil View）**

- **非着色器可见，CPU 读/写**：DSV 描述符同样放在不可见的描述符堆中，供 CPU 管理。CPU 可以在渲染过程中设置和更新深度/模板缓冲区。
- **不在着色器可见描述符堆中**：因为 DSV 主要用于设置深度/模板缓冲区，不需要在着色器中直接访问，所以不需要放在着色器可见的描述符堆中。



描述符堆有以下类型：

- D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV：可以存放CBV, SRV和UAV
- D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER： 存放sampler
- D3D12_DESCRIPTOR_HEAP_TYPE_RTV：存放RTV
- D3D12_DESCRIPTOR_HEAP_TYPE_DSV：存放DSV

索引缓冲区视图 (IBV) 、顶点缓冲区视图 (VBV) 和流输出视图 (SOV) 采用直接传递到 API 方法，没有特定的堆类型。

### 封装

封装的目的是：

- descriptorHeap通过Initialize()申请heap空间
- `_free_handles`数据结构维护可用descriptor空间
- 提供allocate/free函数对descriptor进行管理

- descriptor下一帧延迟释放，防止有其他引用。



具体封装：

- 构造函数参数仅包含一个描述符堆类型
- `bool initialize(u32 capacity, bool is_shader_visible);`真正创建描述符堆
  - 对于`D3D12_DESCRIPTOR_HEAP_TYPE_DSV || D3D12_DESCRIPTOR_HEAP_TYPE_RTV` 无论入参`is_shader_visible`如何都重置其为false。
  - 设置cpu/gpu start, 描述符数量`_capacity`，对应描述符堆类型的一个描述符的大小`descriptor_size`等信息
- 描述符的获取/释放
  - `descriptor_handle descriptor_heap::allocate()`函数会在从`_free_handles`取出一个描述符，返回这个描述符的handle，`descriptor_handle`包含的信息有cpu/gpu地址，desriptor在描述符堆的对应下标等信息。
  - `void descriptor_heap::free(descriptor_handle& handle)` ：记录在`_deferred_free_indices`中，下一帧延迟释放，并加入`_free_handles`。
- `void descriptor_heap::process_deferred_free(u32 frame_idx)`处理延迟释放资源，遍历`_deferred_free_indices`把释放的index加入到`_free_handles`。

```c++
//D3D12Resources.h
class descriptor_heap
{
    public:
    DISABLE_COPY_AND_MOVE(descriptor_heap);
    explicit descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type): _type(type)
    {

    }
    ~descriptor_heap() {assert(!_heap);}

    bool initialize(u32 capacity, bool is_shader_visible);
    void process_deferred_free(u32 frame_idx);
    void release();


    descriptor_handle allocate();
    void free(descriptor_handle& handle);

    constexpr D3D12_DESCRIPTOR_HEAP_TYPE type() const {return _type;}
    constexpr D3D12_CPU_DESCRIPTOR_HANDLE cpu_start() const {return _cpu_start;}
    constexpr D3D12_GPU_DESCRIPTOR_HANDLE gpu_start() const {return _gpu_start;}
    constexpr ID3D12DescriptorHeap* const heap() const {return _heap;}
    constexpr u32 capacity() const {return _capacity;}
    constexpr u32 size() const {return _size;}
    constexpr u32 descriptor_size() const {return _descriptor_size;}
    constexpr bool is_shader_visible() const {return _gpu_start.ptr != 0;}


    private:
    ID3D12DescriptorHeap*               _heap = nullptr;;
    D3D12_CPU_DESCRIPTOR_HANDLE         _cpu_start{};
    D3D12_GPU_DESCRIPTOR_HANDLE         _gpu_start{};
    u32                                 _capacity = 0;
    u32                                 _size = 0;//how many descriptor has been allocated
    u32                                 _descriptor_size = 0;
    utl::vector<u32>                    _deferred_free_indices[frame_buffer_count]{};
    const D3D12_DESCRIPTOR_HEAP_TYPE    _type;
    std::mutex                          _mutex{};
    std::unique_ptr<u32[]>              _free_handles;
};

```







## 资源Texture封装`ChillEngine::graphics::d3d12::d3d12_texture`

- 禁用拷贝构造(我们不像多个d3d12_texture拥有着同一个descriptor_handle)，不禁用移动构造

- 成员对象只有两个：

  ```c++
  ID3D12Resource*     _resource = nullptr;
  descriptor_handle   _srv;
  ```

- 构造函数只有一个参数，封装为`d3d12_texture_init_info`

  ```c++
  struct d3d12_texture_init_info
  {
      ID3D12Heap1*                        heap = nullptr;
      ID3D12Resource*                     resource = nullptr;
      D3D12_SHADER_RESOURCE_VIEW_DESC*    srv_desc = nullptr;
      D3D12_RESOURCE_DESC*                desc = nullptr;
      D3D12_RESOURCE_ALLOCATION_INFO1     allocation_info{};
      D3D12_RESOURCE_STATES               initial_state{};
      D3D12_CLEAR_VALUE                   clear_value{};
  };
  ```

- 构造函数逻辑中做了一些处理：主要是根据不同情况进行资源创建

  - 如果是RenderTarget/DepthBuffer/Stencil资源设置`clear_value`，其他`clear_value`设置为nullptr

  - 根据形参`d3d12_texture_init_info`内的不同，我们做以下处理

    - 如果`info.resource`不为空，说明已有资源来创建texture，直接赋值给`_resource`

    - 如果`info.heap && info.desc`都不为空，说明已有堆，资源采用Placed Resource这种存储方式创建，调用`CreatePlacedResource`创建即可。

    - 如果仅有`info.desc`，那么说明我们需要采用CommittedResource这种存储方式创建资源，调用`CreateCommittedResource`, 该函数第一个参数为`D3D12_HEAP_PROPERTIES`，这里使用`D3D12_HEAP_TYPE_DEFAULT`, 为了方便使用，我们做了不同堆的封装在`ChillEngine::graphics::d3d12::d3dx::heap_properties`

      ```c++
      //D3D12Helpers.h
      //ChillEngine::graphics::d3d12::d3dx::heap_properties
      constexpr struct
      {
          const D3D12_HEAP_PROPERTIES default_heap
          {
              D3D12_HEAP_TYPE_DEFAULT,// D3D12_HEAP_TYPE Type;
              D3D12_CPU_PAGE_PROPERTY_UNKNOWN,// D3D12_CPU_PAGE_PROPERTY CPUPageProperty;
              D3D12_MEMORY_POOL_UNKNOWN, // D3D12_MEMORY_POOL MemoryPoolPreference;
              0,// UINT CreationNodeMask
              0// UINT VisibleNodeMask
          };
      
          const D3D12_HEAP_PROPERTIES upload_heap
          {
              D3D12_HEAP_TYPE_UPLOAD,// D3D12_HEAP_TYPE Type;
              D3D12_CPU_PAGE_PROPERTY_UNKNOWN,// D3D12_CPU_PAGE_PROPERTY CPUPageProperty;
              D3D12_MEMORY_POOL_UNKNOWN, // D3D12_MEMORY_POOL MemoryPoolPreference;
              0,// UINT CreationNodeMask
              0// UINT VisibleNodeMask
          };
      }heap_properties;
      ```

  - 最后`_srv = core::srv_heap().allocate();`获取一个srv类型描述符，将其与资源绑定`device->CreateShaderResourceView(_resource, info.srv_desc, _srv.cpu);`

- `void d3d12_texture::release()` 释放描述符，延迟释放资源

  ```c++
  void d3d12_texture::release()
  {
      core::srv_heap().free(_srv);
      core::deferred_release(_resource);
  }
  ```

  



## 资源RenderTexture封装`ChillEngine::graphics::d3d12::d3d12_render_texture`

- 同样禁用拷贝构造函数，支持移动构造

- 成员变量有3个

  ```c++
  d3d12_texture       _texture{};//纹理资源
  descriptor_handle   _rtv[d3d12_texture::max_mips]{};//我们设置texture不超过16k分辨率，最多14层miamap，所以rtv数组最多14个元素(d3d12_texture::max_mips==14)
  u32                 _mip_count{ 0 };//mipmap层级数
  ```

- 构造函数: 根据资源获得mipmap层数，分别创建rtv

  - 为每一层mipmap创建texrure

    ```c++
    //create texture for each mip
    for (u32 i{ 0 }; i < _mip_count; ++i)
    {
        _rtv[i] = rtv_heap.allocate();
        device->CreateRenderTargetView(resource(), &desc, _rtv[i].cpu);
        ++desc.Texture2D.MipSlice;
    }
    ```

- `void d3d12_render_texture::release()`：释放资源，该函数会在`d3d12_render_texture`析构函数中调用

  ```c++
  void d3d12_render_texture::release()
  {
      for (u32 i{ 0 }; i < _mip_count; ++i) core::rtv_heap().free(_rtv[i]);
      _texture.release();
      _mip_count = 0;
  }
  ```





## 资源DepthBuffer封装`ChillEngine::graphics::d3d12::d3d12_depth_buffer`

- 同样禁用拷贝构造函数，支持移动构造

- 成员变量有2个：一个是纹理资源，另一个是dsv描述符

  ```c++
  d3d12_texture       _texture{};
  descriptor_handle   _dsv{};
  ```

- 构造函数

  - 首先把texture资源的格式进行了调整，因为`DXGI_FORMAT_D32_FLOAT`不能用作SRV，为了能够将深度缓冲区既用于DSV又用于SRV，需要使用对应的无类型格式，并在创建视图时指定具体的格式，这里我们调整为`DXGI_FORMAT_R32_TYPELESS`

  ```c++
  d3d12_depth_buffer::d3d12_depth_buffer(d3d12_texture_init_info info)
  {
      assert(info.desc);
      const DXGI_FORMAT dsv_format{ info.desc->Format };
  
      D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
      if (info.desc->Format == DXGI_FORMAT_D32_FLOAT)
      {
          info.desc->Format = DXGI_FORMAT_R32_TYPELESS;
          srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
      }
  
      srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MipLevels = 1;
      srv_desc.Texture2D.MostDetailedMip = 0;
      srv_desc.Texture2D.PlaneSlice = 0;
      srv_desc.Texture2D.ResourceMinLODClamp = 0.f;
  
      assert(!info.srv_desc && !info.resource);
      info.srv_desc = &srv_desc;
      _texture = d3d12_texture(info);
  
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
      dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
      dsv_desc.Format = dsv_format;
      dsv_desc.Texture2D.MipSlice = 0;
  
      _dsv = core::dsv_heap().allocate();
  
      auto *const device{ core::device() };
      assert(device);
      device->CreateDepthStencilView(resource(), &dsv_desc, _dsv.cpu);
  }
  ```

  



## 资源Buffer封装`ChillEngine::graphics::d3d12::d3d12_buffer`



- 同样禁用拷贝函数，防止一个资源多个owner

- 申请的空间大小要做内存对齐`_size = (u32)math::align_size_up(info.size, info.alignment);`

- 申请Buffer



对于Buffer的申请，我们做了如下封装，函数参数：

- `data`: 指向要上传到缓冲区的初始数据。
- `buffer_size`: 缓冲区的大小（以字节为单位）。
- `is_cpu_accessible`: 指示缓冲区是否可由CPU访问。如果我们想创建完后向buffer填充数据则需为true(同时也意味着要创建在上传堆)，如果只进行一次上传数据随后被GPU读写，则无需设置为true，函数中会立刻利用上传堆进行上传。
- `state`: 缓冲区的初始资源状态。
- `flags`: 资源标志（例如允许无序访问）。
- `heap`: 可选的自定义堆。
- `heap_offset`: 在自定义堆中的偏移量。

```c++
ID3D12Resource* create_buffer(const void* data, u32 buffer_size, bool is_cpu_accessible /*= false*/,
                              D3D12_RESOURCE_STATES state /*= D3D12_RESOURCE_STATE_COMMON*/,
                              D3D12_RESOURCE_FLAGS flags /*= D3D12_RESOURCE_FLAG_NONE*/,
                              ID3D12Heap* heap /*= nullptr*/, u64 heap_offset /*= 0*/)
```

- 考虑`is_cpu_accessible`，如果为True意味着如果是CPU可访问说明需要一个上传堆，同时需要把`D3D12_RESOURCE_STATES`设置为`D3D12_RESOURCE_STATE_GENERIC_READ`，这样GPU就可以读了。

  <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240709170710489.png" alt="image-20240709170710489" style="zoom:50%;" />

- 首先是`D3D12_RESOURCE_DESC desc{};`的相关设置：

  ```
  assert(buffer_size);
  //create buffer resource
  D3D12_RESOURCE_DESC desc{};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Alignment = 0;
  desc.Width = buffer_size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc = {1, 0};
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  desc.Flags = is_cpu_accessible? D3D12_RESOURCE_FLAG_NONE : flags;
  
  assert(desc.Flags == D3D12_RESOURCE_FLAG_NONE || desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
  ID3D12Resource* resource = nullptr;
  const D3D12_RESOURCE_STATES resource_state
  {
      is_cpu_accessible ? D3D12_RESOURCE_STATE_GENERIC_READ : state
  };
  ```

- 创建堆：还是分为两种资源绑定到堆的方式，其中CommittedResource还需特殊处理`is_cpu_accessible`应使用上传堆。

  ```c++
  if(heap)
  {
      DXCall(core::device()->CreatePlacedResource(heap, heap_offset, &desc, resource_state,
                                                  nullptr, IID_PPV_ARGS(&resource)));
  }
  else
  {
      DXCall(core::device()->CreateCommittedResource(
          is_cpu_accessible? &heap_properties.upload_heap : &heap_properties.default_heap, D3D12_HEAP_FLAG_NONE, &desc, resource_state,
          nullptr, IID_PPV_ARGS(&resource)));
  }
  ```

- 创建一个空的 `D3D12_RANGE` 结构，表示写操作（CPU 不需要读取任何数据）。将 GPU 资源映射到 CPU 可访问的内存空间。检查映射是否成功。将数据从 `data` 缓冲区复制到映射的 GPU 资源内存中。取消映射，使 GPU 资源重新可供 GPU 访问。由于没有调用CopyResource(), 可以后续继续添加数据。

  ```c++
  if (is_cpu_accessible)
  {
      // NOTE: range's Begin and End fields are set to 0, to indicate that
      //       the CPU is not reading any data (i.e. write-only)
      const D3D12_RANGE range{};
      void* cpu_address{ nullptr };
      DXCall(resource->Map(0, &range, reinterpret_cast<void**>(&cpu_address)));//资源map映射到CPU系统内存上
      assert(cpu_address);
      memcpy(cpu_address, data, buffer_size);
      resource->Unmap(0, nullptr);
  }
  ```

- 如果是非cpu可访问，则需要一次把数据拷入上传堆，再向CommandList加入“把上传堆数据上传至GPU" 的指令。

  ```c++
  else
  {
      upload::d3d12_upload_context context{ buffer_size };
      memcpy(context.cpu_address(), data, buffer_size);//把数据复制到上传堆
      context.command_list()->CopyResource(resource, context.upload_buffer());//移到gpu里
      context.end_upload();
  }
  ```







最后d3d12_buffer构造函数为:

```c++
////////////////////////////////d3d12 buffer/////////////////////////////
d3d12_buffer::d3d12_buffer(d3d12_buffer_init_info info, bool is_cpu_accessible)
{
    assert(!_buffer && info.size && info.alignment);
    _size = (u32)math::align_size_up(info.size, info.alignment);
    _buffer = d3dx::create_buffer(info.data, _size, is_cpu_accessible, info.initial_state, info.flags, info.heap, info.allocation_info.Offset);
    _gpu_address = _buffer->GetGPUVirtualAddress();
    NAME_D3D12_OBJECT_Indexed(_buffer, _size, L"d3d12 buffer - size");
}
```





## 资源ConstantBuffer封装`ChillEngine::graphics::d3d12::d3d12_texture`

类中包含四个成员变量

```c++
d3d12_buffer    _buffer{};
u8*             _cpu_address{};
u32             _cpu_offset{};//how much does the buffer has used.
std::mutex      _mutex{};
```

一个是buffer，一个是开始地址，一个是当前buffer填充到的offset，mutex用于多线程操作这个类是多线程安全。



构造函数：

```c++
    constant_buffer::constant_buffer(d3d12_buffer_init_info info)
        :_buffer(info, true)//创建一个cpu_accessible的buffer
    {
        NAME_D3D12_OBJECT_Indexed(buffer(), size(), L"Constant Buffer - size");
        D3D12_RANGE range{};
        buffer()->Map(0, &range, (void**)(&_cpu_address));//映射到系统内存，准备后续进行修改
        assert(_cpu_address);
    }

```

进行分配时，首先需要内存对齐，`#define D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT ( 256 )`即25对齐，然后更新offset信息，返回地址。

```c++
u8* const constant_buffer::allocate(u32 size)
{
    std::lock_guard lock{_mutex};//多线程安全
    const u32 alignment_size =  (u32)d3dx::align_size_for_constant_buffer(size);
    assert(_cpu_offset + alignment_size <= _buffer.size());
    if(_cpu_offset + alignment_size <= _buffer.size())
    {
        u8 *const address = _cpu_address + _cpu_offset;
        _cpu_offset += alignment_size;
        return address;
    }
    return nullptr;
}
```



## 上传数据工具类`ChillEngine::graphics::d3d12::upload::d3d12_upload_context`

​	在上述的Buffer中，我们用到了该类，该类本质是创建一个cpu_accessible的Buffer资源，映射到系统内存，我们在CPU上即可上传数据，在结束时调用CoptResource() 和`end_upload()`即可。

​	具体可看`ChillEngine::graphics::d3d12::upload::d3d12_upload_context`

​	我们在`D3D12Upload.h`中定义了一个多个`upload_frame`：

```c++
struct upload_frame
{
    ID3D12CommandAllocator*             cmd_allocator = nullptr;
    ID3D12GraphicsCommandList6*         cmd_list = nullptr;
    ID3D12Resource*                     upload_buffer = nullptr;
    void*                               cpu_address = nullptr;
    u64                                 fence_value = 0;

    void wait_and_reset();

    void release()
    {
        wait_and_reset();
        core::release(cmd_allocator);
        core::release(cmd_list);
    }
    constexpr bool is_ready() const {return upload_buffer == nullptr;}
};
```

​	这样相当于我们的渲染框架拥有多个可以上传资源到GPU的upload_frame，以此多线程进行上传资源，cmd_list/allocator这些东西我们会提前创建，以后复用即可，同时fence_value保证了command。

​	在使用d3d12_upload_context时，就可以从upload_frame中获得上传资源的需要的CommandList，而无需每次重新创建。





## `ChillEngine::graphics::d3d12::d3d12_surface`

封装一个显示图形渲染结果的区域，此类主要用于管理 Direct3D 12 应用程序的渲染表面，并提供了一些必要的功能和接口来创建、管理和显示渲染内容。

**成员变量**：

- `_swap_chain`: 交换链对象。

- `_render_target_data`: 保存渲染目标数据的数组，包括后备缓冲区资源和对应的rtv描述符。

  - render_target_data结构如下

    ```c++
    struct render_target_data
    {
        ID3D12Resource* resource = nullptr;
        descriptor_handle rtv{};
    };
    ```

- `_window`: 窗口对象，用于创建渲染表面，这里我们自己封装了一个`platform::window`。

- `_format`: 后备缓冲区的格式，通常为`DXGI_FORMAT_R8G8B8A8_UNORM_SRGB`。

- `_current_backbuffer_index`: 当前后备缓冲区的索引。

- `_viewport`: 视口对象，控制渲染目标的显示区域。

- `_scissor_rect`: 裁剪矩形，控制渲染目标中的处理区域范围。

- `_allow_tearing`: 是否允许撕裂（Tearing）的标志。**Allow Tearing（允许撕裂）标志**：在某些情况下，开发者可能希望允许撕裂以换取更高的性能和响应速度。通过在创建交换链时设置允许撕裂标志，可以在保持高性能的同时允许一定程度的撕裂现象出现。

- `_present_flag`: 呈现标志，用于配置呈现功能特性。

**功能**：

- `create_swap_chain`: 创建交换链（Swap Chain），用于在窗口中显示图形输出。

  - 设置交换链信息，创建交换链，申请向RTVHeap申请RTV描述符
  - 调用Finalize()

- `finalize()`

  - 为BackBuffer创建rtv

    ```c++
     core::device()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
    ```

  - 获取swapchain中的backbuffer指针，添加至`_render_target_data`。

  - 设置视口`_viewport`和裁剪区域`_scissor_rect`

- `present`: 将当前后备缓冲区（Back Buffer）的内容呈现到前台显示。

- `resize`: 调整渲染表面的尺寸, **允许动态改变渲染窗口大小**。

  - 释放`_render_target_data`的backbuffer资源，重新ResizeBuffer。
  - 调用Finalize()

- `width()`: 获取渲染表面的宽度。

- `height()`: 获取渲染表面的高度。

- `back_buffer()`: 获取当前后备缓冲区的 Direct3D 12 资源对象指针。

- `rtv()`: 获取当前后备缓冲区的渲染目标视图描述符句柄。

- `viewport()`: 获取视口（Viewport）对象，用于控制渲染目标的显示区域。

- `scissor_rect()`: 获取裁剪矩形（Scissor Rectangle），用于指定渲染目标中的区域范围。



## 相机的封装`ChillEngine::graphics::d3d12::camera::d3d12_camera`

**构造函数**:

- `explicit d3d12_camera(camera_init_info info);` - 使用给定的初始化信息初始化相机对象。

**方法**:

- `void update();` - 更新相机的状态。
- `void up(math::v3 up);` - 设置相机的上方向。
- `void field_of_view(f32 fov);` - 设置相机的视野（FOV）。
- `void aspect_ratio(f32 aspect_ratio);` - 设置相机的宽高比。
- `void view_width(f32 width);` - 设置视图宽度（仅适用于正交相机）。
- `void view_height(f32 height);` - 设置视图高度（仅适用于正交相机）。
- `void near_z(f32 near_z);` - 设置近剪裁面。
- `void far_z(f32 far_z);` - 设置远剪裁面。

**常量获取方法**（带有 `[[nodiscard]]` 标记，表示这些方法返回的值不应被忽略）:

- `constexpr DirectX::XMMATRIX view() const` - 获取视图矩阵。
- `constexpr DirectX::XMMATRIX projection() const` - 获取投影矩阵。
- `constexpr DirectX::XMMATRIX inverse_projection() const` - 获取逆投影矩阵。
- `constexpr DirectX::XMMATRIX view_projection() const` - 获取视图投影矩阵。
- `constexpr DirectX::XMMATRIX inverse_view_projection() const` - 获取逆视图投影矩阵。
- `constexpr DirectX::XMVECTOR up() const` - 获取上方向向量。
- `constexpr DirectX::XMVECTOR position() const` - 获取位置向量。
- `constexpr DirectX::XMVECTOR direction() const` - 获取方向向量。
- `constexpr f32 near_z() const` - 获取近剪裁面距离。
- `constexpr f32 far_z() const` - 获取远剪裁面距离。
- `constexpr f32 field_of_view() const` - 获取视野（FOV）。
- `constexpr f32 aspect_ratio() const` - 获取宽高比。
- `constexpr f32 view_width() const` - 获取视图宽度（仅适用于正交相机）。
- `constexpr f32 view_height() const` - 获取视图高度（仅适用于正交相机）。
- `constexpr graphics::camera::type projection_type() const` - 获取投影类型。
- `constexpr id::id_type entity_id() const` - 获取实体ID。

**私有成员变量**:

- `DirectX::XMMATRIX _view;` - 视图矩阵。
- `DirectX::XMMATRIX _projection;` - 投影矩阵。
- `DirectX::XMMATRIX _inverse_projection;` - 逆投影矩阵。
- `DirectX::XMMATRIX _view_projection;` - 视图投影矩阵。
- `DirectX::XMMATRIX _inverse_view_projection;` - 逆视图投影矩阵。
- `DirectX::XMVECTOR _position;` - 位置向量。
- `DirectX::XMVECTOR _direction;` - 方向向量。
- `DirectX::XMVECTOR _up;` - 上方向向量。
- `f32 _near_z;` - 近剪裁面距离。
- `f32 _far_z;` - 远剪裁面距离。
- `union { f32 _field_of_view; f32 _view_width; };` - 透视相机的视野或正交相机的视图宽度。
- `union { f32 _aspect_ratio; f32 _view_height; };` - 透视相机的宽高比或正交相机的视图高度。
- `graphics::camera::type _projection_type;` - 投影类型。
- `id::id_type _entity_id;` - 实体ID。
- `bool _is_dirty;` - 标记相机是否需要更新。

**命名空间中的其他函数**:

- `graphics::camera create(camera_init_info info);` - 创建一个相机。
- `void remove(camera_id id);` - 移除一个相机。
- `void set_parameter(camera_id id, camera_parameter::parameter parameter, const void *const data, u32 data_size);` - 设置相机参数。
- `void get_parameter(camera_id id, camera_parameter::parameter parameter, void *const data, u32 data_size);` - 获取相机参数。
- `[[nodiscard]] d3d12_camera& get(camera_id id);` - 获取相机实例。



相机实例全部保管在全局变量 `utl::free_list<d3d12_camera> cameras;`



## ResourceBarrier的封装

​	资源屏障是在GPU访问资源时的用来防止出现资源冒险的。什么叫做资源冒险，在GPU的多个引擎中，我们看到复制引擎大多数情况下是和其他的两个引擎并行执行的，那么考虑一种场景，这个复制引擎在把一个资源拷贝给3D引擎使用的时候，还没有完全拷贝过来，而3D引擎就开始使用了，他就会读到一些不正确的未知的数据。这就叫资源冒险。

### `d3d12_resource_barrier`

​	封装了三种基本的Barrier:`D3D12_RESOURCE_BARRIER_TYPE_TRANSITION`,`D3D12_RESOURCE_BARRIER_TYPE_ALIASING`	
,`D3D12_RESOURCE_BARRIER_TYPE_UAV`

​	通过add()函数添加Barrier存储在成员变量，通过apply()函数通过调用ResourceBarrier通知驱动程序需要同步对资源的多个访问。



### Split barriers

当我们在设置barrier的时候需要设置一个barrierflag: **D3D12_RESOURCE_BARRIER_FLAG** 

他有三种：

我们一般对资源转换都会用第一种None即可，后两种BEGIN_ONLY和END_ONLY表示什么呢？

这需要提到一个概念Split barriers：

https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#split-barriers

> Split barriers provide hints to the GPU that a resource in state *A* will next be used in state *B* sometime later. This gives the GPU the option to optimize the transition workload, possibly reducing or eliminating execution stalls. Issuing the end-only barrier guarantees that all GPU transition work is finished before moving onto the next command.

也就是说当我们希望把一个资源状态从A转换到B的时候，barrier的转换语句执行完后就可以保证资源屏障转换完成了，然而我们有时并不需要这样及时的转换(到达下一个command前必须缓缓完成)， 我们可以分开，给一段转换的时间，仅仅告诉gpu我们的资源要从状态B一会儿要转换到状态B：这使 GPU 可以选择优化转换工作负载，从而可能减少或消除执行停顿。

也就是从BEGIN_ONLY开始转换，到END_ONLY之前一定会转换完成。

![img](https://bibo9ukvkgq.feishu.cn/space/api/box/stream/download/asynccode/?code=ZTZmNmU0OGExOWUwZmI3MTM3Y2E3YzI0MWQ2NzljMjNfYTdKQWNSR0lhYXJKb2cyQXJDc0M5Skc0YjZuR1lJRmdfVG9rZW46UzVNNGJ1elhsb0hYY0t4SUN5NWNjMll1bkRkXzE3MjA2Mjk4NTE6MTcyMDYzMzQ1MV9WNA)

但是请注意，Split Barrier不可以跨越一次ExecuteCommandLists，比如对于一个buffer，上一次提交命令的结尾加了Begin，这一次提交你希望加上End, 这是错误的。原因如下：

![img](https://bibo9ukvkgq.feishu.cn/space/api/box/stream/download/asynccode/?code=NjY5YzNkZGY4MzQxZWExYTQ5ZTM3OGY1ZTE4MWQ0ZDRfMWhJZUZMRzVCdWFoUmdBeXlNMzlOYVZNZGo0TWRSNFBfVG9rZW46VTNKRWJ5dTRSb3VLanF4Y0xieWNjRG9VbmllXzE3MjA2Mjk4NTE6MTcyMDYzMzQ1MV9WNA)



## RootSignature

### 使用

​		根签名定义了一系列根参数，这些参数可以是描述符表、根描述符或根常量。根参数可以直接绑定到管线上，或者指向描述符堆中的资源。根签名使得你可以将资源绑定到管线而不需要绑定整个资源表，这样可以大幅提高绑定资源的灵活性和性能。根签名描述清楚了渲染管线或者说Shader编译后的执行代码需要的各种资源以什么样的方式传入以及如何在内存、显存中布局。

​	**根签名的最大大小为64 DWORD。** 选择该最大大小是为了防止滥用根签名来存储大量数据。

```c++
// Maximum 64 DWORDs (u32's) divided up amongst all root parameters.
// Root constants = 1 DWORD per 32-bit constant
// Root descriptor  (CBV, SRV or UAV) = 2 DWORDs each
// Descriptor table pointer = 1 DWORD
// Static samplers = 0 DWORDs (compiled into shader)
struct d3d12_root_signature_desc : public D3D12_ROOT_SIGNATURE_DESC1
{

    constexpr static D3D12_ROOT_SIGNATURE_FLAGS default_flags{
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
            D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
    };

    constexpr explicit d3d12_root_signature_desc(const d3d12_root_parameter* parameters,
                                                 u32 parameter_count,
                                                 D3D12_ROOT_SIGNATURE_FLAGS flags = default_flags,
                                                 const D3D12_STATIC_SAMPLER_DESC* static_samplers = nullptr,
                                                 u32 sampler_count = 0)
        : D3D12_ROOT_SIGNATURE_DESC1{ parameter_count, parameters, sampler_count, static_samplers, flags }
    {}

    ID3D12RootSignature* create() const
    {
        return create_root_signature(*this);
    }
};
```



```c++
struct d3d12_root_parameter: public D3D12_ROOT_PARAMETER1
{
    constexpr void as_constants(u32 num_constants, D3D12_SHADER_VISIBILITY visibility, u32 shader_register, u32 register_space = 0)
    {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        ShaderVisibility = visibility;
        Constants.Num32BitValues = num_constants;
        Constants.ShaderRegister = shader_register;
        Constants.RegisterSpace = register_space;
    }

    constexpr void as_cbv(D3D12_SHADER_VISIBILITY visibility, u32 shader_register, u32 register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
    {
        as_descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shader_register, register_space, flags);
    }

    constexpr void as_srv(D3D12_SHADER_VISIBILITY visibility, u32 shader_register, u32 register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
    {
        as_descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shader_register, register_space, flags);
    }

    constexpr void as_uav(D3D12_SHADER_VISIBILITY visibility, u32 shader_register, u32 register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
    {
        as_descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shader_register, register_space, flags);
    }

    constexpr void as_descriptor_table(D3D12_SHADER_VISIBILITY visibility, const d3d12_descriptor_range* ranges, u32 range_count)
    {
        ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        ShaderVisibility = visibility;
        DescriptorTable.NumDescriptorRanges = range_count;
        DescriptorTable.pDescriptorRanges = ranges;
    }
    private:
    constexpr void as_descriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, u32 shader_register, u32 register_space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
    {
        ParameterType = type;
        ShaderVisibility = visibility;
        Descriptor.Flags = flags;
        Descriptor.ShaderRegister = shader_register;
        Descriptor.RegisterSpace = register_space;
    }
};

```

**使用示例：** 创建一个在pixel shader阶段可见常数的rootsig。 

```c++
using idx = fx_root_param_indices;
d3dx::d3d12_root_parameter parameters[idx::count]{};
parameters[idx::root_constants].as_constants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);

//remove info:dynamic resource don't need it anymore
//parameters[idx::descriptor_table].as_descriptor_table(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);

d3dx::d3d12_root_signature_desc root_signature{ &parameters[0], _countof(parameters) };
root_signature.Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
fx_root_sig = root_signature.create();
```

信息填充需要在render()函数中通过commandlist来把rootsig中的需要的数据真正添加。

### DynamicResource

​	可以通过SM6.6后一个叫做DynamicResource的功能，只需简单的SetDesciptorHeaps即可，不区分类型，shader中直接获取即可。

![img](https://bibo9ukvkgq.feishu.cn/space/api/box/stream/download/asynccode/?code=ZGMwMzhlY2M3MzgxYTY2M2FlYjY5N2I5NzhiOGFjODZfcVdQRUk4TTlubmtncmtxWXdsZm5NTXFFWnNGUGxYbWVfVG9rZW46U3h1ZGJsZmg2b0EyeXJ4ZXJuY2MzTXpZbnhiXzE3MjA2MjIzOTk6MTcyMDYyNTk5OV9WNA)

设置这个ResourceDescriptorHeap

![img](https://bibo9ukvkgq.feishu.cn/space/api/box/stream/download/asynccode/?code=MWZlNDRiNTdkZDcwMDNlNjQzY2I4YTQ1NjdkMGIxZTNfdnpHV3N5TktzRmR0OUJFc0dyVVFOMDdQRzE3N25LYTNfVG9rZW46Qlk5bmJBMEZ3bzU3S0p4RDA2ZWN0d3V0blJDXzE3MjA2MjIzOTk6MTcyMDYyNTk5OV9WNA)

​	DR 完全摆脱了 API 原生的 D3D12 Root Signature Descriptor Binding 机制，支持任意资源类型，任意资源访问方式，更是彻底解决了 StructuredBuffer 的自定义数据结构类型导致的组合爆炸问题。

## PSO

`ID3D12Device::CreateGraphicsPipelineState`可以设置的状态包括：

- 所有着色器的字节码，包括顶点、像素、域、外壳和几何着色器
- 输入顶点格式
- 原始拓扑类型
- 混合状态、光栅化器状态、深度模板状态
- 深度模板和渲染目标格式以及渲染目标计数
- 多重采样参数
- 流输出缓冲区
- 根签名



PSO中没有过度封装，只是做了一些易用的宏，现在你只需要这样配置stream，无需再定义很多类了:

```c++
            // Create fx PSO
            struct {
                d3dx::d3d12_pipeline_state_subobject_root_signature         root_signature{ fx_root_sig };
                d3dx::d3d12_pipeline_state_subobject_vs                     vs{ shaders::get_engine_shader(shaders::engine_shader::fullscreen_triangle_vs) };
                d3dx::d3d12_pipeline_state_subobject_ps                     ps{ shaders::get_engine_shader(shaders::engine_shader::post_process_ps) };
                d3dx::d3d12_pipeline_state_subobject_primitive_topology     primitive_topology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
                d3dx::d3d12_pipeline_state_subobject_render_target_formats  render_target_formats;
                d3dx::d3d12_pipeline_state_subobject_rasterizer             rasterizer{ d3dx::rasterizer_state.no_cull };
            } stream;

            D3D12_RT_FORMAT_ARRAY rtf_array{};
            rtf_array.NumRenderTargets = 1;
            rtf_array.RTFormats[0] = d3d12_surface::default_back_buffer_format;

            stream.render_target_formats = rtf_array;

            fx_pso = d3dx::create_pipeline_state(&stream, sizeof(stream));
```

其中创建PSO函数封装如下：

```c++
ID3D12PipelineState* create_pipeline_state(void* stream, u64 stream_size)
{
    assert(stream && stream_size);
    D3D12_PIPELINE_STATE_STREAM_DESC desc{};
    desc.SizeInBytes = stream_size;
    desc.pPipelineStateSubobjectStream = stream;
    return create_pipeline_state(desc);
}

ID3D12PipelineState* create_pipeline_state(D3D12_PIPELINE_STATE_STREAM_DESC desc)
{
    assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
    ID3D12PipelineState* pso{ nullptr };
    DXCall(core::device()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
    assert(pso);
    return pso;
}
```





## Content资源管理模块`ChillEngine::graphics::d3d12::content`

### 概念与数据组织

资源中我们有几个概念：submesh，texture，material，render_item。

如下图：

![image-20240711112819726](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240711112819726.png)

一个high level-render item = Geometry+List of materials

其中一个lowlevel-RenderItem具体结构如下：

![image-20240711112843285](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240711112843285.png)

### Material

#### 添加流程

添加一个材质`ChillEngine::content::create_material_resource`->

`graphics::add_material(*(const graphics::material_init_info *const)data)`->

`ChillEngine::graphics::d3d12::content::material::add`

```c++
struct material_init_info
{
    material_type::type type;
    u32                 texture_count;//NOTE: Textures are optional, it can be zero.
    id::id_type         shader_ids[shader_type::count] = {id::invalid_id, id::invalid_id, id::invalid_id, id::invalid_id, id::invalid_id, id::invalid_id, id::invalid_id, id::invalid_id};
    id::id_type*        texture_ids;

};

id::id_type add(material_init_info info)
{
    std::unique_ptr<u8[]> buffer;
    std::lock_guard lock{material_mutex};
    //create material from info
    d3d12_material_stream stream{buffer, info};

    assert(buffer);
    return materials.add(std::move(buffer));
}
```

关于把material加入我们写了一个`d3d12_material_stream`来实现，根据`material_init_info`load到数据后加入到全局材质`free_list`。

#### d3d12_material_stream

一个material数据需要包含以下内容：

材质类型，用到哪儿种shader，哪儿个shader，多少texture，哪儿些texture，texture对应的描述符，根签名。

我们把数据padding在一起。

![image-20240711140523106](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240711140523106.png)

所以一个Material的size是：

```c++
const u32 buffer_size = 
    sizeof(material_type::type) +               // material type
    sizeof(shader_flags::flags) +               // shader flags
    sizeof(id::id_type) +                       // root signature id
    sizeof(u32) +                               // texture count
    sizeof(id::id_type) * shader_count +        // shader ids
    (sizeof(id::id_type) + sizeof(u32)) * info.texture_count; // texture ids and descriptor indices (maybe 0 if no textures used).

```

开辟一个MaterialSize大的空间，向里面填充数据：

```c++
material_buffer = std::make_unique<u8[]>(buffer_size);
_buffer = material_buffer.get();
u8 *const buffer{ _buffer };

*(material_type::type*)buffer = info.type;
*(shader_flags::flags*)(&buffer[shader_flags_index]) = (shader_flags::flags)flags;
*(id::id_type*)(&buffer[root_signature_index]) = create_root_signature(info.type, (shader_flags::flags)flags);
*(u32*)(&buffer[texture_count_index]) = info.texture_count;

initialize();

if (info.texture_count)
{
    memcpy(_texture_ids, info.texture_ids, info.texture_count * sizeof(id::id_type));
    texture::get_descriptor_indices(_texture_ids, info.texture_count, _descriptor_indices);
}

u32 shader_index{ 0 };
for (u32 i{ 0 }; i < shader_type::count; ++i)
{
    if (id::is_valid(info.shader_ids[i]))
    {
        _shader_ids[shader_index] = info.shader_ids[i];
        ++shader_index;
    }
}

```

通过以上操作，我们就可以从一个material申请出一个包含信息的内存，存着这个material所包含的信息。

### submesh

#### 添加流程

submesh的存储内容为：

```
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
```

二进制数据时，我们要把其拆分成上述存储内容的结构然后存储在submesh_view中，最后添加至全局变量中`utl::free_list<submesh_view>        submesh_views;`中。

```
struct submesh_view
{
    D3D12_VERTEX_BUFFER_VIEW        position_buffer_view{};
    D3D12_VERTEX_BUFFER_VIEW        element_buffer_view{};
    D3D12_INDEX_BUFFER_VIEW         index_buffer_view{};
    D3D_PRIMITIVE_TOPOLOGY          primitive_topology;
    u32                             elements_type{};
};
```

#### utl::blob_stream_reader

​	一个二进制阅读器，方便读取padding紧密的数据块



### render_item

我们这里讨论的是high-level render item. low-level 不会暴漏给用户

high-level render item = Gemoetry + Material 

#### 利用`geometry_hierarchy_stream`添加geometry

​	`id::id_type create_mesh_hierarchy(const void *const data)` 中调用了`geometry_hierarchy_stream`来分析我们的二进制数据，我们.model二进制数据文件的数据组织如下：

![image-20240711180121876](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240711180121876.png)

分析时会开辟一个空间hierarchy_buffer来存储: `lod_count`,每个lod的`thresholds` 每个lod的`lod_offset`。

最后将hierarchy_buffer存储到全局的`utl::free_list<u8*>         geometry_hierarchies;`

到此我们有了几何数据。



#### Render Item

high-level render item = Gemoetry + Material ，参数需要几何体和材质。

添加时首先需要获取几何体的信息：我们从之前存储到geometry_hierarchies这个free_list中取出对应的gpu_id信息，gpu_id就是每个submesh的id。

```c++
    void get_submesh_gpu_ids(id::id_type geometry_content_id, u32 id_count, id::id_type* gpu_ids)
    {
        std::lock_guard lock {geometry_mutex};
        u8 *const pointer{geometry_hierarchies[geometry_content_id]};
        if((uintptr_t)pointer & single_mesh_marker)//single
        {
            assert(id_count == 1);
            *gpu_ids = gpu_id_from_fake_pointer(pointer);
        }
        else//hierarchies
        {
            geometry_hierarchy_stream stream {pointer};
            assert([&](){
                    const u32 lod_count{stream.lod_count()};
                    const lod_offset lod_offset{stream.lod_offsets()[lod_count - 1]};
                    const u32 total_count_of_gpu_id{(u32)lod_offset.offset + (u32)lod_offset.count};
                    return total_count_of_gpu_id == id_count;
                }());
            memcpy(gpu_ids, stream.gpu_ids(), sizeof(id::id_type) * id_count);
        }
    }
```

随后就是构建high-level render item，一个geometry id开始，随后每个low-level render item(submesh + material+ pso)，最后一个结束标识符。

构建出来的每个low-level render item会加入到`utl::free_list<d3d12_render_item>  render_items;`

high-level render item会加入到`utl::free_list<std::unique_ptr<id::id_type[]>>  render_item_ids;`



整体代码如下：

```c++
        id::id_type add(id::id_type entity_id, id::id_type geometry_content_id, u32 material_count, const id::id_type *const material_ids)
        {
            assert(id::is_valid(entity_id) && id::is_valid(geometry_content_id));
            assert(material_count && material_ids);
            id::id_type *const gpu_ids{ (id::id_type *const)alloca(material_count * sizeof(id::id_type)) }; //@todo: why material count??? call it submesh count is better? 
            ChillEngine::content::get_submesh_gpu_ids(geometry_content_id, material_count, gpu_ids);

            submesh::views_cache views_cache
            {
                (D3D12_GPU_VIRTUAL_ADDRESS *const)alloca(material_count * sizeof(D3D12_GPU_VIRTUAL_ADDRESS)),
                (D3D12_GPU_VIRTUAL_ADDRESS *const)alloca(material_count * sizeof(D3D12_GPU_VIRTUAL_ADDRESS)),
                (D3D12_INDEX_BUFFER_VIEW *const)alloca(material_count * sizeof(D3D12_INDEX_BUFFER_VIEW)),
                (D3D_PRIMITIVE_TOPOLOGY *const)alloca(material_count * sizeof(D3D_PRIMITIVE_TOPOLOGY)),
                (u32 *const)alloca(material_count * sizeof(u32))
            };

            submesh::get_views(gpu_ids, material_count, views_cache);

            // NOTE: the list of ids starts with geomtery id and ends with an invalid id to mark the end of the list.
            std::unique_ptr<id::id_type[]> items{ std::make_unique<id::id_type[]>(sizeof(id::id_type) * (1 + (u64)material_count + 1)) };

            items[0] = geometry_content_id;
            id::id_type *const item_ids{ &items[1] };

            std::lock_guard lock{ render_item_mutex };

            for (u32 i{ 0 }; i < material_count; ++i)
            {
                d3d12_render_item item{};
                item.entity_id = entity_id;
                item.submesh_gpu_id = gpu_ids[i];
                item.material_id = material_ids[i];
                
                pso_id id_pair{ create_pso(item.material_id, views_cache.primitive_topologies[i], views_cache.elements_types[i]) };
                item.pso_id = id_pair.gpass_pso_id;
                item.depth_pso_id = id_pair.depth_pso_id;

                assert(id::is_valid(item.submesh_gpu_id) && id::is_valid(item.material_id));
                item_ids[i] = render_items.add(item);
            }

            // mark the end of ids list.
            item_ids[material_count] = id::invalid_id;

            return render_item_ids.add(std::move(items));
        }
```

## Shader模块

​		该模块管理了所有的shader，



## 后处理模块

​	建立一个基本的后处理框架，该模块需要的成员变量仅包含RootSignature和PSO。





## 图形管线模块





