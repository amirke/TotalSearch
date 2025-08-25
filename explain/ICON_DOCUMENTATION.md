# TotalSearch Icon Documentation

## 📁 Icon Files and Their Usage

This document provides a comprehensive overview of all icons used in the TotalSearch application, their locations, and usage patterns.

### Icon Files Table

| **Icon File** 		| **Current Usage** 		| **Location** 																| **Description** |
|---------------		|-------------------		|--------------																|-----------------|
| `app_icon.ico` 		| **Main Application Icon** | `main.cpp` - `app.setWindowIcon()` 										| Application window icon |
| `taskbar_icon.ico` 	| **Taskbar Icon** 			| Available but not currently used 											| Taskbar icon (reserved) |
| `open.png` 			| **Browse Button** 		| `mainwindow.cpp` - Browse directory button 								| Folder icon for directory selection |
| `search.png` 			| **Search Button** 		| `mainwindow.cpp` - Start search button 									| Magnifying glass for search functionality |
| `close_window.png` 	| **Stop Button** 			| `mainwindow.cpp` - Stop search button 									| Stop icon for stopping operations |
| `close_window.png` 	| **Cancel Buttons** 		| `configurationdialog.cpp`, `highlightdialog.cpp`, `rgsearchdialog.cpp` 	| Cancel/close functionality |
| `file.png` 			| **Config Button** 		| `mainwindow.cpp` - Configuration button 									| Settings icon for configuration |
| `file.png` 			| **Font Buttons** 			| `configurationdialog.cpp` - Choose font buttons 							| Font selection functionality |
| `file.png` 			| **Status Bar Color** 		| `configurationdialog.cpp` - Status bar color button 						| Status bar color selection |
| `search_results.png` 	| **Highlight Button** 		| `mainwindow.cpp` - Highlight button 										| Highlight functionality |
| `search_results.png` 	| **Search Results Color** 	| `configurationdialog.cpp` - Search results color button 					| Search results color selection |
| `search_results.png` 	| **Search Results Pane** 	| `DetachablePane.cpp` - Pane title icon 									| Search results pane identification |
| `list.png` 			| **Engineering Button** 	| `mainwindow.cpp` - Engineering button 									| Engineering tools functionality |
| `list.png` 			| **Log Window Color** 		| `configurationdialog.cpp` - Log window color button 						| Log window color selection |
| `save.png` 			| **Apply Buttons** 		| `configurationdialog.cpp`, `highlightdialog.cpp`, `engineeringdialog.cpp` | Apply/save functionality |
| `clear.png` 			| **Clear Buttons** 		| `mainwindow.cpp` - Clear pane buttons 									| Clear functionality |
| `clear.png` 			| **Apply Test Text** 		| `mainwindow.cpp` - Apply test text button 								| Test functionality |
| `detach_pane.png` 	| **Detach Pane Button** 	| `DetachablePane.cpp` - Detach pane control 								| Detach pane functionality |
| `attach_pane.png` 	| **Attach Pane Button** 	| `DetachablePane.cpp` - Attach pane control 								| Attach pane functionality |
| `maximize_window.png` | **Maximize Button** 		| `DetachablePane.cpp` - Maximize window control 							| Window maximize functionality |
| `file_viewer.png` 	| **File Viewer Color** 	| `configurationdialog.cpp` - File viewer color button 						| File viewer color selection |
| `file_viewer.png` 	| **File Viewer Pane** 		| `DetachablePane.cpp` - Pane title icon 									| File viewer pane identification |

## 🎯 Icon Usage Patterns

### Action Icons
Icons used for primary application actions and functionality:

- **Browse**: `open.png` - Folder icon for directory selection
- **Search**: `search.png` - Magnifying glass for search functionality  
- **Stop**: `close_window.png` - Stop icon for stopping operations
- **Config**: `file.png` - Settings icon for configuration
- **Highlight**: `search_results.png` - Highlight icon for highlighting features
- **Engineering**: `list.png` - Engineering icon for engineering features

### Pane Control Icons
Icons used for managing detachable panes and window controls:

- **Detach**: `detach_pane.png` - Arrow pointing out (detach from main window)
- **Attach**: `attach_pane.png` - Arrow pointing in (attach to main window)
- **Maximize**: `maximize_window.png` - Square icon (maximize window)
- **Close**: `close_window.png` - X icon (close window)

### Pane Title Icons
Icons used to identify different pane types in the detachable pane system:

- **Search Results**: `search_results.png` - Identifies search results pane
- **File Viewer**: `file_viewer.png` - Identifies file viewer pane

### Color Selection Icons
Icons used in color picker buttons for different GUI components:

- **Search Results Color**: `search_results.png` - Color selection for search results
- **File Viewer Color**: `file_viewer.png` - Color selection for file viewer
- **Log Window Color**: `list.png` - Color selection for log window
- **Status Bar Color**: `file.png` - Color selection for status bar

### Apply/Cancel Icons
Icons used for dialog action buttons:

- **Apply**: Check mark (✓) - Programmatically created Unicode character
- **Cancel**: `close_window.png` - Standard cancel/close icon

### Utility Icons
Icons used for utility and maintenance functions:

- **Clear**: `clear.png` - Clear functionality for various panes
- **Save**: `save.png` - Save/apply functionality (legacy, now replaced with check mark)
- **Font**: `file.png` - Font selection functionality

## 🎨 Icon Design Principles

### Visual Consistency
- **Size**: Most icons are 16x16 pixels for UI elements
- **Style**: Simple, recognizable symbols
- **Color**: Icons adapt to button styling (white on colored backgrounds)

### Semantic Meaning
- **Action-oriented**: Icons represent the action they perform
- **Intuitive**: Common symbols that users recognize
- **Contextual**: Icons match their functional context

### Accessibility
- **Tooltips**: All icons have descriptive tooltips
- **Text labels**: Icons are accompanied by text labels
- **High contrast**: Icons are visible on various backgrounds

## 🔧 Technical Implementation

### Icon Loading
```cpp
// Standard icon loading from resource file
button->setIcon(QIcon(":/icons/icon_name.png"));

// Programmatic icon creation (for check marks)
QPixmap checkPixmap(16, 16);
checkPixmap.fill(Qt::transparent);
QPainter painter(&checkPixmap);
painter.setPen(QPen(Qt::white, 2));
painter.setFont(QFont("Arial", 12, QFont::Bold));
painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
button->setIcon(QIcon(checkPixmap));
```

### Resource Management
- **Qt Resource System**: All icons are compiled into the executable via `icons.qrc`
- **Memory Efficient**: Icons are loaded once and reused
- **Scalable**: Icons scale properly on different DPI displays

### Icon States
- **Normal**: Default icon appearance
- **Hover**: Icon may change appearance on hover
- **Pressed**: Icon may change appearance when pressed
- **Disabled**: Icon may be grayed out when disabled

## 📋 Icon Maintenance

### Adding New Icons
1. Add icon file to `icons/` directory
2. Add entry to `icons/icons.qrc` resource file
3. Update this documentation
4. Test icon appearance and functionality

### Icon Guidelines
- **Format**: PNG for transparency support
- **Size**: 16x16 pixels for UI elements, 32x32 for application icons
- **Style**: Simple, flat design with good contrast
- **Naming**: Descriptive, lowercase with underscores

### Future Considerations
- **Vector Icons**: Consider SVG format for better scaling
- **Theme Support**: Icons could adapt to light/dark themes
- **Customization**: Users could potentially customize icons
- **Accessibility**: Ensure icons meet accessibility standards

---

*This documentation is maintained as part of the TotalSearch application development process.*
