cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(QGeoView LANGUAGES C CXX)

option(BUILD_EXAMPLES "Build examples" ON)

find_package(GDAL CONFIG)

add_subdirectory(lib)

if (${BUILD_EXAMPLES})
  message(STATUS "Enabled building of examples")
  add_subdirectory(samples/shared)
  add_subdirectory(samples/raster)
  add_subdirectory(samples/Test)

  if(GDAL_FOUND)
    add_subdirectory(samples/gdal-shapefile)
  endif()
else ()
  message(STATUS "Disabled building of examples")
endif ()
