# TotalSearch Application - Icon Design Requirements

## Project Overview
TotalSearch is a powerful search application with detachable panes functionality. We need a complete set of professional icons that maintain visual consistency and provide clear user interface guidance.

## Design Guidelines
- **Style**: Modern, clean, minimalist design
- **Color Palette**: 
  - Primary: Blue (#3498db)
  - Secondary: Gray (#7f8c8d) 
  - Success: Green (#2ecc71)
  - Warning: Orange (#f39c12)
  - Danger: Red (#e74c3c)
  - Purple: (#9b59b6)
- **Visual Consistency**: All icons should follow the same design language
- **Scalability**: Icons must be crisp at all specified sizes
- **Platform**: Windows-focused design with modern flat/material design elements

---

## Icon Requirements

### 1. Application Icons

#### 1.1 Main Application Icon (`app_icon.ico`)
- **Purpose**: Primary application icon shown in file explorer, desktop shortcuts, and taskbar
- **Image Description**: 
  - Main symbol: Magnifying glass with search rays or stylized "TS" letters
  - Background: Circular or rounded square with gradient blue background
  - Style: Professional, modern, recognizable at small sizes
- **Sizes Required**: 16x16, 32x32, 48x48, 256x256 pixels
- **Format**: Windows ICO file with multiple embedded sizes
- **Colors**: Primary blue (#3498db) with white/light accents

#### 1.2 Taskbar Icon (`taskbar_icon.ico`)
- **Purpose**: Icon displayed in Windows taskbar and system tray
- **Image Description**: 
  - Simplified version of main app icon
  - High contrast for visibility on different taskbar themes
  - Clear at very small sizes
- **Sizes Required**: 16x16, 32x32 pixels
- **Format**: Windows ICO file
- **Colors**: Primary blue with high contrast white elements

---

### 2. Pane Control Icons (UI Elements)

#### 2.1 Detach Pane Icon (`detach_pane.png`)
- **Purpose**: Button to detach a pane from main window to separate window
- **Image Description**: 
  - Arrow pointing outward/right (→) or window with arrow moving out
  - Small window icon with separation indicator
  - Suggests "breaking away" or "moving out"
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Green (#2ecc71) to suggest positive action

#### 2.2 Attach Pane Icon (`attach_pane.png`)
- **Purpose**: Button to reattach a detached pane back to main window
- **Image Description**: 
  - Arrow pointing inward/left (←) or window with arrow moving in
  - Small window icon with connection indicator
  - Suggests "joining" or "moving back"
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Purple (#9b59b6) to suggest return/connection action

#### 2.3 Maximize Window Icon (`maximize_window.png`)
- **Purpose**: Button to maximize a detached window to full screen
- **Image Description**: 
  - Square or rectangle outline (□)
  - Clean geometric shape suggesting full screen
  - Traditional maximize symbol
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Gray (#7f8c8d) for neutral window control

#### 2.4 Close Window Icon (`close_window.png`)
- **Purpose**: Button to close a detached window
- **Image Description**: 
  - X symbol (×) or cross
  - Clean, bold lines
  - Traditional close symbol
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Red (#e74c3c) to suggest destructive action

---

### 3. Pane Type Icons

#### 3.1 Search Results Icon (`search_results.png`)
- **Purpose**: Icon representing the search results pane
- **Image Description**: 
  - List with magnifying glass overlay
  - Document with search results lines
  - Bullet points or list items with search symbol
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Orange (#f39c12) for search-related functionality

#### 3.2 File Viewer Icon (`file_viewer.png`)
- **Purpose**: Icon representing the file content viewer pane
- **Image Description**: 
  - Document/file icon with text lines
  - Eye symbol over document
  - Page with visible content lines
- **Size**: 16x16 pixels
- **Format**: PNG with transparency
- **Colors**: Blue (#3498db) for file viewing functionality

---

### 4. Existing Icons (Reference - Already Present)

#### 4.1 Save Icon (`save.png`)
- **Purpose**: Save file action
- **Current**: Floppy disk symbol
- **Size**: 16x16 pixels

#### 4.2 Open Icon (`open.png`)
- **Purpose**: Open file action
- **Current**: Folder opening symbol
- **Size**: 16x16 pixels

#### 4.3 File Icon (`file.png`)
- **Purpose**: Generic file representation
- **Current**: Document symbol
- **Size**: 16x16 pixels

#### 4.4 List Icon (`list.png`)
- **Purpose**: List view representation
- **Current**: Horizontal lines symbol
- **Size**: 16x16 pixels

#### 4.5 Clear Icon (`clear.png`)
- **Purpose**: Clear/reset action
- **Current**: X or clear symbol
- **Size**: 16x16 pixels

#### 4.6 Search Icon (`search.png`)
- **Purpose**: Search action
- **Current**: Magnifying glass
- **Size**: 16x16 pixels

---

## Technical Specifications

### File Formats
- **ICO Files**: For Windows application icons (support multiple sizes in one file)
- **PNG Files**: For UI elements (with alpha transparency)

### Color Requirements
- **Transparency**: All PNG files must support alpha transparency
- **Color Depth**: 32-bit color with alpha channel
- **Compression**: Optimized for small file sizes while maintaining quality

### Quality Standards
- **Pixel Perfect**: Icons must be designed at target resolution (no scaling artifacts)
- **Anti-aliasing**: Smooth edges where appropriate
- **Consistency**: Uniform stroke width, corner radius, and visual weight across all icons
- **Accessibility**: Sufficient contrast for visibility on light and dark backgrounds

---

## Deliverables

### Primary Deliverables (High Priority)
1. `app_icon.ico` - Multi-size application icon
2. `taskbar_icon.ico` - Taskbar-optimized icon
3. `detach_pane.png` - Detach functionality icon
4. `attach_pane.png` - Attach functionality icon
5. `maximize_window.png` - Window maximize icon
6. `close_window.png` - Window close icon
7. `search_results.png` - Search results pane icon
8. `file_viewer.png` - File viewer pane icon

### Optional Enhancements (Lower Priority)
- Hover states for interactive icons
- High-DPI versions (2x, 3x scaling)
- Dark theme variants
- Animated versions for loading states

---

## Usage Context

These icons will be used in:
- **Windows Desktop Application**: TotalSearch file search and content viewer
- **Target Users**: Developers, power users, system administrators
- **Use Cases**: Code searching, log file analysis, text file examination
- **Environment**: Windows 10/11 desktop application with modern UI

## Brand Consistency

The icon set should convey:
- **Professionalism**: Clean, modern design suitable for professional use
- **Functionality**: Clear visual metaphors for each action
- **Reliability**: Stable, trustworthy appearance
- **Efficiency**: Streamlined design suggesting fast, efficient operations

---

## File Naming Convention
- Use exact filenames as specified above
- All lowercase with underscores
- Maintain file extensions as specified (.ico for application icons, .png for UI elements)

## Testing Requirements
- Icons should be tested on both light and dark Windows themes
- Verify clarity at actual display sizes (not zoomed)
- Ensure proper transparency handling
- Test on high-DPI displays if possible
