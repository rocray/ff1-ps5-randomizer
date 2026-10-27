#include <cassert>
#include <cstdlib>
#include <cstdint>
extern "C" void *ff1_test_calloc(std::size_t,std::size_t);
int main(){
 auto *p=static_cast<unsigned char*>(ff1_test_calloc(262160,1));assert(p);
 for(unsigned i=0;i<262160;i++)assert(p[i]==0);
 free(p);assert(ff1_test_calloc(SIZE_MAX,2)==nullptr);
 p=static_cast<unsigned char*>(ff1_test_calloc(0,99));assert(p);free(p);
}

