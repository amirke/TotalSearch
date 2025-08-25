cmake_minimum_required(VERSION 3.16)

# Scintilla core source files
set(SCINTILLA_CORE_SOURCES
    AutoComplete.cxx
    CallTip.cxx
    CaseConvert.cxx
    CaseFolder.cxx
    CellBuffer.cxx
    ChangeHistory.cxx
    CharacterCategoryMap.cxx
    CharacterType.cxx
    CharClassify.cxx
    ContractionState.cxx
    DBCS.cxx
    Decoration.cxx
    Document.cxx
    EditModel.cxx
    Editor.cxx
    EditView.cxx
    Geometry.cxx
    Indicator.cxx
    KeyMap.cxx
    LineMarker.cxx
    MarginView.cxx
    PerLine.cxx
    PositionCache.cxx
    RESearch.cxx
    RunStyles.cxx
    ScintillaBase.cxx
    Selection.cxx
    Style.cxx
    UniConversion.cxx
    UniqueString.cxx
    ViewStyle.cxx
    XPM.cxx
)

# Create Scintilla core library
add_library(Scintilla STATIC ${SCINTILLA_CORE_SOURCES})

# Include directories
target_include_directories(Scintilla PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(Scintilla PRIVATE /EHsc /W3 /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
else()
    target_compile_options(Scintilla PRIVATE -Wall -Wextra -O2)
endif()

# Set output directory
set_target_properties(Scintilla PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 