# Fast File Opening and Display Techniques

## Overview

The key to fast file opening and display lies in **lazy loading** and **efficient data structures**. Instead of loading entire files into memory, we use techniques that only load what's needed when it's needed.

## Scrollbar Behavior: Full Range from the Start

With the line offset indexing approach, the vertical scrollbar always shows the **full range of the file** immediately after opening, even for files with millions of lines. This is because:
- The model builds an index of all line offsets during file open (O(n) time, O(l) memory for n bytes, l lines)
- The model reports the total number of lines to the view (`rowCount()`)
- The view (QListView, etc.) sets the scrollbar range to match the total number of lines
- Only visible lines are loaded and rendered, but the scrollbar always represents the entire file

**Result:**
- You can instantly scroll anywhere in the file, regardless of file size
- The scrollbar thumb size and range are correct for the whole file

This is a key advantage of the line-offset indexing approach: you get instant, accurate, full-range scrolling, just like in Klogg, even for huge files.

## Current Qt Implementation (C++)

### 1. **Line Offset Indexing**
```cpp
QVector<qint64> m_lineOffsets; // Byte offsets for each line start
```

**How it works:**
- During file loading, we scan the file once and record the byte position of each line start
- This creates a "map" of where each line begins in the file
- Time complexity: O(n) for initial scan, where n = file size
- Space complexity: O(l) where l = number of lines

**Benefits:**
- Instant access to any line without reading from the beginning
- No need to scan through previous lines to reach line N
- Memory usage is minimal (just offsets, not content)

### 2. **Model-View Architecture**
```cpp
class FileLineModel : public QAbstractListModel
QListView *fileContentView;
```

**How it works:**
- Qt's Model-View pattern only requests data for visible items
- When you scroll, Qt asks the model: "Give me line 1000-1050"
- The model uses the offset index to seek directly to line 1000
- Only visible lines are loaded into memory

**Benefits:**
- Memory usage stays constant regardless of file size
- Scrolling performance is O(1) for any file size
- UI remains responsive even with 1GB+ files

### 3. **Lazy Data Loading**
```cpp
QString FileLineModel::getLine(int row) const {
    m_file.seek(m_lineOffsets[row]);  // Jump directly to line
    QByteArray line = m_file.readLine(); // Read only this line
    return QString::fromUtf8(line);
}
```

**How it works:**
- File remains open but only reads when needed
- Each line is read on-demand when the view requests it
- No caching of content (unless Qt's view decides to cache visible items)

## Performance Characteristics

| Operation | Time Complexity | Memory Usage |
|-----------|----------------|--------------|
| File Open | O(n) - one scan | O(l) - line offsets |
| Display Line N | O(1) - direct seek | O(1) - one line |
| Scroll | O(1) - model provides | O(v) - visible lines |
| Search | O(n) - scan all lines | O(r) - results only |

Where:
- n = file size in bytes
- l = number of lines
- v = visible lines (typically 50-100)
- r = search results

## Rust Implementation

### 1. **Using egui (Immediate Mode GUI)**
```rust
use egui::{ScrollArea, Ui};
use std::fs::File;
use std::io::{BufRead, BufReader, Seek, SeekFrom};

struct FileViewer {
    file: File,
    line_offsets: Vec<u64>,
    visible_range: (usize, usize),
}

impl FileViewer {
    fn load_file(&mut self, path: &str) -> Result<(), std::io::Error> {
        let mut file = File::open(path)?;
        let mut offsets = vec![0];
        let mut buffer = [0; 8192];
        
        // Build line offset index
        loop {
            let bytes_read = file.read(&mut buffer)?;
            if bytes_read == 0 { break; }
            
            for &byte in &buffer[..bytes_read] {
                if byte == b'\n' {
                    offsets.push(file.stream_position()?);
                }
            }
        }
        
        self.line_offsets = offsets;
        Ok(())
    }
    
    fn get_line(&mut self, line_num: usize) -> Option<String> {
        if line_num >= self.line_offsets.len() - 1 { return None; }
        
        self.file.seek(SeekFrom::Start(self.line_offsets[line_num]))?;
        let mut line = String::new();
        self.file.read_line(&mut line)?;
        Some(line)
    }
    
    fn render(&mut self, ui: &mut Ui) {
        ScrollArea::vertical().max_height(400.0).show(ui, |ui| {
            let visible_lines = self.calculate_visible_lines(ui);
            
            for line_num in visible_lines {
                if let Some(line) = self.get_line(line_num) {
                    ui.label(&format!("{}: {}", line_num + 1, line));
                }
            }
        });
    }
}
```

### 2. **Using Tauri (WebView + Rust Backend)**
```rust
// backend/src/main.rs
use tauri::{command, State};
use std::sync::Mutex;
use std::fs::File;
use std::io::{BufRead, BufReader, Seek, SeekFrom};

struct FileState {
    file: Option<File>,
    line_offsets: Vec<u64>,
}

#[command]
async fn open_file(path: String, state: State<'_, Mutex<FileState>>) -> Result<Vec<u64>, String> {
    let mut state = state.lock().unwrap();
    let mut file = File::open(&path).map_err(|e| e.to_string())?;
    
    // Build offset index
    let mut offsets = vec![0];
    let mut buffer = [0; 8192];
    
    loop {
        let bytes_read = file.read(&mut buffer).map_err(|e| e.to_string())?;
        if bytes_read == 0 { break; }
        
        for &byte in &buffer[..bytes_read] {
            if byte == b'\n' {
                offsets.push(file.stream_position().map_err(|e| e.to_string())?);
            }
        }
    }
    
    state.file = Some(file);
    state.line_offsets = offsets.clone();
    Ok(offsets)
}

#[command]
async fn get_lines(start: usize, count: usize, state: State<'_, Mutex<FileState>>) -> Result<Vec<String>, String> {
    let mut state = state.lock().unwrap();
    let file = state.file.as_mut().ok_or("No file open")?;
    
    let mut lines = Vec::new();
    for i in start..(start + count).min(state.line_offsets.len() - 1) {
        file.seek(SeekFrom::Start(state.line_offsets[i])).map_err(|e| e.to_string())?;
        let mut line = String::new();
        file.read_line(&mut line).map_err(|e| e.to_string())?;
        lines.push(line);
    }
    
    Ok(lines)
}
```

```javascript
// frontend/src/main.js
import { invoke } from '@tauri-apps/api/core';

class FileViewer {
    constructor() {
        this.lineOffsets = [];
        this.visibleLines = new Map();
        this.scrollTop = 0;
    }
    
    async openFile(path) {
        this.lineOffsets = await invoke('open_file', { path });
        this.render();
    }
    
    async loadVisibleLines() {
        const container = document.getElementById('file-content');
        const visibleHeight = container.clientHeight;
        const lineHeight = 20; // pixels per line
        const visibleCount = Math.ceil(visibleHeight / lineHeight);
        
        const startLine = Math.floor(this.scrollTop / lineHeight);
        const endLine = startLine + visibleCount;
        
        // Load only visible lines
        const lines = await invoke('get_lines', { 
            start: startLine, 
            count: visibleCount 
        });
        
        // Update virtual scroll
        this.updateVirtualScroll(startLine, lines);
    }
    
    updateVirtualScroll(startLine, lines) {
        const container = document.getElementById('file-content');
        const totalHeight = this.lineOffsets.length * 20;
        
        container.style.height = `${totalHeight}px`;
        container.innerHTML = '';
        
        lines.forEach((line, index) => {
            const lineElement = document.createElement('div');
            lineElement.className = 'file-line';
            lineElement.style.position = 'absolute';
            lineElement.style.top = `${(startLine + index) * 20}px`;
            lineElement.textContent = `${startLine + index + 1}: ${line}`;
            container.appendChild(lineElement);
        });
    }
}
```

## Web-Based Implementation

### 1. **Pure JavaScript with Virtual Scrolling**
```javascript
class FastFileViewer {
    constructor() {
        this.lineOffsets = [];
        this.file = null;
        this.visibleLines = new Map();
    }
    
    async openFile(file) {
        this.file = file;
        this.lineOffsets = await this.buildLineIndex(file);
        this.render();
    }
    
    async buildLineIndex(file) {
        const offsets = [0];
        const chunkSize = 64 * 1024; // 64KB chunks
        let position = 0;
        
        while (position < file.size) {
            const chunk = file.slice(position, position + chunkSize);
            const text = await chunk.text();
            
            for (let i = 0; i < text.length; i++) {
                if (text[i] === '\n') {
                    offsets.push(position + i + 1);
                }
            }
            position += chunkSize;
        }
        
        return offsets;
    }
    
    async getLine(lineNumber) {
        if (lineNumber >= this.lineOffsets.length - 1) return null;
        
        const start = this.lineOffsets[lineNumber];
        const end = this.lineOffsets[lineNumber + 1] || this.file.size;
        
        const chunk = this.file.slice(start, end);
        const text = await chunk.text();
        return text.replace(/\r?\n$/, '');
    }
    
    async renderVisibleLines() {
        const container = document.getElementById('file-content');
        const visibleHeight = container.clientHeight;
        const lineHeight = 20;
        const visibleCount = Math.ceil(visibleHeight / lineHeight);
        
        const startLine = Math.floor(container.scrollTop / lineHeight);
        const endLine = startLine + visibleCount;
        
        // Load visible lines
        const lines = [];
        for (let i = startLine; i < endLine; i++) {
            const line = await this.getLine(i);
            if (line !== null) {
                lines.push({ number: i + 1, content: line });
            }
        }
        
        this.updateVirtualScroll(startLine, lines);
    }
}
```

### 2. **WebAssembly (WASM) for Performance**
```rust
// lib.rs
use wasm_bindgen::prelude::*;
use std::io::{BufRead, BufReader};

#[wasm_bindgen]
pub struct FileViewer {
    line_offsets: Vec<u64>,
    file_data: Vec<u8>,
}

#[wasm_bindgen]
impl FileViewer {
    pub fn new() -> FileViewer {
        FileViewer {
            line_offsets: Vec::new(),
            file_data: Vec::new(),
        }
    }
    
    pub fn load_file(&mut self, data: &[u8]) {
        self.file_data = data.to_vec();
        self.build_line_index();
    }
    
    fn build_line_index(&mut self) {
        self.line_offsets.clear();
        self.line_offsets.push(0);
        
        for (i, &byte) in self.file_data.iter().enumerate() {
            if byte == b'\n' {
                self.line_offsets.push((i + 1) as u64);
            }
        }
    }
    
    pub fn get_line(&self, line_number: usize) -> Option<String> {
        if line_number >= self.line_offsets.len() - 1 {
            return None;
        }
        
        let start = self.line_offsets[line_number] as usize;
        let end = self.line_offsets[line_number + 1] as usize;
        
        let line_data = &self.file_data[start..end];
        Some(String::from_utf8_lossy(line_data).to_string())
    }
    
    pub fn line_count(&self) -> usize {
        self.line_offsets.len() - 1
    }
}
```

## Performance Comparison

| Implementation | File Open | Line Access | Memory Usage | Scalability |
|----------------|-----------|-------------|--------------|-------------|
| **Qt (C++)** | O(n) | O(1) | O(l) | Excellent |
| **Rust + egui** | O(n) | O(1) | O(l) | Excellent |
| **Rust + Tauri** | O(n) | O(1) | O(l) | Excellent |
| **Pure JS** | O(n) | O(1) | O(l) | Good |
| **WASM** | O(n) | O(1) | O(n) | Good |

## Key Performance Principles

### 1. **Index Once, Access Fast**
- Build line offset index during file open
- Use binary search or direct indexing for line access
- Never scan from beginning to reach line N

### 2. **Load Only What's Visible**
- Virtual scrolling: only render visible lines
- Lazy loading: load content on-demand
- Minimal memory footprint regardless of file size

### 3. **Efficient Data Structures**
- Use vectors/arrays for O(1) access
- Avoid linked lists or complex structures
- Keep data layout cache-friendly

### 4. **Async When Possible**
- Use background threads for file I/O
- Don't block UI thread during file operations
- Implement progressive loading for very large files

## Can Web-Based Solutions Match Native Performance?

**Yes, with caveats:**

### **Advantages of Web-Based:**
- WASM can achieve near-native performance
- Modern browsers have excellent virtual scrolling
- Web Workers for background processing
- Cross-platform deployment

### **Limitations:**
- JavaScript garbage collection overhead
- Browser memory limits (typically 2-4GB)
- Network overhead for remote files
- Less direct file system access

### **Best Web Performance:**
1. **WASM + Virtual Scrolling** - Near-native performance
2. **Tauri (Rust + WebView)** - Native backend, web frontend
3. **Pure JS with optimizations** - Good for most use cases

## Conclusion

The key to fast file viewing is **not** loading everything into memory, but rather:
1. **Index the file structure** (line offsets)
2. **Load only visible content** (virtual scrolling)
3. **Use efficient data structures** (arrays, direct access)
4. **Implement lazy loading** (on-demand content)

This approach works equally well in C++, Rust, and JavaScript, with the main difference being the underlying performance characteristics of each platform. 