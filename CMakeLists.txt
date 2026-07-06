cmake_minimum_required(VERSION 3.23)

project(CommonCPP)

set(CMAKE_CXX_STANDARD 23)

add_library(CommonCPP)

target_sources(CommonCPP
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS .
)

add_subdirectory(util)
add_subdirectory(gl)
add_subdirectory(math)

target_include_directories(CommonCPP INTERFACE .)