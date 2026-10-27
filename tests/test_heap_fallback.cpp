#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdint>
extern "C" void *ff1_test_calloc(std::size_t,std::size_t);
extern "C" void ff1_heap_reset();
extern "C" const char *ff1_heap_error();
static bool deny=false;
static unsigned calls=0;
extern "C" void *ff1_test_malloc(std::size_t){return nullptr;}
extern "C" int ff1_test_aligned(void **p,std::size_t a,std::size_t n){
 ++calls;if(deny)return 12;
 int result=posix_memalign(p,a,n);if(!result)std::memset(*p,0xa5,n);return result;
}
int main(){
 ff1_heap_reset();
 auto *p=static_cast<unsigned char*>(ff1_test_calloc(262144,1));assert(p);
 assert(reinterpret_cast<std::uintptr_t>(p)%64==0);
 for(unsigned i=0;i<262144;i++)assert(p[i]==0);
 free(p);assert(!ff1_heap_error());assert(calls==1);
 deny=true;assert(!ff1_test_calloc(262144,1));
 assert(!std::strcmp(ff1_heap_error(),"BOTH ALLOCATORS FAILED: 262144 BYTES"));
 ff1_heap_reset();assert(!ff1_heap_error());
 unsigned previous=calls;assert(!ff1_test_calloc(SIZE_MAX,2));assert(calls==previous);
}
