#pragma once
#include <cstdint>

// Monotonic time schedules emulation, never directly advances the run timer.
struct FramePacer {
    std::uint64_t previous=0;
    double credit=0;
    bool active=false;
    void reset() { previous=0;credit=0;active=false; }
    unsigned advance(std::uint64_t now,double fps,bool running) {
        if(!running){reset();return 0;}
        if(!now || fps<=0){reset();return 1;}
        if(!active){active=true;previous=now;return 1;}
        const auto elapsed=now>=previous?now-previous:0;
        previous=now;
        credit+=static_cast<double>(elapsed)*fps/1000000.0;
        // Bound catch-up after a long stall; never replay seconds of held input.
        if(credit>4.0)credit=4.0;
        unsigned frames=static_cast<unsigned>(credit);
        credit-=frames;
        return frames;
    }
};

// Either press order works; holding the chord does not retrigger on resume.
constexpr bool menu_chord(std::uint32_t held,std::uint32_t pressed) {
    constexpr std::uint32_t sticks=0x0002|0x0004;
    return (held&sticks)==sticks && (pressed&sticks)!=0;
}
