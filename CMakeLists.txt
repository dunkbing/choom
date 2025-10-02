cmake_minimum_required(VERSION 3.16)

project(dbclient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set Qt path - update this to match your Qt installation
if(NOT DEFINED CMAKE_PREFIX_PATH)
    if(APPLE)
        set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.2/macos")
    elseif(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/msvc2019_64")
    else()
        set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.9.2/gcc_64")
    endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Concurrent Svg Quick QuickWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Concurrent Svg Quick QuickWidgets)

# Collect all header files automatically
file(GLOB_RECURSE PROJECT_HEADERS "include/*.h")

set(PROJECT_SOURCES
        src/main.cpp

        # UI
        src/ui/mainwindow.cpp
        src/ui/title_bar.cpp
        src/ui/connection_tree_model.cpp
        src/ui/table_viewer.cpp
        src/ui/sql_editor.cpp
        src/ui/welcome_widget.cpp
        src/ui/connection_dialog.cpp
        src/ui/spinner_icon.cpp

        # Database
        src/database/database_connection.cpp
        src/database/sqlite_connection.cpp
        src/database/mysql_connection.cpp
        src/database/postgres_connection.cpp
        src/database/connection_manager.cpp

        # Core
        src/core/query_executor.cpp
        src/core/utils.cpp
        src/core/connection_storage.cpp

        # Resources
        resources.qrc

        # Headers (automatically collected)
        ${PROJECT_HEADERS}
)

# Add macOS-specific files
if(APPLE)
    list(APPEND PROJECT_SOURCES src/macos_titlebar.mm)

    # Enable Objective-C++ support
    set(CMAKE_OBJCXX_STANDARD 11)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

    # Add Objective-C++ flags
    set(OBJCXX_FLAGS "-fobjc-arc")
    set(CMAKE_OBJCXX_FLAGS "${CMAKE_OBJCXX_FLAGS} ${OBJCXX_FLAGS}")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dbclient
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(dbclient SHARED
                ${PROJECT_SOURCES}
        )
    else()
        add_executable(dbclient
                ${PROJECT_SOURCES}
        )
    endif()
endif()

# Add include directories
target_include_directories(dbclient PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/database
    ${CMAKE_SOURCE_DIR}/include/ui
    ${CMAKE_SOURCE_DIR}/include/core
)

target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::Svg)
target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::Quick)
target_link_libraries(dbclient PRIVATE Qt${QT_VERSION_MAJOR}::QuickWidgets)

# Link macOS frameworks if building for Apple
if(APPLE)
    find_library(APPKIT AppKit)
    find_library(FOUNDATION Foundation)
    target_link_libraries(dbclient PRIVATE ${APPKIT} ${FOUNDATION})
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.dbclient)
endif()
set_target_properties(dbclient PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS dbclient
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dbclient)
endif()
