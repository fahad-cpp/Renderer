cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Renderer LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(GLOB SOURCES src/*.cpp)
add_subdirectory(vendor/FSWindow)
add_executable(Renderer)
target_sources(Renderer PRIVATE ${SOURCES})

set_target_properties(Renderer PROPERTIES WIN32_EXECUTABLE FALSE)
target_link_libraries(Renderer PRIVATE FSWindow)
target_compile_definitions(Renderer PRIVATE
_UNICODE
UNICODE
)
set_property(TARGET Renderer PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
if(MSVC)
    target_compile_options(Renderer PRIVATE
        /permissive-
        /W3
        /WX-
        /EHsc
        /GS
        /sdl
        /Zc:wchar_t
        /Zc:inline
        /Zc:forScope
        /fp:precise
        /Oi
        /Gy
        /Zi
    )

    target_compile_options(Renderer PRIVATE
        $<$<CONFIG:Release>:
            /O2
            /GL
        >
        $<$<CONFIG:Debug>:
            /Od
            /RTC1
        >
    )

    target_link_options(Renderer PRIVATE
        /SUBSYSTEM:CONSOLE
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    target_compile_options(Renderer PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Woverloaded-virtual
        -pedantic-errors
    )

    # Optimization & debug info
    target_compile_options(Renderer PRIVATE
        $<$<CONFIG:Release>:
            -O2
            -march=native
            -flto
            -DNDEBUG
            -D_NDEBUG
        >
        $<$<CONFIG:Debug>:
            -O0
            -ggdb
            -fno-omit-frame-pointer
        >
    )
    target_link_options(Renderer PRIVATE -fuse-ld=lld -pthread)
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID} — no custom flags applied")
endif()
