cmake_minimum_required(VERSION 3.16)
project(TotalSearchWatchdog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Set up Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    main.cpp
    TotalSearchWatchdog.cpp
)

set(HEADERS
    TotalSearchWatchdog.h
)

# Create executable
add_executable(TotalSearchWatchdog ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(TotalSearchWatchdog Qt6::Core Qt6::Widgets)

# Set output directory
set_target_properties(TotalSearchWatchdog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release
)

# Windows-specific settings
if(WIN32)
    set_target_properties(TotalSearchWatchdog PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
