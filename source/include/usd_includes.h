#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // '=': conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4267) // '=': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4305) // '=': truncation from 'double' to 'float'
#pragma warning(disable: 4800) // '=': forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4996) // 'this': used in base member initializer list
#pragma warning(disable: 4100) // unused parameter
#pragma warning(disable: 4189) // local variable is initialized but not used
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#include <windows.h>
#undef small
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <boost/python/object.hpp>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>


#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
