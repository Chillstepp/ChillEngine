#include "ContentLoader.h"

#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "../Graphics/Render.h"

namespace ChillEngine::content
{
    namespace 
    {
        bool read_file(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size)
        {
            if (!std::filesystem::exists(path)) return false;

            size = std::filesystem::file_size(path);
            assert(size);
            if (!size) return false;
            data = std::make_unique<u8[]>(size);
            std::ifstream file{ path, std::ios::in | std::ios::binary };
            if (!file || !file.read((char*)data.get(), size))
            {
                file.close();
                return false;
            }

            file.close();
            return true;
        }   
    }

    //加载 已经编译好的shader
    bool load_engine_shaders(std::unique_ptr<u8[]>& shaders, u64& size)
    {
        auto path = graphics::get_engine_shaders_path();
        return read_file(path, shaders, size);
    }
}
