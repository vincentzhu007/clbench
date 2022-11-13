include(FetchContent)

FetchContent_Declare(
    freeimage_download
    GIT_REPOSITORY https://github.com/WinMerge/freeimage.git
    GIT_TAG        53cc1a4efa6989552922c69c22a6e9f94f084736 # newest commit, This is a cloned repo, no tag or release.
)

FetchContent_MakeAvailable(freeimage_download)

set(FREE_IMAGE_SRC_DIR ${CMAKE_BINARY_DIR}/_deps/freeimage-src/)
set(FREE_IMAGE_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/freeimage-build/output)
set(FREE_IMAGE_INSTALL_HEADER_DIR ${FREE_IMAGE_INSTALL_DIR}/usr/include/)
set(FREE_IMAGE_INSTALL_LIB_DIR ${FREE_IMAGE_INSTALL_DIR}/usr/lib/)
set(FREE_IMAGE_LIB ${FREE_IMAGE_INSTALL_LIB_DIR}/libfreeimage.so)

add_custom_target(freeimage_build ALL
        COMMAND sed -i "s/-o root -g root//g" ${FREE_IMAGE_SRC_DIR}/Makefile.gnu # fix permission error of 'chown to root'
        COMMAND mkdir -p ${FREE_IMAGE_INSTALL_DIR}
        COMMAND ${CMAKE_MAKE_PROGRAM} -j 4
        COMMAND ${CMAKE_MAKE_PROGRAM} install DESTDIR=${FREE_IMAGE_INSTALL_DIR}
        WORKING_DIRECTORY ${FREE_IMAGE_SRC_DIR}
        COMMENT "making freeimage...")

add_library(freeimage SHARED IMPORTED)
set_property(TARGET freeimage APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(freeimage PROPERTIES IMPORTED_LOCATION_NOCONFIG "${FREE_IMAGE_LIB}")
target_include_directories(freeimage INTERFACE ${FREE_IMAGE_INSTALL_HEADER_DIR})
add_dependencies(freeimage freeimage_build)