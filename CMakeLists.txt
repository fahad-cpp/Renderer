cmake_minimum_required(VERSION 3.10)
set(WINDOWS_SUBSYTEM "WINDOWS")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
project(Renderer)

file(GLOB SOURCES src/*.cpp)
add_executable(Renderer WIN32 ${SOURCES})

target_compile_definitions(Renderer PRIVATE 
	_UNICODE
	UNICODE
)