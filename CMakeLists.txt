cmake_minimum_required(VERSION 3.23)

project(CommonCPP)

add_library(CommonCPP INTERFACE)
target_include_directories(CommonCPP INTERFACE .)
target_sources(CommonCPP
	INTERFACE
		FILE_SET HEADERS
		FILES
			gl/color.hpp
			gl/window.hpp
			math/half.hpp
			math/interpolation.hpp
			math/random.hpp
			math/util.hpp
			util/binary.hpp
			util/bitfield.hpp
			util/cache.hpp
			util/debug.hpp
			util/files.hpp
			util/iterable.hpp
			util/movingAverage.hpp
			util/progressBar.hpp
			util/sets.hpp
			util/string.hpp
			util/sync.hpp
			util/threadpool.hpp
			util/timer.hpp
			util/windows.hpp
)