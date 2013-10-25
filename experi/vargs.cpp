#include <cstring>
#include <cstdio>

void vastuff(const char* format, ...) {
    va_list vformat_args;
    va_start(vformat_args, format)