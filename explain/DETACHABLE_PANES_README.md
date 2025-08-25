# TotalSearch - Detachable Panes Feature

## Overview

TotalSearch now supports detachable panes, allowing users to separate the Search Results and File Viewer panes into independent windows for better workflow management.

## Features

### 1. Detachable Panes
- **Search Results Pane**: Can be detached to show search results in a separate window
- **File Viewer Pane**: Can be detached to show file content in a separate window
- **Window Management**: Each detached pane becomes a full-featured window with minimize, maximize, and close buttons
- **State Persistence**: Window positions, sizes, and states are saved and restored between sessions

### 2. Icon System
- **Application Icons**: Complete icon set for the application and taskbar
- **Pane Control Icons**: Icons for detach, attach, maximize, and close operations
- **Resource Compilation**: All icons are compiled into the application for easy distribution

## Usage

### Detaching a Pane
1. Look for the detach button (→) in the title bar of either pane
2. Click the detach button to open the pane in a separate window
3. The detached window can be moved, resized, and positioned independently

### Attaching a Pane
1. In the detached window, click the attach button (←) in the title bar
2. The pane will return to its original position in the main window

### Window Controls
- **Maximize**: Click the maximize button (□) to maximize the detached window
- **Close**: Click the close button (×) to close the detached window
- **Minimize**: Use the standard window minimize button

## Technical Implementation

### DetachablePane Class
The `DetachablePane` class provides the core functionality:
- **Title Bar**: Custom title bar with control buttons
- **Window Management**: Handles window flags, positioning, and state
- **State Persistence**: Saves/restores window state using QSettings
- **Signal System**: Emits signals for pane events (detached, attached, closed)

### Integration
- **Search Results**: Wraps the `CollapsibleSearchResults` widget
- **File Viewer**: Wraps the `ScintillaEdit` widget with filename label
- **Main Window**: Handles pane events and updates status messages

### Icon System
- **Resource File**: `icons/icons.qrc` compiles all icons into the application
- **Icon Types**: ICO files for application icons, PNG files for UI icons
- **Placeholder Icons**: Simple generated icons for development (replace with proper designs for production)

## Configuration

### Window State Storage
Window states are stored in `app.ini` under the `[DetachedPanes]` section:
```ini
[DetachedPanes]
[Search Results]
position=@Point(100, 100)
size=@Size(800, 400)
maximized=false

[File Viewer]
position=@Point(200, 200)
size=@Size(1000, 600)
maximized=false
```

### Default Sizes
- **Search Results Pane**: 800x400 pixels
- **File Viewer Pane**: 1000x600 pixels

## Files Added/Modified

### New Files
- `src/DetachablePane.h` - Header for detachable pane functionality
- `src/DetachablePane.cpp` - Implementation of detachable pane
- `icons/icons.qrc` - Qt resource file for icons
- `create_icons.py` - Python script to generate placeholder icons
- `create_icons.bat` - Batch script to create icon placeholders

### Modified Files
- `src/mainwindow.h` - Added detachable pane members and slots
- `src/mainwindow.cpp` - Integrated detachable panes into UI
- `src/main.cpp` - Set application icon
- `CMakeLists.txt` - Added resource files and new source files

### Icon Files
- `icons/app_icon.ico` - Main application icon
- `icons/taskbar_icon.ico` - Taskbar icon
- `icons/detach_pane.png` - Detach button icon
- `icons/attach_pane.png` - Attach button icon
- `icons/search_results.png` - Search results pane icon
- `icons/file_viewer.png` - File viewer pane icon
- `icons/close_window.png` - Close window icon
- `icons/maximize_window.png` - Maximize window icon

## Future Enhancements

1. **Additional Panes**: Support for detaching other UI elements
2. **Docking System**: Advanced docking and undocking capabilities
3. **Custom Icons**: Replace placeholder icons with professionally designed icons
4. **Layout Presets**: Save and restore different pane layouts
5. **Multi-Monitor Support**: Enhanced multi-monitor window management

## Notes

- The current icons are placeholders generated for development
- For production use, replace the placeholder icons with properly designed icons
- Window states are automatically saved when panes are moved or resized
- Detached panes maintain all their original functionality
