cmake_minimum_required(VERSION 3.16)

project(ZipScout VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

include(FetchContent)
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
)

target_link_libraries(ZipScout PRIVATE
    Qt6::Widgets
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

add_executable(ZipScoutTests tests/test_main.cpp)
target_link_libraries(ZipScoutTests PRIVATE
    Qt6::Core
    gtest_main
)
