#pragma clang attribute push (__attribute__((target("avx512f,avx512dq,avx512bw,bmi,popcnt"))), apply_to = any(function))

#include "common.h"

static
void CheckBitList(const unsigned* bl1, unsigned bl1_cnt,
                  const unsigned* bl2, unsigned bl2_cnt)
{
    assert(bl1_cnt == bl2_cnt);
    if (bl1_cnt != bl2_cnt)
    {
        fmt::print(stderr, "Check list count failed!\n");
        exit(1);
    }
    for (unsigned i = 0; i < bl1_cnt; ++i)
    {
        assert(bl1[i] == bl2[i]);
        if (bl1[i] != bl2[i])
        {
            fmt::print(stderr, "BitList check failed!\n");
            exit(1);
        }
    }
}

static
void BM_foreach_bitscan_nibble_switch(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum1(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; ++j) {
            sum1 += bm::bitscan_nibble(test_arr[j], bit_list);
        }
    }
    benchmark::DoNotOptimize(sum1++);
    delete [] test_arr;
}

#ifdef BM_NONSTANDARD_EXTENTIONS
#ifdef __GNUC__
static
void BM_foreach_bitscan_nibble_gccgoto(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum2(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; ++j)
        {
            sum2 += bm::bitscan_nibble_gcc(test_arr[j], bit_list);
        }
    }
    benchmark::DoNotOptimize(sum2++);
    delete [] test_arr;
}
#endif
#endif

static
void BM_foreach_bitscan_popcnt(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; j+=2)
        {
            sum += bm::bitscan_popcnt(test_arr[j], bit_list);
            sum += bm::bitscan_popcnt(test_arr[j + 1], bit_list);
        }
    }
    benchmark::DoNotOptimize(sum++);
    delete [] test_arr;
}

static
void BM_foreach_bitscan_popcnt64(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; j+=2)
        {
            bm::id64_t w0 = test_arr[j];
            bm::id64_t w1 = test_arr[j+1];
            bm::id64_t w = w0 | (w1 << 32);
            sum += bm::bitscan_popcnt64(w, bit_list);
        }
    }
    benchmark::DoNotOptimize(sum++);
    delete [] test_arr;
}

static
void BM_foreach_bitscan_bsf(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; j+=2)
        {
            sum += bm::bitscan_bsf(test_arr[j], bit_list);
            sum += bm::bitscan_bsf(test_arr[j + 1], bit_list);
        }
    }
    benchmark::DoNotOptimize(sum++);
    delete [] test_arr;
}

static
void BM_foreach_bitscan_bsf64(benchmark::State& state) {
    // setup the test data
    //
    unsigned value_to = 65536 * 64;
    size_t value = 0;
    auto *test_arr = new unsigned[value_to];
    {
        unsigned j = 0;
        for (; j < value_to / 3; ++j) // sparse
        {
            test_arr[j] = (1u << 25) | (1u << 7) | 2u | 4u;
            test_arr[++j] = 0;
        }
        for (; j < 2 * (value_to / 3); ++j) // medium
            test_arr[j] = j;

        for (; j < value_to; ++j) // dense
            test_arr[j] = ~0u & ~((7u << 25) | (19u << 7) | 2u | 4u);
    }

    // quick test
    {
        unsigned bit_list0[32];
        unsigned bit_list1[32];
        unsigned bit_list2[32];
        for (unsigned j = 0; j < value_to; ++j) {
            auto c0 = bm::bitscan_nibble(test_arr[j], bit_list0);
            auto c1 = bm::bitscan_popcnt(test_arr[j], bit_list1);
            auto c2 = bm::bitscan_bsf(test_arr[j], bit_list2);
            CheckBitList(bit_list0, c0, bit_list1, c1);
            CheckBitList(bit_list0, c0, bit_list2, c2);
        }
    }

    bm::id64_t sum(0);
    unsigned bit_list[32];

    for (auto _: state) {
        for (unsigned j = 0; j < value_to; j+=2)
        {
            bm::id64_t w0 = test_arr[j];
            bm::id64_t w1 = test_arr[j+1];
            bm::id64_t w = w0 | (w1 << 32);
            sum += bm::bitscan_bsf64(w, bit_list);
        }
    }
    benchmark::DoNotOptimize(sum++);
    delete [] test_arr;
}

BENCHMARK(BM_foreach_bitscan_nibble_switch);
BENCHMARK(BM_foreach_bitscan_nibble_gccgoto);
BENCHMARK(BM_foreach_bitscan_popcnt);
BENCHMARK(BM_foreach_bitscan_popcnt64);
BENCHMARK(BM_foreach_bitscan_bsf);
BENCHMARK(BM_foreach_bitscan_bsf64);

#pragma clang attribute pop