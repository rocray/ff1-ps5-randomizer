// SPDX-License-Identifier: GPL-3.0-or-later
// The loader shim's heap table implements malloc/free but not calloc.
#include <cstddef>
#include <cstdint>
extern "C" void *malloc(std::size_t);
extern "C" int posix_memalign(void **,std::size_t,std::size_t);
static std::size_t failed_bytes=0;
static bool allocation_failed=false;
extern "C" void ff1_heap_reset() { allocation_failed=false; failed_bytes=0; }
extern "C" const char *ff1_heap_error() {
    if(!allocation_failed)return nullptr;
    static char message[80];
    const char *prefix="BOTH ALLOCATORS FAILED: ";unsigned n=0;
    while(prefix[n]){message[n]=prefix[n];++n;}
    char digits[24];unsigned count=0;auto v=failed_bytes;
    do{digits[count++]=char('0'+v%10);v/=10;}while(v);
    while(count)message[n++]=digits[--count];
    const char *suffix=" BYTES";for(unsigned i=0;suffix[i];++i)message[n++]=suffix[i];
    message[n]=0;return message;
}
extern "C" __attribute__((visibility("hidden"))) void *calloc(std::size_t count,std::size_t size) {
    if(size && count>SIZE_MAX/size){allocation_failed=true;failed_bytes=SIZE_MAX;return nullptr;}
    const std::size_t bytes=count*size;
    void *p=malloc(bytes?bytes:1);
    if(!p) {
        // Standard aligned allocations use the same free-compatible ownership.
        void *aligned=nullptr;
        if(posix_memalign(&aligned,64,bytes?bytes:1)==0) p=aligned;
    }
    if(!p){allocation_failed=true;failed_bytes=bytes;return nullptr;}
    // Volatile stores prevent optimization back into a recursive calloc call.
    auto *out=static_cast<volatile unsigned char *>(p);
    for(std::size_t i=0;i<bytes;i++)out[i]=0;
    return p;
}
