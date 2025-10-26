# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/melatonin_inspector")
  file(MAKE_DIRECTORY "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/melatonin_inspector")
endif()
file(MAKE_DIRECTORY
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-build"
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix"
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/tmp"
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/src/melatonin_inspector-populate-stamp"
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/src"
  "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/src/melatonin_inspector-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/src/melatonin_inspector-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/brackenasay/Documents/School_Files_U_of_U/CS_4000/groovekit/cmake-build-mac11_universal/_deps/melatonin_inspector-subbuild/melatonin_inspector-populate-prefix/src/melatonin_inspector-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
