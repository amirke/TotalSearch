cmake_minimum_required(VERSION 3.16)
project(TotalSearch VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Build Scintilla core library first
add_subdirectory(scintilla/scintilla/src)

# Build ScintillaEditBase
add_subdirectory(scintilla/scintilla/qt/ScintillaEditBase)

# Source files for TotalSearch application
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/scintillaedit.cpp
    src/preferencesdialog.cpp
    src/logger.cpp
    src/filelinemodel.cpp
    src/utils.cpp
    src/filesearcher.cpp
    src/searchresults.cpp
    src/logviewer.cpp
    src/fileviewer.cpp
    src/highlighter.cpp
    src/filewatcher.cpp
    src/searchwidget.cpp
    src/LogDataWorker.cpp
    src/LogMainView.cpp
    src/searchdialog.cpp
    src/KSearchBun.cpp
    src/rgsearchdialog.cpp
    src/clocktestdialog.cpp
    src/CollapsibleSearchResults.cpp
    src/highlightdialog.cpp
    src/engineeringdialog.cpp
    src/configurationdialog.cpp
    src/JsonParseWorker.cpp
    src/KSearch.cpp
    src/DetachablePane.cpp
)

set(HEADERS
    src/mainwindow.h
    src/scintillaedit.h
    src/preferencesdialog.h
    src/logger.h
    src/filelinemodel.h
    src/utils.h
    src/filesearcher.h
    src/searchresults.h
    src/logviewer.h
    src/fileviewer.h
    src/highlighter.h
    src/filewatcher.h
    src/searchwidget.h
    src/LogDataWorker.h
    src/LogMainView.h
    src/searchdialog.h
    src/KSearchBun.h
    src/rgsearchdialog.h
    src/clocktestdialog.h
    src/CollapsibleSearchResults.h
    src/highlightdialog.h
    src/engineeringdialog.h
    src/configurationdialog.h
    src/JsonParseWorker.h
    src/KSearch.h
    src/DetachablePane.h
)

# UI files
set(UI_FILES
    src/mainwindow.ui
    src/searchwidget.ui
    src/fileviewer.ui
    src/logviewer.ui
    src/searchresults.ui
)

# Resource files
set(RESOURCE_FILES
    icons/icons.qrc
)

# Create executable
add_executable(TotalSearch ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCE_FILES})

# Create watchdog executable
add_executable(TotalSearchWatchdog
    watchdog/main.cpp
    watchdog/TotalSearchWatchdog.cpp
    watchdog/TotalSearchWatchdog.h
)

# Include directories
target_include_directories(TotalSearch PRIVATE 
    src 
    include
    scintilla/scintilla/include
    scintilla/scintilla/src
    scintilla/scintilla/qt/ScintillaEditBase
)

# Link Qt libraries and ScintillaEditBase
target_link_libraries(TotalSearch 
    Qt6::Core 
    Qt6::Widgets
    ScintillaEditBase
)

# Link Qt libraries for watchdog
target_link_libraries(TotalSearchWatchdog 
    Qt6::Core 
    Qt6::Widgets
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(TotalSearch
        comctl32
        shlwapi
        user32
        gdi32
        shell32
        advapi32
    )
    
    # Set Windows subsystem to GUI (no console window)
    set_target_properties(TotalSearch PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Set Windows subsystem to GUI for watchdog
    set_target_properties(TotalSearchWatchdog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Compiler-specific settings
if(MSVC)
    target_compile_options(TotalSearch PRIVATE /EHsc /W3 /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
else()
    target_compile_options(TotalSearch PRIVATE -Wall -Wextra -O2)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output) 