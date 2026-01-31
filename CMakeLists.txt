cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(WINDOWS_SUBSYTEM "WINDOWS")


project(Renderer LANGUAGES CXX)

if(MSVC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
	message(STATUS "Using Microslop visual studio")
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
	message(STATUS "Not Using Microslop visual studio")
	message(STATUS "Generator: " ${CMAKE_GENERATOR})
	message(STATUS "MSVC     : " ${MSVC})
	message(STATUS "CXX ID   : " ${CMAKE_CXX_COMPILER_ID})
	message(STATUS "CXX comp : " ${CMAKE_CXX_COMPILER})
endif()

file(GLOB SOURCES src/*.cpp)
add_executable(Renderer WIN32 ${SOURCES})

target_compile_definitions(Renderer PRIVATE 
_UNICODE
UNICODE
)

if(MSVC)
    # Your original MSVC flags, made more target-oriented
    target_compile_options(Renderer PRIVATE
        /permissive-                # strict conformance
        /W3                         # warning level
        /WX-                        # warnings not errors
        /EHsc                       # standard exception handling
        /GS                         # buffer security check
        /sdl                        # extra SDL checks
        /Zc:wchar_t                 # native wchar_t
        /Zc:inline                  # remove unreferenced COMDATs
        /Zc:forScope                # for-loop scope conformance
        /fp:precise                 # precise FP semantics
        /Oi                         # enable intrinsics
        /Gy                         # function-level linking
        /Zi                         # debug info (even in Release)
    )

    target_compile_options(Renderer PRIVATE
        $<$<CONFIG:Release>:
            /O2                     # speed optimization
            /GL                     # whole-program optimization
        >
        $<$<CONFIG:Debug>:
            /Od                     # no optimization (CMake default, but explicit)
            /RTC1                   # runtime checks
        >
    )

    # Runtime: dynamic (/MD) in Release, /MDd in Debug (CMake default when not set)
    # Whole-program opt needs LTCG → CMake adds /LTCG automatically when /GL is seen

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Reasonable cross-platform equivalents
    target_compile_options(Renderer PRIVATE
        -Wall
        -Wextra
        -Wpedantic                  # closest to /permissive- spirit (strict + warnings)
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Woverloaded-virtual
        -fno-exceptions             # if you want -fno-rtti too → add it
        # -Werror                   # uncomment only if you want warnings-as-errors
    )

    # Optimization & debug info
    target_compile_options(Renderer PRIVATE
        $<$<CONFIG:Release>:
            -O2                     # or -O3 / -Ofast if you prefer more aggressive
            -flto                   # link-time optimization ≈ /GL
            -DNDEBUG
        >
        $<$<CONFIG:Debug>:
            -O0
            -g                      # debug symbols
            -fno-omit-frame-pointer # better stack traces
        >
    )

    # Optional: sanitizers in Debug (very useful during development)
    # target_compile_options(Renderer PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    # target_link_options   (Renderer PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)

else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID} — no custom flags applied")
endif()

