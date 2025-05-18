cmake_minimum_required(VERSION 3.16)

project(browser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets WebEngineWidgets Svg)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/tab.h
        src/tab.cpp
        src/title_bar.cpp
        src/title_bar.h
        src/utils.h
        src/utils.cpp
        resources.qrc
        src/command_palette.cpp
        src/command_palette.h
        src/macos_titlebar.h
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
    qt_add_executable(browser
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(browser SHARED
                ${PROJECT_SOURCES}
        )
    else()
        add_executable(browser
                ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(browser PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(browser PRIVATE Qt${QT_VERSION_MAJOR}::WebEngineWidgets)
target_link_libraries(browser PRIVATE Qt${QT_VERSION_MAJOR}::Svg)

# Link macOS frameworks if building for Apple
if(APPLE)
    find_library(APPKIT AppKit)
    find_library(FOUNDATION Foundation)
    target_link_libraries(browser PRIVATE ${APPKIT} ${FOUNDATION})
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.browser)
endif()
set_target_properties(browser PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS browser
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(browser)
endif()
