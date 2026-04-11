cmake_minimum_required(VERSION 3.25)

include(CheckCSourceCompiles)

project(STRUMOK_AVX2
    VERSION 0.0.1
    LANGUAGES C
)

option(STRUMOK_BUILD_LIB_ONLY "Build only the STRUMOK library." OFF)
option(STRUMOK_BUILD_SHARED "Build STRUMOK as a shared library (DLL/.so) instead of static." OFF)

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(WARNING
        "This project targets GCC 15+ for complete C23 support. "
        "Current compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}. "
        "Behaviour may differ or be incompatible."
    )
elseif(CMAKE_C_COMPILER_VERSION VERSION_LESS "15")
    message(WARNING
        "GCC 15 or later is required for full C23 support. "
        "Detected GCC ${CMAKE_C_COMPILER_VERSION}. "
        "Behaviour may differ or be incompatible."
    )
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(STRUMOK_AVX2_FLAG "")
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(STRUMOK_AVX2_FLAG "/arch:AVX2")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(STRUMOK_AVX2_FLAG "-mavx2")
else()
    message(FATAL_ERROR "Unsupported compiler for AVX2 build: ${CMAKE_C_COMPILER_ID}")
endif()

unset(STRUMOK_HAVE_AVX2 CACHE)
unset(STRUMOK_HAVE_AVX2)
set(_strumok_required_flags_backup "${CMAKE_REQUIRED_FLAGS}")
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${STRUMOK_AVX2_FLAG}")
check_c_source_compiles(
    "#include <immintrin.h>
     int main(void) {
         __m256i a = _mm256_set1_epi32(1);
         __m256i b = _mm256_set1_epi32(2);
         __m256i c = _mm256_xor_si256(a, b);
         return _mm256_extract_epi32(c, 0);
     }"
    STRUMOK_HAVE_AVX2
)
set(CMAKE_REQUIRED_FLAGS "${_strumok_required_flags_backup}")
unset(_strumok_required_flags_backup)

if(NOT STRUMOK_HAVE_AVX2)
    message(FATAL_ERROR "AVX2 support is required but was not detected with ${STRUMOK_AVX2_FLAG}.")
endif()

set(STRUMOK_API_SOURCES
    strumok.c
    strumok_tables.c
)

set(STRUMOK_LIBRARY_TYPE STATIC)
if(STRUMOK_BUILD_SHARED)
    set(STRUMOK_LIBRARY_TYPE SHARED)
endif()

add_library(STRUMOK ${STRUMOK_LIBRARY_TYPE} ${STRUMOK_API_SOURCES})
target_include_directories(STRUMOK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(STRUMOK_BUILD_SHARED AND WIN32)
    set_target_properties(STRUMOK PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(STRUMOK PRIVATE -O3)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(STRUMOK PRIVATE /O3)
endif()

message(STATUS "Building AVX2 target with ${STRUMOK_AVX2_FLAG}.")
target_compile_options(STRUMOK PRIVATE ${STRUMOK_AVX2_FLAG})

if(NOT STRUMOK_BUILD_LIB_ONLY)
    add_executable(${PROJECT_NAME}
        use-example.c
        strumok_selftest.c
        strumok_benchmark.c
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE STRUMOK)

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE ${STRUMOK_AVX2_FLAG})
endif()
