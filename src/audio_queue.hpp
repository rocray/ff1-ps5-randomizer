#pragma once
#include <atomic>
#include <cstdint>
#include <cstddef>
struct AudioQueue {
 static constexpr unsigned capacity=8192;
 std::int16_t samples[capacity*2]{};
 unsigned read_at=0,write_at=0,count=0;
 std::atomic_flag lock=ATOMIC_FLAG_INIT;
 void acquire(){while(lock.test_and_set(std::memory_order_acquire)){}}
 void release(){lock.clear(std::memory_order_release);}
 void clear(){acquire();read_at=write_at=count=0;release();}
 void push(const std::int16_t *p,std::size_t n){
  acquire();for(std::size_t i=0;i<n&&count<capacity;i++){
   samples[write_at*2]=p[i*2];samples[write_at*2+1]=p[i*2+1];write_at=(write_at+1)%capacity;++count;
  }release();
 }
 unsigned pop(std::int16_t *p,unsigned n){
  acquire();unsigned got=0;for(unsigned i=0;i<n;i++){
   if(count){p[i*2]=samples[read_at*2];p[i*2+1]=samples[read_at*2+1];read_at=(read_at+1)%capacity;--count;++got;}
   else p[i*2]=p[i*2+1]=0;
  }release();return got;
 }
};

