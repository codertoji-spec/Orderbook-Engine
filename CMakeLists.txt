cmake_minimum_required(VERSION 3.20)
project(orderbook_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wnull-dereference")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")

if (ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

if (ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

include_directories(include)

file(GLOB SOURCES "src/*.cpp")
file(GLOB TEST_SOURCES "tests/*.cpp")
file(GLOB BENCHMARK_SOURCES "benchmarks/*.cpp")

# Require pthreads on Linux
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Executables
add_executable(matching_engine src/main.cpp src/order_book_map.cpp src/order_book_optimized.cpp src/matching_engine.cpp src/tcp_server.cpp)
target_link_libraries(matching_engine Threads::Threads)

add_executable(benchmark benchmarks/benchmark_main.cpp src/order_book_map.cpp src/order_book_optimized.cpp src/matching_engine.cpp)
target_link_libraries(benchmark Threads::Threads)

add_executable(stress_test benchmarks/stress_test.cpp src/order_book_map.cpp src/order_book_optimized.cpp src/matching_engine.cpp)
target_link_libraries(stress_test Threads::Threads)

add_executable(order_client tools/order_client.cpp src/tcp_client.cpp)
target_link_libraries(order_client Threads::Threads)

# Unit Tests (using simple assertions for now to avoid external dependencies like GTest if not available, or we can just fetch content)
# To keep it simple and dependency free, we'll write a lightweight test runner in tests/
add_executable(unit_tests tests/test_main.cpp tests/order_book_tests.cpp tests/matching_tests.cpp tests/cancellation_tests.cpp tests/concurrency_tests.cpp src/order_book_map.cpp src/order_book_optimized.cpp src/matching_engine.cpp)
target_link_libraries(unit_tests Threads::Threads)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)
