# How to Install and Build KLOGG with Cursor IDE on Windows

This guide provides step-by-step instructions for building KLOGG (advanced log viewer) from source using Cursor IDE on a fresh Windows PC.

## 📋 Prerequisites

### System Requirements
- **OS**: Windows 10/11 (64-bit)
- **RAM**: Minimum 8GB (16GB recommended)
- **Storage**: At least 10GB free space
- **Internet**: Required for downloading dependencies

## 🛠️ Required Software Installation

### 1. Visual Studio 2022 Community
- **Download**: [Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/)
- **Required Workloads**:
  - Desktop development with C++
  - MSVC v143 compiler toolset
  - Windows 11 SDK (latest version)
- **Installation**: Run installer and select the C++ workload

### 2. CMake (Version 3.30 or higher)
- **Download**: [CMake 3.30+](https://cmake.org/download/)
- **Version Used**: CMake 4.1.0 (but 3.30+ works)
- **Installation**: 
  - Download Windows x64 Installer
  - During installation, select "Add CMake to system PATH"
- **Verify**: Open PowerShell and run `cmake --version`

### 3. Qt 6.9.1 with MSVC 2022
- **Download**: [Qt Online Installer](https://www.qt.io/download-qt-installer)
- **Installation Path**: `C:\Qt\6.9.1\msvc2022_64\`
- **Required Components**:
  - Qt 6.9.1
  - MSVC 2022 64-bit
  - Qt Creator (optional)
  - **Essential Modules**:
    - Qt6Core
    - Qt6Gui
    - Qt6Widgets
    - Qt6Network
    - Qt6Xml
    - Qt6Core5Compat ⚠️ **CRITICAL**
    - Qt6WebView (if available)
    - Qt6Concurrent
- **Environment**: Add `C:\Qt\6.9.1\msvc2022_64\bin` to PATH

### 4. Boost Libraries (Version 1.57+)
- **Download**: [Boost 1.88.0](https://www.boost.org/users/download/)
- **Installation Path**: `C:\boost\`
- **Setup Options**:
  - **Option A (Recommended)**: Download pre-built binaries for MSVC 2022
  - **Option B**: Build from source using `bootstrap.bat` and `b2.exe`
- **Structure**: Ensure `C:\boost\include\boost\` contains header files

### 5. Cursor IDE
- **Download**: [Cursor IDE](https://cursor.sh/)
- **Installation**: Standard installation
- **Extensions**: CMake Tools extension (usually pre-installed)

## 📁 Project Setup

### 1. Clone KLOGG Repository
```powershell
# Create project directory
mkdir C:\P\proj\TotalSearch
cd C:\P\proj\TotalSearch

# Clone KLOGG repository
git clone https://github.com/variar/klogg.git KLOGG
cd KLOGG
```

### 2. Create Build Directory
```powershell
mkdir build_cursor
cd build_cursor
```

## 🔧 Critical CMake Fixes

### 1. Fix Root CMakeLists.txt
Edit `KLOGG/CMakeLists.txt`:

**Original (around line 1-8):**
```cmake
cmake_minimum_required(VERSION 3.12)

project(
  klogg
  VERSION 24.11.0
  DESCRIPTION "klogg log viewer"
  LANGUAGES C CXX ASM
)
```

**Fixed:**
```cmake
cmake_minimum_required(VERSION 3.5...4.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(
  klogg
  VERSION 24.11.0
  DESCRIPTION "klogg log viewer"
  LANGUAGES C CXX
)
```

### 2. Fix cmake/ucm.cmake
Edit `KLOGG/cmake/ucm.cmake` (around line 12):

**Original:**
```cmake
cmake_minimum_required(VERSION 3.12)
```

**Fixed:**
```cmake
cmake_minimum_required(VERSION 3.5...4.1)
```

## ⚙️ Cursor IDE Configuration

### 1. Open Project in Cursor
- Launch Cursor IDE
- Open the `KLOGG` folder

### 2. Configure CMake Tools
1. Press `Ctrl+,` to open Settings
2. Search for "cmake"
3. Find "CMake Tools: Configure Args"
4. Add these configure arguments:
```
-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/msvc2022_64/lib/cmake;-DBOOST_ROOT=C:/boost
```

### 3. Set CMake Generator
- In CMake Tools settings, set:
  - **Generator**: Visual Studio 17 2022
  - **Architecture**: x64

## 🚀 Build Process

### 1. Configure CMake (Command Line Method)
```powershell
# Navigate to build directory
cd C:\P\proj\TotalSearch\KLOGG\build_cursor

# Configure with CMake
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.9.1/msvc2022_64/lib/cmake" -DBOOST_ROOT="C:/boost" ..
```

### 2. Build the Project
```powershell
# Build in Release mode (recommended)
cmake --build . --config Release

# Alternative: Build in Debug mode
cmake --build . --config Debug
```

### 3. Deploy Qt Dependencies
```powershell
# Navigate to output directory
cd output/Release

# Deploy Qt DLLs (CRITICAL STEP)
C:\Qt\6.9.1\msvc2022_64\bin\windeployqt.exe --release klogg.exe
```

## 🎯 Running KLOGG

### 1. Navigate to Executable
```powershell
cd C:\P\proj\TotalSearch\KLOGG\build_cursor\output\Release
```

### 2. Run KLOGG
```powershell
# Start KLOGG GUI
.\klogg.exe

# Or with a specific file
.\klogg.exe "C:\path\to\your\logfile.txt"

# Or start in background
Start-Process .\klogg.exe
```

## 🔍 Troubleshooting

### Common Issues and Solutions

#### 1. "Qt6Core5Compat.dll not found"
**Solution**: Run `windeployqt` as shown in step 3 above.

#### 2. "CMAKE_POLICY_VERSION_MINIMUM value '3'"
**Solution**: Apply the CMakeLists.txt fixes from the "Critical CMake Fixes" section.

#### 3. "No CMAKE_ASM_COMPILER could be found"
**Solution**: Remove `ASM` from the `LANGUAGES` list in CMakeLists.txt (already done in the fix above).

#### 4. CMake can't find Qt6 or Boost
**Solution**: 
- Verify installation paths match exactly: `C:\Qt\6.9.1\msvc2022_64\` and `C:\boost\`
- Check that Qt6Core5Compat is installed via Qt Maintenance Tool

#### 5. Build fails with "LNK" errors
**Solution**: 
- Ensure Visual Studio 2022 with C++ workload is installed
- Try cleaning and rebuilding: `cmake --build . --config Release --clean-first`

### Verification Steps
```powershell
# Check if KLOGG is running
Get-Process klogg -ErrorAction SilentlyContinue

# Verify Qt DLLs are present
dir *.dll | findstr Qt6

# Check CMake version
cmake --version

# Verify Qt installation
dir "C:\Qt\6.9.1\msvc2022_64\bin\windeployqt.exe"
```

## 📦 Expected Output Files

After successful build, you should have:
- `klogg.exe` (9.45 MB) - Main application
- `klogg_grep.exe` (7.98 MB) - Command-line utility
- `klogg_portable.exe` (9.45 MB) - Portable version
- Various Qt DLLs (Qt6Core.dll, Qt6Core5Compat.dll, etc.)
- Plugin directories (platforms/, imageformats/, etc.)

## 🎓 Key Lessons Learned

1. **Qt6Core5Compat is essential** - Missing this DLL causes runtime failures
2. **CMake 4.1+ requires policy version fixes** - Older projects need updates
3. **ASM compiler issues** - Remove ASM language requirement on Windows
4. **windeployqt is mandatory** - Qt applications need runtime deployment
5. **Exact paths matter** - Use the specified installation paths for consistency

## 📝 Version Summary

- **CMake**: 3.30+ (tested with 4.1.0)
- **Qt**: 6.9.1 with MSVC 2022 64-bit
- **Boost**: 1.88.0 (minimum 1.57+)
- **Visual Studio**: 2022 Community with C++ workload
- **Windows**: 10/11 64-bit
- **KLOGG**: Latest from GitHub (master branch)

## 🔄 For Clean Rebuilds

```powershell
# Clean build directory
cd C:\P\proj\TotalSearch\KLOGG\build_cursor
Remove-Item * -Recurse -Force

# Reconfigure and rebuild
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.9.1/msvc2022_64/lib/cmake" -DBOOST_ROOT="C:/boost" ..
cmake --build . --config Release
cd output/Release
C:\Qt\6.9.1\msvc2022_64\bin\windeployqt.exe --release klogg.exe
```

---

**Success Indicator**: When KLOGG starts without errors and you see the log viewer interface, you've successfully built KLOGG from source! 🎉 