cmake_minimum_required(VERSION 3.9)
project(Menu CXX OBJCXX)

# CXX standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options:
option(SANITIZE "Build with sanitizer support if available" OFF)
option(VENDOR "Vendor libraries" OFF)
# CMAKE_BUILD_TYPE Release Debug

set(SRC
src/main.cpp
)

set(COCOA_SRC
src/menu.mm

include/menu.h
)

# Check debug
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are:
		 Debug Release"
      	FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	message("Debug enabled")
	set(DEBUG TRUE)
endif()

if(DEBUG)
	add_definitions(-D_DEBUG)
else()
	add_definitions(-DNDEBUG)
endif()

# Check platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(ANDROID TRUE)
	set(BUILD_NAME main)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(MACOSX TRUE)
	set(BUILD_NAME Menu)
	# Find cocoa
	find_library(COCOA_LIBRARY Cocoa)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS TRUE)
	set(BUILD_NAME Menu)
else()
	set(BUILD_NAME Menu)
endif()

# Sanitize
if(SANITIZE STREQUAL ON)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()

# Optimization flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Check LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTOSupported OUTPUT error)

include_directories(include)

if(NOT ANDROID)
	if(VENDOR)
    		add_subdirectory(external/SDL3 EXCLUDE_FROM_ALL)
	else()
    		find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
	endif()
endif()

# Find and include SDL2
# Don't need for Android
if (ANDROID)
	message("Building for Android")

	add_library(${BUILD_NAME} SHARED)

	target_sources(${BUILD_NAME} PRIVATE ${SRC})

	target_link_libraries(${BUILD_NAME} PUBLIC SDL3::SDL3)
elseif(MACOSX)
	message("Building for MacOS")
	include_directories(${SDL3_INCLUDE_DIRS})
	add_executable(${BUILD_NAME} MACOSX_BUNDLE ${SRC} ${COCOA_SRC})            
	set_target_properties(${BUILD_NAME} PROPERTIES
	    BUNDLE True
	    MACOSX_BUNDLE_ICON_FILE ${CMAKE_SOURCE_DIR}/cmake/Pong.ico
	    MACOSX_BUNDLE_GUI_IDENTIFIER org.cyao.${BUILD_NAME}
	    MACOSX_BUNDLE_BUNDLE_NAME ${BUILD_NAME}
	    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
	    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
	    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/info.plist.in
	)
	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3 ${COCOA_LIBRARY})
elseif(WINDOWS)
	message("Building for windows")
	add_executable(${BUILD_NAME} WIN32 ${SRC})
	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3)
else()
	message("Building for linux (probably)")

	include_directories(${SDL3_INCLUDE_DIRS})

	add_executable(${BUILD_NAME} ${SRC})

	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3)
endif()


if(ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXE
		NAMES "clang-tidy"
		DOC "Path to clang-tidy executable")
        if(NOT CLANG_TIDY_EXE)
		message(STATUS "clang-tidy not found.")
        else()
		message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
		set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
        endif()

        if(CLANG_TIDY_EXE)
		set_target_properties(${BUILD_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
        endif()
endif()

# Set errors if debug
if(DEBUG)
	if(MSVC)
		target_compile_options(${BUILD_NAME} PRIVATE /W4 /WX)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
	endif()
endif()

# do LTO
if(LTOSupported AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	if(NOT DEBUG)
		message(STATUS "IPO / LTO enabled")
		set_property(TARGET ${BUILD_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
else()
	message(STATUS "IPO / LTO not supported: <${error}>")
endif()
