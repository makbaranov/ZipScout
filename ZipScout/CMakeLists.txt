cmake_minimum_required(VERSION 3.16)

project(ZipScout VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Concurrent )
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)

include(FetchContent)
set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "Qt major version")
set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "Build QuaZip tests")
FetchContent_Declare(
    QuaZip
    GIT_REPOSITORY https://github.com/stachenov/quazip.git
    GIT_TAG v1.4
)
FetchContent_MakeAvailable(QuaZip)

# FetchContent_Declare(
#     zeromq
#     GIT_REPOSITORY https://github.com/zeromq/libzmq.git
#     GIT_TAG        v4.3.5
# )
# FetchContent_MakeAvailable(zeromq)
# if(NOT TARGET libzmq)
#     message(FATAL_ERROR "ZeroMQ not available!")
# endif()

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
if(NOT googletest_POPULATED)
    message(FATAL_ERROR "GTest not available!")
endif()

qt_add_executable(ZipScout
    MANUAL_FINALIZATION
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ZipWordSearcher.h
    ZipWordSearcher.cpp
    ZipArchiveCreator.h
    ZipArchiveCreator.cpp
)

target_link_libraries(ZipScout PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    QuaZip::QuaZip
    libzmq
    cppzmq
)

if(WIN32)
    set_target_properties(ZipScout PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

include(GNUInstallDirs)
install(TARGETS ZipScout
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(ZipScout)

add_executable(ZipScoutWorker worker.cpp)
target_link_libraries(ZipScoutWorker PRIVATE
    Qt6::Core
    libzmq
    cppzmq
)

add_executable(ZipScoutTests tests/test_main.cpp)
target_link_libraries(ZipScoutTests PRIVATE
    Qt6::Core
    gtest_main
)
