#pragma once

#include "../Common/CommonHeaders.h"

namespace ChillEngine::content
{
    struct primitive_topology
    {
        enum type: u32{
            point_list = 1,
            line_list,
            line_strip,
            triangle_list,
            triangle_strip,

            count
        };
    };
}
