# $id$

cmake_minimum_required(VERSION 2.6)

project(GD_PANGO C)

set(CMAKE_MODULE_PATH "${GD_PANGO_SOURCE_DIR}/cmake/modules")

find_package(GD REQUIRED)
check_library_exists("${GD_LIBRARY}" "gdColorMapLookup" "${GD_LIB_PATH}" GD_2_1_FEATURE)
if(NOT GD_2_1_FEATURE)
  message(FATAL_ERROR "Could not find gd 2.1.x feature")
endif(NOT GD_2_1_FEATURE)

find_package(PkgConfig)
pkg_check_modules(PANGOFT2 REQUIRED pangoft2)

include_directories(${GD_INCLUDE_DIR})
link_directories(${GD_LIBRARY_DIR})
include_directories(${PANGOFT2_INCLUDE_DIRS})
link_directories(${PANGOFT2_LIBRARY_DIRS})

add_library(gd_pango SHARED gd_pango)
target_link_libraries(gd_pango ${PANGOFT2_LIBRARIES} ${GD_LIBRARY})

add_subdirectory(examples)
add_subdirectory(tests)
if(BUILD_TESTING)
  enable_testing()
endif(BUILD_TESTING)

set(CPACK_PACKAGE_NAME gd-pango)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_VENDOR "http://libgd.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "gd-pango, Pango support for GD")
set(CPACK_RESOURCE_FILE_LICENSE "${GD_PANGO_SOURCE_DIR}/LICENSE.NEWBSD")
set(CPACK_RESOURCE_FILE_README "${GD_PANGO_SOURCE_DIR}/README")
include(CPack)
