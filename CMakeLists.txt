cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0015 NEW)

project(WYVERN_DEMO)

add_subdirectory(Wyvern)

include_directories(Wyvern/contrib/include)
include_directories(Wyvern/src)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message("64-bit libs")
	link_directories(Wyvern/contrib/lib/x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message("32-bit libs")
	link_directories(Wyvern/contrib/lib/x86)
endif()

#file(GLOB srcs "src/contrib/imgui/*.h" "src/contrib/imgui/*.cpp")

add_executable(wyverndemo src/main.cpp)

target_link_libraries(wyverndemo wyvern)

if (MSVC)
	file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/resources)
	file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Release/resources)
endif ()

if (${CMAKE_VERSION} VERSION_LESS 3.8.0)
	message("Need CMake 3.8 to set VS working directory")
else ()
	set_target_properties(wyverndemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
endif ()