cmake_minimum_required(VERSION 3.16)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ScintillaEditBase source files
set(SCINTILLA_EDIT_BASE_SOURCES
    ScintillaEditBase.cpp
    ScintillaQt.cpp
    PlatQt.cpp
)

set(SCINTILLA_EDIT_BASE_HEADERS
    ScintillaEditBase.h
    ScintillaQt.h
    PlatQt.h
)

# Create ScintillaEditBase library
add_library(ScintillaEditBase STATIC ${SCINTILLA_EDIT_BASE_SOURCES} ${SCINTILLA_EDIT_BASE_HEADERS})

# Include directories
target_include_directories(ScintillaEditBase PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Link Qt libraries and Scintilla core
target_link_libraries(ScintillaEditBase 
    Qt6::Core 
    Qt6::Widgets
    Scintilla
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(ScintillaEditBase PRIVATE /EHsc /W3 /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    add_definitions(-DMAKING_LIBRARY)
else()
    target_compile_options(ScintillaEditBase PRIVATE -Wall -Wextra -O2)
    add_definitions(-DMAKING_LIBRARY)
endif()

# Set output directory
set_target_properties(ScintillaEditBase PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 