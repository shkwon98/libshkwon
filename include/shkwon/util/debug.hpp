#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace shkwon
{
void Assert(bool condition, const char *message, ...)
{
    if (!condition)
    {
        va_list args;
        va_start(args, message);
        vfprintf(stderr, message, args);
        va_end(args);
        exit(EXIT_FAILURE);
    }
}
} // namespace shkwon