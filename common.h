#ifndef BITMAGICBENCH_COMMON_H
#define BITMAGICBENCH_COMMON_H

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

extern std::random_device rand_dev;
extern std::mt19937 gen; // mersenne_twister_engine
extern std::uniform_int_distribution<> rand_dis;

#endif //BITMAGICBENCH_COMMON_H
