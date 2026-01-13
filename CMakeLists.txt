cmake_minimum_required(VERSION 3.10)

project(Renderer)

file(GLOB SOURCES src/*.cpp)
add_executable(Renderer WIN32 ${SOURCES})

add_subdirectory(tracy)
target_link_libraries(Renderer PRIVATE Tracy::TracyClient user32 kernel32)

set(WINDOWS_SUBSYTEM "WINDOWS")

target_compile_definitions(Renderer PRIVATE 
	_UNICODE
	 UNICODE
	 NDEBUG
)

target_compile_options(Renderer PUBLIC -O3)