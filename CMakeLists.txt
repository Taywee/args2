cmake_minimum_required(VERSION 3.14)

project(
    args2
    VERSION "1.0.0"
    DESCRIPTION "An obvious and easy, idiomatic C++17 argument parsing library."
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}
    CACHE PATH "Location of header files"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    args2

        src/args2/temporary_directory.cxx
        src/args2/temporary_file.cxx
        src/args2/temporary_path.cxx
)

if (WIN32)
target_sources(
    args2

    PRIVATE
        src/args2/windows/file_handle.cxx
        src/args2/windows/tempfile_support.cxx
        src/args2/windows/env.cxx
        src/args2/windows/error.cxx
)
else()
target_sources(
    args2

    PRIVATE
        src/args2/unix/file_handle.cxx
        src/args2/unix/env.cxx
        src/args2/unix/error.cxx
)
endif()

set_property(
    TARGET args2
    PROPERTY POSITION_INDEPENDENT_CODE TRUE
)

target_compile_options(
    args2
    PRIVATE
        -Wall
        -Wextra
        -Werror
        -I${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    args2
    PRIVATE
        -D_POSIX_C_SOURCE=200809L
        -D_XOPEN_SOURCE=700
        -D_XOPEN_SOURCE_EXTENDED=1
        -D_ALL_SOURCE
)

target_link_libraries(args2 PUBLIC)

install(
    FILES
        src/args2/env.hxx
        src/args2/error.hxx
        src/args2/file_handle.hxx
        src/args2/string_convert.hxx
        src/args2/temporary_directory.hxx
        src/args2/temporary_file.hxx
        src/args2/temporary_path.hxx

    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/args2"
)

install(
    TARGETS args2
    EXPORT args2Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT args2Targets
    NAMESPACE args2::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/args2"
)

configure_package_config_file("args2Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/args2Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/args2"
    PATH_VARS INCLUDE_INSTALL_DIR
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/args2ConfigVersion.cmake"
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/args2Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/args2ConfigVersion.cmake

    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/args2"
)

