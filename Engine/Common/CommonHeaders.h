#pragma once

//c++
#include <cstdint>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>

#if defined(_WIN64)
#include <DirectXMath.h>
#endif


#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x) (void(0))
#endif


#ifndef DISABLE_COPY
#define DISABLE_COPY(T)                     \
explicit T(const T&) = delete;  \
T& operator=(const T&) = delete;
#endif

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)                 \
explicit T(T&&) = delete;   \
T& operator=(T&&) = delete;
#endif

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif

//common headers
#include "PrimitiveTypes.h"
#include "../Utilities/MathTypes.h"
#include "../Utilities/Utilities.h"
#include "Id.h"