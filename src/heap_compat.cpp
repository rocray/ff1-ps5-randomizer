// SPDX-License-Identifier: GPL-3.0-or-later
// The loader shim's heap table implements malloc/free but not calloc.
#include <cstddef>
#include <cstdint>
extern "C" void *malloc(std::size_t);
extern "C" __attribute__((visibility("hidden"))) void *calloc(std::size_t count,std::size_t size) {
    if(size && count>SIZE_MAX/size)return nullptr;
    const std::size_t bytes=count*size;
    void *p=malloc(bytes?bytes:1);
    if(!p)return nullptr;
    // Volatile stores prevent optimization back into a recursive calloc call.
    auto *out=static_cast<volatile unsigned char *>(p);
    for(std::size_t i=0;i<bytes;i++)out[i]=0;
    return p;
}
