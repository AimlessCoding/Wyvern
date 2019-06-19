cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0015 NEW)

project(WYVERN)

include_directories(contrib/include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message("64-bit libs")
	link_directories(contrib/lib/x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message("32-bit libs")
	link_directories(contrib/lib/x86)
endif()

#file(GLOB srcs "src/contrib/imgui/*.h" "src/contrib/imgui/*.cpp")

add_library(wyvern src/Wyvern.h src/Wyvern.cpp
	src/WyvObject.h src/WyvObject.cpp
	src/WyvWindow.h src/WyvWindow.cpp)

target_link_libraries(wyvern glfw3 vulkan-1)

if (MSVC)
	file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Debug/resources)
	file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Release/resources)
endif ()

if (${CMAKE_VERSION} VERSION_LESS 3.8.0)
	message("Need CMake 3.8 to set VS working directory")
else ()
	set_target_properties(wyvern PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
endif ()