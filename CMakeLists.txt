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
option(ARGS2_TESTS OFF)

add_library(
    args2
    INTERFACE
)

set_property(
    TARGET args2
    PROPERTY POSITION_INDEPENDENT_CODE TRUE
)


install(
    FILES
        src/args2/args2.hxx
        src/args2/parser.hxx

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

if (${ARGS2_TESTS})
    find_package(Catch2 3 REQUIRED)
    add_executable(
        tests
            test/test.cxx
            test/parser.cxx
    )
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain args2)
    target_include_directories(tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    include (CTest)
    add_test(
        NAME tests
        COMMAND tests
    )
endif()



