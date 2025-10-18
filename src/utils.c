#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void util_inf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);
}

void util_err(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);
}

void util_fat(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    va_end(ap);

    exit(EXIT_FAILURE);
}
