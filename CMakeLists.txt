cmake_minimum_required(VERSION 3.10)
project(PSNN VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(psnn_core STATIC src/core.cpp)
target_include_directories(psnn_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(psnn_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(psnn src/main.cpp)
target_link_libraries(psnn PRIVATE psnn_core)
target_compile_options(psnn PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
    add_executable(psnn_tests tests/test_core.cpp)
    target_link_libraries(psnn_tests PRIVATE psnn_core)
    target_compile_options(psnn_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME offline_core COMMAND psnn_tests)
endif()

# Compiles the game-scoped lifecycle ABI on the host only. A Linux .so is
# NOT a PlayStation 4 PRX and must never be installed on a console.
option(PSNN_BUILD_HOST_SHIM "Compile diagnostic-only plugin shim on host" ON)
if(PSNN_BUILD_HOST_SHIM)
    add_library(psnn_plugin_shim SHARED ps4/plugin_entry.c)
    target_include_directories(psnn_plugin_shim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_options(psnn_plugin_shim PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(PSNN_BUILD_HOST_SHIM AND BUILD_TESTING)
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME plugin_shim_host_abi
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/test_shim.py $<TARGET_FILE:psnn_plugin_shim>)
    endif()
endif()
