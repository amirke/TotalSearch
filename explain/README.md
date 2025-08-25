# TotalSearch

A powerful C++/Qt desktop application for advanced file search and log viewing, inspired by [klogg](https://github.com/variar/klogg) and expanded with comprehensive file search capabilities.

## Features

### 🔍 Advanced File Search
- **Multi-file search**: Search across entire directories and subdirectories
- **Regex support**: Full regular expression search capabilities using Qt's QRegularExpression
- **File type filtering**: Search specific file types (txt, log, md, json, js, ts, py, java, cpp, etc.)
- **Case-sensitive options**: Toggle case sensitivity for searches
- **Real-time results**: View search results with line numbers and context
- **Background search**: Non-blocking search operations with progress reporting

### 📁 File Viewer
- **Syntax highlighting**: View files with proper formatting and syntax highlighting
- **Line navigation**: Jump to specific line numbers with "Go to Line" functionality
- **Find in file**: Search within the currently open file with highlighting
- **File information**: Display file size, line count, and path details
- **Zoom controls**: Zoom in/out and reset zoom functionality

### 📊 Log Viewer
- **Real-time monitoring**: Watch log files for changes using QFileSystemWatcher
- **Auto-scroll**: Automatically scroll to new log entries
- **Log levels**: Color-coded log levels (error, warn, info, debug)
- **Timestamp display**: Show timestamps for log entries
- **Log filtering**: Filter log entries by level and content

### 🎯 Key Features
- **Modern Qt6 UI**: Clean, responsive interface with native look and feel
- **Cross-platform**: Works on Windows, macOS, and Linux
- **Performance**: Optimized for large files and directories with multi-threading
- **Keyboard shortcuts**: Full keyboard navigation support
- **File watching**: Monitor files for real-time updates
- **Dockable interface**: Customizable layout with dockable widgets

## Building

### Prerequisites
- **Qt6**: Qt6 Core, Widgets, and Gui modules
- **CMake**: Version 3.16 or higher
- **C++17**: Compatible compiler (GCC, Clang, MSVC)
- **Build tools**: Make, Ninja, or Visual Studio

### Dependencies
- Qt6 Core
- Qt6 Widgets  
- Qt6 Gui
- Qt6 Concurrent (for background operations)

### Build Instructions

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd TotalSearch
   ```

2. **Create build directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake ..
   ```

4. **Build the project**
   ```bash
   cmake --build .
   ```

5. **Run the application**
   ```bash
   ./TotalSearch  # Linux/macOS
   # or
   TotalSearch.exe  # Windows
   ```

### Platform-Specific Notes

#### Windows
- Use Visual Studio 2019/2022 or MinGW-w64
- Qt6 can be installed via Qt installer or vcpkg
- Build with: `cmake --build . --config Release`

#### macOS
- Install Qt6 via Homebrew: `brew install qt6`
- Build with: `cmake --build . --config Release`

#### Linux
- Install Qt6 via package manager or Qt installer
- Build with: `cmake --build . --config Release`

## Usage

### Command Line Interface
```bash
# Open a file
./TotalSearch -f /path/to/file.txt

# Open a directory for searching
./TotalSearch -d /path/to/directory

# Search with pattern
./TotalSearch -s "search pattern"

# Combine options
./TotalSearch -d /path/to/directory -s "error"
```

### GUI Interface

#### Opening Files and Folders
1. **Open a single file**: Use `Ctrl+O` or File → Open File
2. **Open a folder for search**: Use `Ctrl+Shift+O` or File → Open Directory

#### Searching Files
1. **Enter search pattern**: Type in the search widget
2. **Configure options**: Set case sensitivity, regex, file types
3. **Perform search**: Click "Search Files" or press Enter
4. **View results**: Results appear in the Search Results tab

#### File Viewer Features
- **Navigate to line**: Use "Go to Line" button or `Ctrl+G`
- **Find in file**: Use `Ctrl+F` to search within current file
- **Zoom controls**: Use `Ctrl++`, `Ctrl+-`, `Ctrl+0` for zoom

#### Log Monitoring
1. **Open a log file**: Use the file viewer
2. **Start watching**: Click "Start Watching" button
3. **View real-time updates**: New entries appear in Log Viewer tab

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+O` | Open File |
| `Ctrl+Shift+O` | Open Directory |
| `Ctrl+S` | Save File |
| `Ctrl+F` | Find in File |
| `Ctrl+G` | Go to Line |
| `Ctrl++` | Zoom In |
| `Ctrl+-` | Zoom Out |
| `Ctrl+0` | Reset Zoom |
| `F3` | Find Next |
| `Shift+F3` | Find Previous |
| `Ctrl+Q` | Quit Application |

## Architecture

### Core Components
- **MainWindow**: Main application window with menu, toolbar, and status bar
- **SearchWidget**: Search interface with pattern input and options
- **FileViewer**: Text editor component for viewing files
- **LogViewer**: Real-time log monitoring component
- **SearchResults**: Display and manage search results
- **FileSearcher**: Background file search engine
- **FileWatcher**: File system monitoring

### Key Technologies
- **Qt6**: Cross-platform application framework
- **CMake**: Build system and project configuration
- **C++17**: Modern C++ features for performance
- **QRegularExpression**: Advanced regex support
- **QFileSystemWatcher**: File monitoring
- **QtConcurrent**: Background processing

## Project Structure
```
TotalSearch/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── mainwindow.h/cpp      # Main window implementation
│   ├── searchwidget.h/cpp    # Search interface
│   ├── fileviewer.h/cpp      # File viewing component
│   ├── logviewer.h/cpp       # Log monitoring component
│   ├── searchresults.h/cpp   # Search results display
│   ├── filesearcher.h/cpp    # File search engine
│   ├── filewatcher.h/cpp     # File system watcher
│   ├── highlighter.h/cpp     # Syntax highlighting
│   ├── utils.h/cpp           # Utility functions
│   └── ui/                   # Qt Designer UI files
├── resources/                # Application resources
├── CMakeLists.txt           # Build configuration
└── README.md               # This file
```

## Development

### Adding Features
1. **New file types**: Add to `FileTypes` namespace in `utils.h`
2. **Search algorithms**: Modify `FileSearcher` class
3. **UI components**: Add new widgets and connect signals/slots
4. **Syntax highlighting**: Extend `Highlighter` class

### Code Style
- Follow Qt coding conventions
- Use Qt's signal/slot mechanism
- Prefer Qt containers over STL when appropriate
- Use RAII and smart pointers
- Document public interfaces

### Testing
The application can be tested by:
1. Opening various file types
2. Searching through code repositories
3. Monitoring log files
4. Testing with large files and directories

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes following Qt/C++ best practices
4. Test thoroughly on multiple platforms
5. Submit a pull request

## License

MIT License - see LICENSE file for details.

## Acknowledgments

- Inspired by [klogg](https://github.com/variar/klogg) - a fast log explorer
- Built with Qt6 for cross-platform compatibility
- Uses modern C++17 features for performance

## Support

For issues and feature requests, please create an issue in the repository.

---

**TotalSearch** - Advanced file search and log viewing made simple with C++ and Qt6. 