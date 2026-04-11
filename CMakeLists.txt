cmake_minimum_required(VERSION 3.25)

include(CheckCSourceCompiles)

project(STRUMOK_AVX2
    VERSION 0.0.1
    LANGUAGES C
)

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

option(STRUMOK_ENABLE_AVX2 "Enable AVX2 optimizations when available" ON)

set(STRUMOK_AVX2_FLAG "")
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(STRUMOK_AVX2_FLAG "/arch:AVX2")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(STRUMOK_AVX2_FLAG "-mavx2")
endif()

if(STRUMOK_ENABLE_AVX2 AND NOT STRUMOK_AVX2_FLAG STREQUAL "")
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
else()
    set(STRUMOK_HAVE_AVX2 OFF)
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS
    *.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(STRUMOK_ENABLE_AVX2)
    if(STRUMOK_HAVE_AVX2)
        message(STATUS "AVX2 support detected. Building with ${STRUMOK_AVX2_FLAG}.")
        target_compile_options(${PROJECT_NAME} PRIVATE ${STRUMOK_AVX2_FLAG})
        target_compile_definitions(${PROJECT_NAME} PRIVATE STRUMOK_HAVE_AVX2=1)
    else()
        message(WARNING "AVX2 support was not detected. Building without AVX2 flags.")
    endif()
endif()
