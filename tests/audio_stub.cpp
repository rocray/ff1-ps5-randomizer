#include "../src/title_music.hpp"
std::size_t test_audio_frames=0;bool test_audio_nonzero=false;
namespace ps5::demo::music {
void submit(const std::int16_t *p,std::size_t n) noexcept {test_audio_frames+=n;for(std::size_t i=0;i<n*2;i++)if(p[i])test_audio_nonzero=true;}
}

