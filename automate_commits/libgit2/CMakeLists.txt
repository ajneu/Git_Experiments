cmake_minimum_required (VERSION 3.2)

project(use_libgit2)


set(src     go.cpp)
set(target  go)



################
# cmake_lib_hints.txt
################
include(${PROJECT_SOURCE_DIR}/cmake_lib_hints.txt)

################
# c++14
################
set(CMAKE_CXX_STANDARD   14)
set(CMAKE_CXX_EXTENSIONS OFF)


################
# libgit2
################
# set(ENV{PKG_CONFIG_PATH} "/home/albert/tmp/libgit2/build/install/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}") # see cmake_lib_hints.txt

include(FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBGIT2 libgit2)
endif ()

if(LIBGIT2_FOUND)
  link_directories(   ${LIBGIT2_LIBRARY_DIRS})
  include_directories(${LIBGIT2_INCLUDE_DIRS})
  set(libs ${libs}    ${LIBGIT2_LIBRARIES})
else()
  message(FATAL_ERROR "libgit2 not found")
endif()

################
# boost
################
find_package(Boost COMPONENTS system filesystem)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  set(libs ${libs}    ${Boost_LIBRARIES})
else()
  message()
endif()


################
# build target
################
add_executable(       ${target} ${src})
target_link_libraries(${target} ${libs})
