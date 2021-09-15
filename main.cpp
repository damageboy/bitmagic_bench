/*
Copyright(c) 2002-2017 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

For more information please visit:  http://bitmagic.io
*/

#pragma clang attribute push (__attribute__((target("avx512f,avx512dq,avx512bw"))), apply_to = any(function))
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <benchmark/benchmark.h>
#include <bitset>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <cassert>
#include <vector>
#include <random>
#include <memory>
#include <cmath>

//#define BMWASMSIMDOPT
//#define BMSSE2OPT
//#define BMSSE42OPT
#define BMAVX2OPT
//#define BMAVX512OPT
#define BM_NONSTANDARD_EXTENTIONS

#include <src/bm.h>
#include <src/bmalgo.h>
#include <src/bmintervals.h>
#include <src/bmaggregator.h>
#include <src/bmserial.h>
#include <src/bmsparsevec.h>
#include <src/bmsparsevec_algo.h>
#include <src/bmsparsevec_serial.h>
#include <src/bmstrsparsevec.h>
#include <src/bmsparsevec_compr.h>
#include <src/bmrandom.h>
//#include "bmdbg.h"

//#define BM64ADDR
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996)
#endif

using namespace std;

const unsigned int BSIZE = 150000000;
const unsigned int REPEATS = 100;

typedef  bitset<BSIZE>  test_bitset;
unsigned platform_test = 1;

std::random_device rand_dev;
std::mt19937 gen(rand_dev()); // mersenne_twister_engine
std::uniform_int_distribution<> rand_dis(0, BSIZE); // generate uniform numebrs for [1, vector_max]

typedef bm::bvector<> bvect;

// generate pseudo-random bit-vector, mix of compressed/non-compressed blocks
//
static
void generate_bvector(bvect& bv, unsigned vector_max = 40000000, bool optimize = true)
{
    unsigned i, j;
    for (i = 0; i < vector_max;)
    {
        // generate bit-blocks
        {
            bvect::bulk_insert_iterator iit(bv);
            for (j = 0; j < 65535 * 10; i += 10, j++)
            {
                iit = i;
            }
        }
        if (i > vector_max)
            break;
        // generate GAP (compressed) blocks
        for (j = 0; j < 65535; i += 120, j++)
        {
            unsigned len = rand() % 64;
            bv.set_range(i, i + len);
            i += len;
            if (i > vector_max)
                break;
        }
    }
    if (optimize)
        bv.optimize();
}

static
void SimpleFillSets(test_bitset* bset,
                    bvect& bv,
                    unsigned min,
                    unsigned max,
                    unsigned fill_factor,
                    bool set_flag=true)
{
    for (unsigned i = min; i < max; i+=fill_factor)
    {
        if (bset)
            (*bset)[i] = set_flag;
        bv[i] = set_flag;
    } // for i
}


//
// Interval filling.
// 111........111111........111111..........11111111.......1111111...
//
static
void FillSetsIntervals(test_bitset* bset,
                       bm::bvector<> &bv,
                       unsigned min = 0,
                       unsigned max = BSIZE,
                       unsigned fill_factor = 10,
                       bool set_flag=true)
{
    srand(666);
    while(fill_factor==0)
    {
        fill_factor=rand()%10;
    }

    unsigned i, j;
    unsigned factor = 10 * fill_factor;
    for (i = min; i < max; ++i)
    {
        unsigned len, end;

        do
        {
            len = unsigned(rand()) % factor;
            end = i+len;

        } while (end >= max);
        for (j = i; j < end; ++j)
        {
            if (set_flag)
            {
                if (bset)
                    bset->set(j, true);
                bv[j]= true;
            }
            else
            {
                if (bset)
                    bset->set(j, false);
                bv[j] = false;
            }

        } // j
        i = end;
        len = rand() % 10;

        i+=len;
        {
            for(unsigned k=0; k < 1000 && i < max; k+=3,i+=3)
            {
                if (set_flag)
                {
                    if (bset)
                        bset->set(i, true);
                    bv[i] = true;
                }
                else
                {
                    if (bset)
                        bset->set(j, false);
                    bv[j] = false;
                }
            }
        }

    } // for i

}

static
void generate_sparse_bvector(bvect& bv,
                             unsigned min = 0,
                             unsigned max = BSIZE,
                             unsigned fill_factor = 65536)
{
    bvect::bulk_insert_iterator iit(bv);
    unsigned ff = fill_factor / 10;
    for (unsigned i = min; i < max; i+= ff)
    {
        //bv.set(i);
        iit = i;
        ff += ff / 2;
        if (ff > fill_factor)
            ff = fill_factor / 10;
    }
    iit.flush();
}


static
void GenerateTestCollection(std::vector<bvect>* target,
                            unsigned count = 30,
                            unsigned vector_max = 40000000,
                            bool optimize = true)
{
    assert(target);
    bvect bv_common; // sub-vector common for all collection
    generate_sparse_bvector(bv_common, vector_max/10, vector_max, 250000);

    unsigned cnt1 = (count / 2);

    unsigned i = 0;

    for (i = 0; i < cnt1; ++i)
    {
        std::unique_ptr<bvect> bv (new bvect);
        generate_bvector(*bv, vector_max, optimize);
        *bv |= bv_common;
        if (optimize)
            bv->optimize();
        target->push_back(std::move(*bv));
    } // for

    unsigned fill_factor = 10;
    for (; i < count; ++i)
    {
        std::unique_ptr<bvect> bv (new bvect);

        FillSetsIntervals(0, *bv, vector_max/ 10, vector_max, fill_factor);
        *bv |= bv_common;

        target->push_back(std::move(*bv));
    } // for
}

static
void BM_handrolled_memcpy(benchmark::State& state) {
    unsigned *m1 = new unsigned[BSIZE / 32];
    unsigned *m2 = new unsigned[BSIZE / 32];

    unsigned int i, j;

    for (auto _: state) {
        for (j = 0; j < BSIZE / 32; j += 4) {
            m1[j + 0] += m2[j + 0];
            m1[j + 1] += m2[j + 1];
            m1[j + 2] += m2[j + 2];
            m1[j + 3] += m2[j + 3];
        }
    }

    delete [] m1;
    delete [] m2;
}

BENCHMARK(BM_handrolled_memcpy);

static
void BM_libc_memcpy(benchmark::State& state) {
    unsigned *m1 = new unsigned[BSIZE / 32];
    unsigned *m2 = new unsigned[BSIZE / 32];

    for (auto _: state) {
        benchmark::DoNotOptimize(memcpy(m1, m2, BSIZE/32 * sizeof(unsigned)));
    }

    delete [] m1;
    delete [] m2;
}

BENCHMARK(BM_libc_memcpy);

static
void BM_bvector_count(benchmark::State& state) {
    bm::bvector<> *bv = new bvect();
    test_bitset *bset = new test_bitset();
    unsigned value = 0;
    FillSetsIntervals(bset, *bv, 0, BSIZE, 10);

    {
        auto c0 = bset->count();
        auto c1 = bv->count();
        assert(c0 == c1);
        if (c0 != c1) {
            fmt::print("Fill sets integrity failed!\n");
            exit(1);
        }
    }

    for (auto _: state) {
        benchmark::DoNotOptimize(value += bv->count());
    }

    delete bset;
    delete bv;
}

BENCHMARK(BM_bvector_count);

static
void BM_std_bitset_count(benchmark::State& state) {
    bm::bvector<> *bv = new bvect();
    test_bitset *bset = new test_bitset();
    unsigned value = 0;
    FillSetsIntervals(bset, *bv, 0, BSIZE, 10);

    for (auto _: state) {
        benchmark::DoNotOptimize(value += (unsigned)bset->count());
    }

    delete bset;
    delete bv;
}

BENCHMARK(BM_std_bitset_count);


const size_t KB = 1024;
const size_t MB = KB * 1024;
const size_t GB = MB * 1024;
const size_t MEM_SIZE = 64*GB;
const size_t BLOCK_SIZE = 4 * KB;
const size_t NUM_BITS = MEM_SIZE / BLOCK_SIZE;

BENCHMARK_MAIN();
#pragma clang attribute pop