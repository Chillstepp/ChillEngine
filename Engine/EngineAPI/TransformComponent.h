﻿#pragma once
#include "../Components/ComponentsCommonHeaders.h"

namespace ChillEngine::transform
{
    DEFINE_TYPE_ID(transform_id);

    class component final
    {
    public:
        constexpr explicit component(transform_id id) : _id(id) {}
        constexpr component() : _id(id::invalid_id) {}
        constexpr transform_id get_id() const { return _id;}
        constexpr bool is_valid() const {return id::is_valid(_id);}

        math::v3 position() const;
        math::v4 rotation() const;
        math::v3 scale() const;
    private:
        transform_id _id;
    };
}