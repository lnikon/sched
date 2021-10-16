cmake_minimum_required(VERSION 3.18)
project(cpp-project-template)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_compile_options(-Wall -Wextra -pedantic -O0 -g)
set(sources "src/main.cpp")

### Threading library
set(THREADING_LIB_SRC "src/coroutine.cpp")
add_library(Threading STATIC ${THREADING_LIB_SRC})

### Main
add_executable(main ${sources})
target_link_libraries(main ${CONAN_LIBS} Threads::Threads)

### Unit Testing
enable_testing()

include(CTest)
include(ParseAndAddCatchTests)
include(Catch)

add_executable(test_thread_safe_queue ${CMAKE_SOURCE_DIR}/tests/test_thread_safe_queue.cpp)
target_link_libraries(test_thread_safe_queue ${CONAN_LIBS} Threads::Threads)
catch_discover_tests(test_thread_safe_queue)

add_executable(test_thread_pool ${CMAKE_SOURCE_DIR}/tests/test_thread_pool.cpp)
target_link_libraries(test_thread_pool ${CONAN_LIBS} Threads::Threads)
catch_discover_tests(test_thread_pool)

add_executable(test_coroutine ${CMAKE_SOURCE_DIR}/tests/test_coroutine.cpp)
target_link_libraries(test_coroutine ${CONAN_LIBS} Threads::Threads Threading)
catch_discover_tests(test_coroutine)
