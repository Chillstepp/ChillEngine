/*std:*/
#include <filesystem>
#include <fstream>
/*latest DXshaderCompiler*/
#include "../packages/DirectXShaderCompiler/inc/d3d12shader.h"
#include "../packages/DirectXShaderCompiler/inc/dxcapi.h"

#include "ShaderCompilation.h"
#include "../Engine/Content/ContentToEngine.h"
#include "../Engine/Graphics/Direct3D12/D3D12Core.h"
#include "../Engine/Graphics/Direct3D12/D3D12Shaders.h"

#pragma comment(lib, "../packages/DirectXShaderCompiler/lib/x64/dxcompiler.lib")



using namespace ChillEngine;
using namespace ChillEngine::graphics::d3d12::shaders;
using namespace Microsoft::WRL;

namespace
{
    constexpr const char* shaders_source_path{ "../Engine/Graphics/Direct3D12/Shaders/" };
    
    struct engine_shader_info
    {
        engine_shader::id id;
        shader_file_info info;
    };
    constexpr engine_shader_info engine_shader_files[]
    {
        engine_shader::fullscreen_triangle_vs, {"FullScreenTriangle.hlsl", "FullScreenTriangleVS", shader_type::vertex},
        engine_shader::fill_color_ps, {"FillColor.hlsl", "FillColorPS", shader_type::pixel},
        engine_shader::post_process_ps, {"PostProcess.hlsl", "PostProcessPS", shader_type::pixel}
    };

    static_assert(_countof(engine_shader_files) == engine_shader::count);

    struct dxc_compiled_shader
    {
        ComPtr<IDxcBlob>        byte_code;
        ComPtr<IDxcBlobUtf8>    disassembly;
        DxcShaderHash           hash;
    };

    std::wstring to_wstring(const char* c)
    {
        std::string s{ c };
        return { s.begin(), s.end() };
    }

    class shader_compiler
    {
    public:
        shader_compiler()
        {
            HRESULT hr = S_OK;
            DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler)));
            if(FAILED(hr)) return;
            DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils)));
            if(FAILED(hr)) return;
            DXCall(hr = _utils->CreateDefaultIncludeHandler(&_include_handler));
            if(FAILED(hr)) return;
        }
        DISABLE_COPY_AND_MOVE(shader_compiler);
        dxc_compiled_shader compile(shader_file_info info, std::filesystem::path full_path)
        {
            assert(_compiler && _utils && _include_handler);
            HRESULT hr{ S_OK };

            // Load the source file using Utils interface.
            ComPtr<IDxcBlobEncoding> source_blob{ nullptr };
            DXCall(hr = _utils->LoadFile(full_path.c_str(), nullptr, &source_blob));
            if (FAILED(hr)) return {};
            assert(source_blob && source_blob->GetBufferSize());

            std::wstring file{ to_wstring(info.file_name) };
            std::wstring func{ to_wstring(info.function) };
            std::wstring prof{ to_wstring(_profile_strings[(u32)info.type]) };
            std::wstring inc { to_wstring(shaders_source_path)};

            //compile parameter
            LPCWSTR args[]
            {
                file.c_str(),                       // Optional shader source file name for error reporting
                L"-E", func.c_str(),                // Entry function
                L"-T", prof.c_str(),                // Target profile
                L"-I", inc.c_str(),                 //include path
                L"-enable-16bit-types",             //support  16-bits integer in shader
                DXC_ARG_ALL_RESOURCES_BOUND,
    #if _DEBUG
                DXC_ARG_DEBUG,
                DXC_ARG_SKIP_OPTIMIZATIONS,
    #else
                DXC_ARG_OPTIMIZATION_LEVEL3,
    #endif
                DXC_ARG_WARNINGS_ARE_ERRORS,
                L"-Qstrip_reflect",                 // Strip reflections into a separate blob
                L"-Qstrip_debug",                   // Strip debug information into a separate blob
            };

            OutputDebugStringA("Compiling ");
            OutputDebugStringA(info.file_name);
            OutputDebugStringA(":");
            OutputDebugStringA(info.function);
            OutputDebugStringA("\n");
            

            return compileInternal(source_blob.Get(), args, _countof(args));
        }

        
        dxc_compiled_shader compileInternal(IDxcBlobEncoding* source_blob, LPCWSTR* args, u32 num_args)
        {
            DxcBuffer buffer{};
            buffer.Encoding = DXC_CP_ACP; // auto-detect text format, I guess?
            buffer.Ptr = source_blob->GetBufferPointer();
            buffer.Size = source_blob->GetBufferSize();

            HRESULT hr{ S_OK };
            ComPtr<IDxcResult> results{ nullptr };
            DXCall(hr = _compiler->Compile(&buffer, args, num_args, _include_handler.Get(), IID_PPV_ARGS(&results)));
            if (FAILED(hr)) return {};

            ComPtr<IDxcBlobUtf8> errors{ nullptr };
            DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
            if (FAILED(hr)) return {};

            if (errors && errors->GetStringLength())
            {
                OutputDebugStringA("\nShader compilation error: \n");
                OutputDebugStringA(errors->GetStringPointer());
            }
            else
            {
                OutputDebugStringA(" [ Succeeded ]");
            }
            OutputDebugStringA("\n");

            HRESULT status{ S_OK };
            DXCall(hr = results->GetStatus(&status));
            if (FAILED(hr) || FAILED(status)) return {};

            
            ComPtr<IDxcBlob> shader{ nullptr };
            DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
            if (FAILED(hr)) return {};
            
            ComPtr<IDxcBlob> hash { nullptr };
            DXCall(hr = results->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr));
            if (FAILED(hr)) return {};

            DxcShaderHash *const hash_buffer = (DxcShaderHash *const)hash->GetBufferPointer();
            assert(!(hash_buffer->Flags & DXC_HASHFLAG_INCLUDES_SOURCE));
            OutputDebugStringA("Shader hash:");
            for(u32 i = 0; i < _countof(hash_buffer->HashDigest); ++i)
            {
                char hash_bytes[3]{};// 2 chars for hex value plus termination 0.
                sprintf_s(hash_bytes, "%02x", hash_buffer->HashDigest[i]);
                OutputDebugStringA(hash_bytes);
                OutputDebugStringA("");
            }
            OutputDebugStringA("\n");

            buffer.Ptr = shader->GetBufferPointer();
            buffer.Size = shader->GetBufferSize();

            ComPtr<IDxcResult> disasm_results{ nullptr};
            DXCall(hr = _compiler->Disassemble(&buffer, IID_PPV_ARGS(&disasm_results)));
            ComPtr<IDxcBlobUtf8> disassembly {nullptr};
            DXCall(hr = disasm_results->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr));

            dxc_compiled_shader result{shader.Detach(), disassembly.Detach()};
            memcpy(&result.hash.HashDigest[0], &hash_buffer->HashDigest[0], _countof(hash_buffer->HashDigest));
            return result;
        }
    private:
        constexpr static const char* _profile_strings[]{ "vs_6_6", "hs_6_6", "ds_6_6", "gs_6_6", "ps_6_6", "cs_6_6", "as_6_6", "ms_6_6" };
        static_assert(_countof(_profile_strings) == shader_type::count);
        
        ComPtr<IDxcCompiler3>       _compiler;
        ComPtr<IDxcUtils>           _utils;
        ComPtr<IDxcIncludeHandler>  _include_handler;
    };

    // Get the path to the compiled shaders binary file.
    decltype(auto) get_engine_shaders_path()
    {
        return std::filesystem::absolute(graphics::get_engine_shaders_path(graphics::graphics_platform::direct3d12));
    }
    
    bool compiled_shaders_are_up_to_date()
    {
        //return false;
        auto engine_shaders_path = get_engine_shaders_path();
        if (!std::filesystem::exists(engine_shaders_path)) return false;
        auto shaders_compilation_time = std::filesystem::last_write_time(engine_shaders_path);
        
        std::filesystem::path full_path{};

        // Check if either of engine shader source files is newer than the compiled shader file.
        // In that case, we need to recompile.
        for (u32 i{ 0 }; i < engine_shader::count; ++i)
        {
            auto& file = engine_shader_files[i];

            full_path = shaders_source_path;
            full_path += file.info.file_name;
            if (!std::filesystem::exists(full_path)) return false;

            auto shader_file_time = std::filesystem::last_write_time(full_path);
            if (shader_file_time > shaders_compilation_time)
            {
                return false;
            }
        }

        return true;
    }
    

    bool save_compiled_shaders(utl::vector<dxc_compiled_shader>& shaders)
    {
        auto engine_shaders_path = get_engine_shaders_path();
        std::filesystem::create_directories(engine_shaders_path.parent_path());
        std::ofstream file(engine_shaders_path, std::ios::out | std::ios::binary);
        if (!file || !std::filesystem::exists(engine_shaders_path))
        {
            file.close();
            return false;
        }

        for (auto& shader : shaders)
        {
            const D3D12_SHADER_BYTECODE byte_code{ shader.byte_code->GetBufferPointer(), shader.byte_code->GetBufferSize() };
            file.write((char*)&byte_code.BytecodeLength, sizeof(byte_code.BytecodeLength));
            file.write((char*)&shader.hash.HashDigest[0], _countof(shader.hash.HashDigest));
            file.write((char*)byte_code.pShaderBytecode, byte_code.BytecodeLength);
        }

        file.close();
        return true;
    }

    
    
}

std::unique_ptr<u8[]> compile_shader(shader_file_info info, const char* file_path)
{
    std::filesystem::path full_path = file_path;
    full_path += info.file_name;
    auto x = std::filesystem::absolute(full_path);
    if(!std::filesystem::exists(full_path)) return {};
    // NOTE: according to marcelolr (https://github.com/Microsoft/DirectXShaderCompiler/issues/79)
    //       "...creating compiler instances is pretty cheap, so it's probably not worth the hassle of caching / sharing them."
    shader_compiler compiler{};
    dxc_compiled_shader compiled_shader{compiler.compile(info, full_path)};

    if (compiled_shader.byte_code && compiled_shader.byte_code->GetBufferPointer() && compiled_shader.byte_code->GetBufferSize())
    {
        static_assert(content::compiled_shader::hash_length == _countof(DxcShaderHash::HashDigest));
        const u64 buffer_size{ sizeof(u64) + content::compiled_shader::hash_length + compiled_shader.byte_code->GetBufferSize() };
        std::unique_ptr<u8[]> buffer{ std::make_unique<u8[]>(buffer_size) };
        utl::blob_stream_writer blob{ buffer.get(), buffer_size };
        blob.write(compiled_shader.byte_code->GetBufferSize());
        blob.write(compiled_shader.hash.HashDigest, content::compiled_shader::hash_length);
        blob.write((u8*)compiled_shader.byte_code->GetBufferPointer(), compiled_shader.byte_code->GetBufferSize());;

        assert(blob.offset() == buffer_size);
        return buffer;
    }
    return {};
}

bool compile_shaders()
{
    if(compiled_shaders_are_up_to_date()) return true;
    utl::vector<dxc_compiled_shader> shaders;
    std::filesystem::path full_path{};

    shader_compiler compiler{};
    // compile shaders and them together in a buffer in the same order of compilation.
    for (u32 i{ 0 }; i < engine_shader::count; ++i)
    {
        auto& file = engine_shader_files[i];

        full_path = shaders_source_path;
        full_path += file.info.file_name;
        if (!std::filesystem::exists(full_path)) return false;
        dxc_compiled_shader compiled_shader{ compiler.compile(file.info, full_path) };
        if (compiled_shader.byte_code && compiled_shader.byte_code->GetBufferPointer() && compiled_shader.byte_code->GetBufferSize())
        {
            shaders.emplace_back(std::move(compiled_shader));
        }
        else
        {
            return false;
        }
    }

    return save_compiled_shaders(shaders);
}