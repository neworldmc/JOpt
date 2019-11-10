#include <new>
#include "u.h"

void *
operator new(std::size_t size, Region &r)
{
    return ralloc(&r, (int)size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
}

void *
operator new(std::size_t size, std::align_val_t align, Region &r)
{
    return ralloc(&r, (int)size, (int)align);
}

void *
operator new[](std::size_t size, Region &r)
{
    return ralloc(&r, (int)size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
}

void *
operator new[](std::size_t size, std::align_val_t align, Region &r)
{
    return ralloc(&r, (int)size, (int)align);
}
