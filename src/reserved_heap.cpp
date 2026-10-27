// SPDX-License-Identifier: GPL-3.0-or-later
#include "reserved_heap.hpp"
#include <atomic>
#include <cstdint>
namespace reserved_heap {
namespace {
constexpr std::size_t page=16384,pages=1024;
alignas(page) unsigned char memory[page*pages];
unsigned short spans[pages]{};
std::size_t sizes[pages]{};
std::atomic_flag lock=ATOMIC_FLAG_INIT;
struct Guard {Guard(){while(lock.test_and_set(std::memory_order_acquire)){}} ~Guard(){lock.clear(std::memory_order_release);}};
}
bool owns(const void *p) noexcept {auto a=reinterpret_cast<std::uintptr_t>(p),b=reinterpret_cast<std::uintptr_t>(memory);return a>=b&&a-b<sizeof(memory);}
void *allocate(std::size_t bytes) noexcept {
 if(bytes>sizeof(memory))return nullptr;
 auto count=((bytes?bytes:1)+page-1)/page;Guard guard;
 for(std::size_t i=0,run=0;i<pages;i++){
  run=spans[i]?0:run+1;
  if(run==count){auto first=i+1-count;spans[first]=count;sizes[first]=bytes;
   for(auto j=first+1;j<=i;j++)spans[j]=0xffff;
   return memory+first*page;
  }
 }return nullptr;
}
void release(void *p) noexcept {
 if(!owns(p))return;auto offset=static_cast<unsigned char*>(p)-memory;if(offset%page)return;
 Guard guard;auto index=offset/page;auto count=spans[index];if(!count||count==0xffff)return;
 for(unsigned j=0;j<count;j++)spans[index+j]=0;sizes[index]=0;
}
std::size_t size(const void *p) noexcept {
 if(!owns(p))return 0;auto offset=static_cast<const unsigned char*>(p)-memory;if(offset%page)return 0;
 Guard guard;return sizes[offset/page];
}
}
extern "C" void __real_free(void *);
extern "C" void *__real_realloc(void *,std::size_t);
extern "C" void *calloc(std::size_t,std::size_t);
extern "C" void __wrap_free(void *p){if(reserved_heap::owns(p))reserved_heap::release(p);else __real_free(p);}
extern "C" void *__wrap_realloc(void *p,std::size_t n){
 if(!reserved_heap::owns(p))return __real_realloc(p,n);
 if(!n){reserved_heap::release(p);return nullptr;}
 auto old=reserved_heap::size(p);if(n<=old)return p;
 auto *out=static_cast<unsigned char*>(calloc(1,n));if(!out)return nullptr;
 for(std::size_t i=0;i<old;i++)out[i]=static_cast<unsigned char*>(p)[i];
 reserved_heap::release(p);return out;
}

