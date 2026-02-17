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
endif()

file(GLOB SOURCES src/*.cpp)
add_executable(Renderer WIN32 ${SOURCES})

target_compile_definitions(Renderer PRIVATE 
_UNICODE
UNICODE
)

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

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Renderer PRIVATE
        -Wall
        -Wextra
        -Wpedantic                  
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Woverloaded-virtual
        -fno-exceptions                 
    )

    # Optimization & debug info
    target_compile_options(Renderer PRIVATE
        $<$<CONFIG:Release>:
            -O3                    
            -flto                  
            -DNDEBUG
        >
        $<$<CONFIG:Debug>:
            -O0
            -g                      
            -fno-omit-frame-pointer 
        >
    )
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID} — no custom flags applied")
endif()

