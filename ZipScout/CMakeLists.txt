cmake_minimum_required(VERSION 3.16)
project(ZipScout VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent)
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
find_package(ZLIB REQUIRED)

include(FetchContent)

set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "Qt major version")
set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "Build QuaZip tests")
FetchContent_Declare(
    QuaZip
    GIT_REPOSITORY https://github.com/stachenov/quazip.git
    GIT_TAG v1.4
)
FetchContent_MakeAvailable(QuaZip)

FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

qt_add_executable(ZipScout
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    WorkerManager.h
    WorkerManager.cpp
)

target_link_libraries(ZipScout PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    libzmq
    cppzmq
)

qt_add_executable(ZipScoutWorker
    worker.cpp
    ZipWordSearcher.h
    ZipWordSearcher.cpp
    ZipArchiveCreator.h
    ZipArchiveCreator.cpp
)

target_link_libraries(ZipScoutWorker PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    QuaZip::QuaZip
    ZLIB::ZLIB
    libzmq
    cppzmq
)

add_executable(ZipScoutTests
    tests/test_main.cpp
)

target_link_libraries(ZipScoutTests PRIVATE
    Qt6::Core
    gtest_main
)

if(WIN32)
    set_target_properties(ZipScout ZipScoutWorker PROPERTIES WIN32_EXECUTABLE TRUE)
    add_custom_command(TARGET ZipScoutWorker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:QuaZip::QuaZip>"
            "$<TARGET_FILE_DIR:ZipScoutWorker>"
        COMMENT "Copying QuaZip DLL"
    )

    add_custom_command(TARGET ZipScoutWorker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/vcpkg/installed/x64-windows/bin/zlib1.dll"
            "$<TARGET_FILE_DIR:ZipScoutWorker>"
        COMMENT "Copying ZLIB DLL"
    )
endif()

include(GNUInstallDirs)
install(TARGETS ZipScout ZipScoutWorker
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_finalize_executable(ZipScout)
qt_finalize_executable(ZipScoutWorker)
