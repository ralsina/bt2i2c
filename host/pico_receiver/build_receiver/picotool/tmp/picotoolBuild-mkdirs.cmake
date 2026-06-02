# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/_deps/picotool-src")
  file(MAKE_DIRECTORY "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/_deps/picotool-src")
endif()
file(MAKE_DIRECTORY
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/_deps/picotool-build"
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/_deps"
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/tmp"
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/src/picotoolBuild-stamp"
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/src"
  "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ralsina/code/terminado/bt2i2c/host/pico_receiver/build_receiver/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
