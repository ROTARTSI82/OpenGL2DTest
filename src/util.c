//
// Created by grant on 6/2/21.
//

#include "util.h"

// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp
#if defined(_MSC_VER)
#   define BIG_CONSTANT(x) (x)
// Other compilers
#else	// defined(_MSC_VER)
#   define BIG_CONSTANT(x) (x##LLU)
#endif // !defined(_MSC_VER)

#include "config.h"
#include "common.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <GLFW/glfw3.h>

void init_fps_limiter(fps_limiter_t *limiter, uint64_t target_frametime) {
    limiter->target_frametime = target_frametime;
    clock_gettime(CEL_CLOCK_SRC, &limiter->last_tick);
}

double timespec_to_sec(struct timespec *spec) {
    return spec->tv_sec + spec->tv_nsec / 1000000000.0;
}

void tick_fps_limiter(fps_limiter_t *limiter) {
    struct timespec now;
    clock_gettime(CEL_CLOCK_SRC, &now);
    // PRINT("now = %lf\n", timespec_to_sec(&now));

    uint64_t elapsed_time = (now.tv_sec - limiter->last_tick.tv_sec) * 1000000000UL + (now.tv_nsec - limiter->last_tick.tv_nsec);
    if (elapsed_time < limiter->target_frametime) {
        struct timespec to_sleep;
        to_sleep.tv_nsec = limiter->target_frametime - elapsed_time;
        to_sleep.tv_sec = to_sleep.tv_nsec / 1000000000L;
        to_sleep.tv_nsec %= 1000000000L;

        clock_nanosleep(CEL_CLOCK_SRC, 0, &to_sleep, NULL);

        limiter->last_tick.tv_nsec += limiter->target_frametime;
        limiter->last_tick.tv_sec += limiter->last_tick.tv_nsec / 1000000000L;
        limiter->last_tick.tv_nsec %= 1000000000L;
    } else {
        limiter->last_tick = now;
    }
}

uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed )
{
    const uint64_t m = BIG_CONSTANT(0xc6a4a7935bd1e995);
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while(data != end)
    {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
        case 7: h ^= (uint64_t) data2[6] << 48;
        case 6: h ^= (uint64_t) data2[5] << 40;
        case 5: h ^= (uint64_t) data2[4] << 32;
        case 4: h ^= (uint64_t) data2[3] << 24;
        case 3: h ^= (uint64_t) data2[2] << 16;
        case 2: h ^= (uint64_t) data2[1] << 8;
        case 1: h ^= (uint64_t) data2[0];
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

// DOOM RNG!!
uint8_t rand_table[256] = {0x6f, 0x14, 0x73, 0xe0, 0x27, 0x66, 0xe6, 0x56, 0x00, 0x75, 0x3c, 0x05, 0xa6, 0x0e, 0x69,
                           0x71, 0xbf, 0x94, 0xd6, 0xa3, 0x4e, 0xf6, 0xef, 0x6e, 0x3c, 0x99, 0x1f, 0x09, 0x8e, 0x67,
                           0x40, 0x48, 0xf4, 0x98, 0x43, 0xb1, 0xfc, 0x3e, 0x22, 0x34, 0x54, 0xf6, 0x88, 0xe3, 0x22,
                           0x52, 0x6f, 0xff, 0x96, 0x27, 0x5d, 0x5a, 0x7a, 0x24, 0x77, 0xb0, 0x49, 0x17, 0xd2, 0x9e,
                           0x5f, 0x42, 0xe6, 0xe3, 0xa2, 0x68, 0xc8, 0x24, 0xe6, 0x23, 0x1f, 0x31, 0x47, 0xfb, 0xab,
                           0xc3, 0x6a, 0xb2, 0xb6, 0x1c, 0x14, 0x54, 0x27, 0xeb, 0x41, 0x2d, 0xb1, 0x42, 0x4d, 0x64,
                           0x79, 0xb4, 0x76, 0xe2, 0x0b, 0x9a, 0x1f, 0xf1, 0xd5, 0x80, 0xb9, 0x67, 0xed, 0x6c, 0x1a,
                           0xa8, 0xb7, 0x79, 0xd3, 0x4c, 0xc7, 0xc2, 0xa9, 0x32, 0x99, 0x1f, 0x2b, 0xeb, 0xb5, 0x4d,
                           0x30, 0xed, 0xb5, 0x81, 0x17, 0xe4, 0x72, 0xb1, 0x18, 0xc7, 0x03, 0x39, 0x87, 0xb8, 0x42,
                           0x41, 0xc8, 0xd3, 0xb0, 0x5a, 0x74, 0x2d, 0x76, 0x91, 0x7f, 0x4c, 0x31, 0x49, 0x67, 0x4b,
                           0xd1, 0x8a, 0xff, 0x3f, 0xd7, 0xbe, 0xe1, 0xf5, 0x2c, 0x7f, 0x45, 0x1c, 0x68, 0x57, 0x30,
                           0x49, 0x5d, 0xf6, 0xe7, 0x50, 0xb0, 0xd0, 0x5e, 0xcf, 0xb2, 0x6b, 0x7f, 0x88, 0x95, 0x3f,
                           0x06, 0xf0, 0xf7, 0x16, 0xe6, 0x89, 0x53, 0xf5, 0x54, 0x92, 0x34, 0x0d, 0xc5, 0x7c, 0xd2,
                           0xf5, 0xa8, 0xd2, 0x0a, 0x00, 0x3c, 0x0d, 0xcc, 0x8c, 0x4a, 0xe7, 0x80, 0xae, 0x8b, 0xb3,
                           0x55, 0x4f, 0x51, 0x3a, 0xa9, 0x35, 0x4e, 0x71, 0x6c, 0x75, 0x78, 0x3f, 0x96, 0x77, 0x38,
                           0xd5, 0x97, 0x2b, 0x65, 0x02, 0xd3, 0x09, 0x10, 0x8c, 0x62, 0x66, 0xf8, 0xdb, 0xd7, 0xf0,
                           0x09, 0x95, 0x28, 0x54, 0xb8, 0x10, 0xa5, 0x9f, 0xc4, 0xd1, 0x6c, 0x74, 0xe1, 0xfe, 0x99, 0x19};

uint64_t reverse_bits(uint64_t in) {
    uint64_t out = 0;
    for (int i = 0; i < 64; i++) {
        out <<= 1;
        out |= in & 1;
        in >>= 1;
    }
    return out;
}

uint64_t reverse_bytes(uint64_t in) {
    uint64_t out = 0;
    for (int i = 0; i < 8; i++) {
        out <<= 8;
        out |= in & 0xFF;
        in >>= 8;
    }

    return out;
}

uint64_t rand_nextl(uint64_t x) {
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x;
}

uint8_t rand_nextb(uint8_t x) {
    x ^= x << 3;
    x ^= x >> 4;
    x ^= x << 1;
    return x;
}

uint16_t rand_nexts(uint16_t x) {
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 3;
    return x;
}

uint32_t rand_nexti(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

uint8_t is_little_endian() {
    uint16_t number = 0x1;
    uint8_t *numPtr = (uint8_t *)&number;
    return (numPtr[0] == 1);
}


