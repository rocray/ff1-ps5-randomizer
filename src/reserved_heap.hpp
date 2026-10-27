#pragma once
#include <cstddef>
namespace reserved_heap {
void *allocate(std::size_t) noexcept;
bool owns(const void *) noexcept;
void release(void *) noexcept;
std::size_t size(const void *) noexcept;
}
