cmake_minimum_required(VERSION 3.17)
project(
	EvoEngine-RayTracer
	VERSION 0.1
	DESCRIPTION "An implementation of MLVQ library in CUDA with OptiX ray tracer."
)
# Set a default build type if none was specified
set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	# Set the possible values of build type for cmake-gui
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release")
endif ()
# ------------------------------------------------------------------
# 3rd Party libraries
# ------------------------------------------------------------------
# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(EVOENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../EvoEngine)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT TARGET evoengine)
	add_subdirectory(${EVOENGINE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/EvoEngine)
endif ()

include(${CMAKE_MODULE_PATH}/configure_build_type.cmake)
include(${CMAKE_MODULE_PATH}/configure_optix.cmake)
mark_as_advanced(CUDA_SDK_ROOT_DIR)
include_directories(${OptiX_INCLUDE})

# ------------------------------------------------------------------
# RayTracer
# ------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
enable_language(CUDA)

file(GLOB LOCAL_EVOENGINE_RAYTRACER_SOURCES
	"src/lib/*.cpp"
	"src/lib/MLVQ/*.cpp"
	)

file(GLOB EVOENGINE_RAYTRACER_CUDA_SOURCES_LOCAL
	"src/lib/*.cu"
	)

include_directories(${EVOENGINE_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/lib/MLVQ)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/lib/RayTracer)
cuda_compile_and_embed(CAMERA_RENDERING_PTX ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ptx/CameraRendering.cu)
cuda_compile_and_embed(ILLUMINATION_ESTIMATION_PTX ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ptx/IlluminationEstimation.cu)
cuda_compile_and_embed(POINT_CLOUD_SCANNING_PTX ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ptx/PointCloudScanning.cu)

add_library(evoengine-raytracer
	STATIC
	#Default
	${CAMERA_RENDERING_PTX}
	${ILLUMINATION_ESTIMATION_PTX}
	${POINT_CLOUD_SCANNING_PTX}

	#C++
	${LOCAL_EVOENGINE_RAYTRACER_SOURCES}
	${EVOENGINE_RAYTRACER_CUDA_SOURCES_LOCAL}
	${CMAKE_MODULE_PATH}/configure_optix.cmake
	${CMAKE_MODULE_PATH}/FindOptiX.cmake
	)

set_property(TARGET evoengine-raytracer PROPERTY CUDA_ARCHITECTURES 61-real 61-virtual)

if (NOT WIN32)
	message(STATUS CUDALIB=${CUDA_TOOLKIT_ROOT_DIR})
	set(CUDA_CUDA_LIBRARY ${CUDA_TOOLKIT_ROOT_DIR}/lib64/stubs)
endif ()
message(STATUS EVOENGINE_INCLUDES:${EVOENGINE_INCLUDES})
message(STATUS OptiX_INCLUDE:${OptiX_INCLUDE})
message(STATUS CUDA_LIBRARIES:${CUDA_LIBRARIES})
message(STATUS CUDA_CUDA_LIBRARY:${CUDA_CUDA_LIBRARY})

# Linker settings for all platforms
target_link_libraries(evoengine-raytracer
	evoengine
	cuda
	#${CUDA_LIBRARIES}
	#${CUDA_CUDA_LIBRARY}
	)

set(LOCAL_EVOENGINE_RAYTRACER_INCLUDES
	${OptiX_INCLUDE}
	${EVOENGINE_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR}
	${CUDA_TOOLKIT_ROOT_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/lib
	${CMAKE_CURRENT_SOURCE_DIR}/include/lib/MLVQ
	${CMAKE_CURRENT_SOURCE_DIR}/include/lib/RayTracer
	${CMAKE_CURRENT_SOURCE_DIR}/include/lib/Utilities
	)

target_include_directories(evoengine-raytracer
	PRIVATE
	${LOCAL_EVOENGINE_RAYTRACER_INCLUDES}
	)
if (WIN32)
	set(CUDA_PROPAGATE_HOST_FLAGS ON)
endif ()

if (WIN32)
	# Compiler settings for Windows platform
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif (UNIX AND NOT APPLE)
	if (${CMAKE_BUILD_TYPE} STREQUAL Release)
		set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	elseif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
		set(CMAKE_CXX_FLAGS_DEBUG "-g")
	endif ()
elseif (APPLE)
endif ()

# ------------------------------------------------------------------
# Example program
# ------------------------------------------------------------------
if (${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	add_executable(Application
		"src/app/Application.cpp"
		src/lib/PointCloudScanner.cpp include/lib/Utilities/PointCloudScanner.hpp)
	target_precompile_headers(Application
		PRIVATE
		${EVOENGINE_PCH}
		)
	target_include_directories(Application
		PRIVATE
		${LOCAL_EVOENGINE_RAYTRACER_INCLUDES}
		)
	target_link_libraries(Application
		evoengine-raytracer
		)
endif ()
# ------------------------------------------------------------------
# Copy Internal resources
# ------------------------------------------------------------------
file(COPY src/app/imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (NOT ${CMAKE_BINARY_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	set(EVOENGINE_RAYTRACER_INCLUDES ${LOCAL_EVOENGINE_RAYTRACER_INCLUDES} PARENT_SCOPE)
endif ()