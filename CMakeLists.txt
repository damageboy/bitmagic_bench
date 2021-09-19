cmake_minimum_required(VERSION 3.18)
project(bitmagicbench)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#8.0.1")
CPMAddPackage("gh:neargye/magic_enum#v0.7.3")

set(BMOPTFLAGS BMAVX2OPT)


CPMAddPackage("gh:tlk00/BitMagic#v7.5.0")

set(BENCHMARK_ENABLE_TESTING NO)
CPMAddPackage("gh:google/benchmark#v1.6.0")

find_package(Threads REQUIRED) # for pthread

add_executable(bitmagicbench main.cpp bitscan.cpp common.h)

target_link_libraries(bitmagicbench
	PUBLIC
	fmt::fmt
	magic_enum
	bm
	benchmark
	Threads::Threads
)
