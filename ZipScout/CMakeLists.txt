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

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent Core5Compat)
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
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=ON
        -DINSTALL_GTEST=OFF
        -Dgtest_force_shared_crt=ON
)
FetchContent_MakeAvailable(googletest)

qt_add_executable(ZipScout
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    WorkerManager.h
    WorkerManager.cpp
    FoundFilesModel.h
    FoundFilesModel.cpp
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

qt_add_executable(ZipScoutTests
    tests/test_main.cpp
    WorkerManager.h
    WorkerManager.cpp
    ZipWordSearcher.h
    ZipWordSearcher.cpp
    ZipArchiveCreator.h
    ZipArchiveCreator.cpp
)

target_link_libraries(ZipScoutTests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    QuaZip::QuaZip
    ZLIB::ZLIB
    libzmq
    cppzmq
    gtest_main
)

if(WIN32)
    set_target_properties(ZipScout ZipScoutWorker PROPERTIES WIN32_EXECUTABLE TRUE)

    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ZipScout POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --no-translations
                "$<TARGET_FILE_DIR:ZipScout>"
            COMMENT "Deploying Qt dependencies for ZipScout"
        )
    else()
        message(WARNING "windeployqt not found - Qt dependencies won't be deployed")
    endif()

    add_custom_command(TARGET ZipScout POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::Core5Compat>"
            "$<TARGET_FILE_DIR:ZipScout>"
        COMMENT "Copying Qt6Core5Compat.dll"
    )

    add_custom_command(TARGET ZipScout POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:QuaZip::QuaZip>"
            "$<TARGET_FILE_DIR:ZipScout>"
        COMMENT "Copying QuaZip DLL"
    )

    add_custom_command(TARGET ZipScout POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/vcpkg/installed/x64-windows/bin/zlib1.dll"
            "$<TARGET_FILE_DIR:ZipScout>"
        COMMENT "Copying ZLIB DLL"
    )

    add_custom_command(TARGET ZipScout POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/vcpkg/installed/x64-windows/bin/libzmq-mt-4_3_5.dll"
            "$<TARGET_FILE_DIR:ZipScout>"
    )

    add_custom_command(TARGET ZipScoutTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:gtest>"
            "$<TARGET_FILE_DIR:ZipScoutTests>"
    )
endif()

include(GNUInstallDirs)
install(TARGETS ZipScout ZipScoutWorker
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_finalize_executable(ZipScout)
qt_finalize_executable(ZipScoutWorker)
