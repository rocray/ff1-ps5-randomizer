#include "../src/reserved_heap.hpp"
#include <cassert>
#include <cstring>
#include <cstdlib>
extern "C" void __wrap_free(void*);
extern "C" void *__wrap_realloc(void*,std::size_t);
extern "C" void __real_free(void*p){std::free(p);}
extern "C" void *__real_realloc(void*p,std::size_t n){return std::realloc(p,n);}
int main(){
 void *blocks[64];for(auto &p:blocks){p=reserved_heap::allocate(262144);assert(p);}
 assert(!reserved_heap::allocate(1));for(auto p:blocks)__wrap_free(p);
 auto *p=static_cast<unsigned char*>(reserved_heap::allocate(262144));assert(p);std::memset(p,0x5a,262144);
 auto *q=static_cast<unsigned char*>(__wrap_realloc(p,524288));assert(q);
 for(unsigned i=0;i<262144;i++)assert(q[i]==0x5a);__wrap_free(q);
 auto *large=reserved_heap::allocate(16*1024*1024);assert(large);__wrap_free(large);
 assert(!reserved_heap::allocate(16*1024*1024+1));
}
