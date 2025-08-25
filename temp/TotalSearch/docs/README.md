# TotalSearch v1.0.0

Professional file search application with advanced highlighting and multi-pane interface.

## Quick Start

1. **Windows**: Double-click `TotalSearch.exe` or run `scripts/Start_TotalSearch.bat`
2. **Linux/Mac**: Run `scripts/Start_TotalSearch.sh`

## Features

- **Ultra-Fast Search**: Powered by ripgrep engine
- **Smart Highlighting**: Advanced syntax highlighting with custom rules
- **Multi-Pane Interface**: Collapsible search results with detachable panels
- **Intelligent Caching**: Smart file caching system
- **Advanced Rules Engine**: Custom highlighting rules with regex support
- **Performance Monitoring**: Real-time status indicators

## Directory Structure

```
TotalSearch/
├── TotalSearch.exe          # Main application
├── TotalSearchWatchdog.exe  # Health monitoring
├── TS.exe                   # Launcher
├── App.ini                  # Configuration file
├── Qt6Core.dll             # Qt libraries
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── lib/                     # Libraries
│   ├── Scintilla.lib
│   ├── ScintillaEditBase.lib
│   └── rg.exe              # Search engine
├── plugins/                 # Qt plugins
│   ├── platforms/
│   ├── imageformats/
│   └── iconengines/
├── data/                    # Application data
│   ├── config/             # Configuration files
│   ├── cache/              # Cache files
│   ├── logs/               # Log files
│   └── user/               # User data
├── docs/                    # Documentation
│   ├── README.md
│   ├── TODO.md
│   └── LICENSING.md
└── scripts/                 # Launch scripts
    ├── Start_TotalSearch.bat
    └── Start_TotalSearch.sh
```

## Configuration

Edit `App.ini` to customize:
- Search settings
- UI preferences
- File viewer options
- Logging configuration

## Support

For support and updates, visit our website or contact support.
