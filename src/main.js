const { app, BrowserWindow, ipcMain, dialog, Menu } = require('electron');
const path = require('path');
const fs = require('fs-extra');
const chokidar = require('chokidar');
const glob = require('glob');

let mainWindow;
let fileWatchers = new Map();

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
      enableRemoteModule: true
    },
    icon: path.join(__dirname, '../assets/icon.png'),
    titleBarStyle: 'default',
    show: false
  });

  mainWindow.loadFile(path.join(__dirname, 'renderer/index.html'));

  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
  });

  mainWindow.on('closed', () => {
    mainWindow = null;
  });

  // Create menu
  const template = [
    {
      label: 'File',
      submenu: [
        {
          label: 'Open File',
          accelerator: 'CmdOrCtrl+O',
          click: () => {
            openFileDialog();
          }
        },
        {
          label: 'Open Folder',
          accelerator: 'CmdOrCtrl+Shift+O',
          click: () => {
            openFolderDialog();
          }
        },
        {
          type: 'separator'
        },
        {
          label: 'Exit',
          accelerator: process.platform === 'darwin' ? 'Cmd+Q' : 'Ctrl+Q',
          click: () => {
            app.quit();
          }
        }
      ]
    },
    {
      label: 'Edit',
      submenu: [
        { role: 'undo' },
        { role: 'redo' },
        { type: 'separator' },
        { role: 'cut' },
        { role: 'copy' },
        { role: 'paste' }
      ]
    },
    {
      label: 'View',
      submenu: [
        { role: 'reload' },
        { role: 'forceReload' },
        { role: 'toggleDevTools' },
        { type: 'separator' },
        { role: 'resetZoom' },
        { role: 'zoomIn' },
        { role: 'zoomOut' },
        { type: 'separator' },
        { role: 'togglefullscreen' }
      ]
    }
  ];

  const menu = Menu.buildFromTemplate(template);
  Menu.setApplicationMenu(menu);
}

async function openFileDialog() {
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openFile'],
    filters: [
      { name: 'All Files', extensions: ['*'] },
      { name: 'Text Files', extensions: ['txt', 'log', 'md', 'json', 'xml', 'csv'] },
      { name: 'Code Files', extensions: ['js', 'ts', 'py', 'java', 'cpp', 'c', 'h', 'hpp'] }
    ]
  });

  if (!result.canceled && result.filePaths.length > 0) {
    mainWindow.webContents.send('file-opened', result.filePaths[0]);
  }
}

async function openFolderDialog() {
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openDirectory']
  });

  if (!result.canceled && result.filePaths.length > 0) {
    mainWindow.webContents.send('folder-opened', result.filePaths[0]);
  }
}

// IPC handlers
ipcMain.handle('read-file', async (event, filePath) => {
  try {
    const content = await fs.readFile(filePath, 'utf8');
    return { success: true, content, filePath };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('search-files', async (event, searchParams) => {
  const { directory, pattern, fileTypes, caseSensitive, useRegex } = searchParams;
  
  try {
    const globPattern = path.join(directory, '**/*');
    const files = await new Promise((resolve, reject) => {
      glob(globPattern, { nodir: true }, (err, files) => {
        if (err) reject(err);
        else resolve(files);
      });
    });

    const results = [];
    const searchRegex = useRegex ? new RegExp(pattern, caseSensitive ? 'g' : 'gi') : 
                                 new RegExp(pattern.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), caseSensitive ? 'g' : 'gi');

    for (const file of files) {
      if (fileTypes && fileTypes.length > 0) {
        const ext = path.extname(file).toLowerCase();
        if (!fileTypes.includes(ext)) continue;
      }

      try {
        const content = await fs.readFile(file, 'utf8');
        const matches = content.match(searchRegex);
        
        if (matches) {
          const lines = content.split('\n');
          const lineMatches = [];
          
          lines.forEach((line, index) => {
            const lineMatches = line.match(searchRegex);
            if (lineMatches) {
              lineMatches.push({
                line: index + 1,
                content: line,
                matches: lineMatches.length
              });
            }
          });

          results.push({
            file,
            matches: matches.length,
            lineMatches
          });
        }
      } catch (error) {
        // Skip files that can't be read
        continue;
      }
    }

    return { success: true, results };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('watch-file', async (event, filePath) => {
  if (fileWatchers.has(filePath)) {
    fileWatchers.get(filePath).close();
  }

  const watcher = chokidar.watch(filePath, {
    persistent: true,
    awaitWriteFinish: {
      stabilityThreshold: 2000,
      pollInterval: 100
    }
  });

  watcher.on('change', (path) => {
    mainWindow.webContents.send('file-changed', path);
  });

  fileWatchers.set(filePath, watcher);
  return { success: true };
});

ipcMain.handle('unwatch-file', async (event, filePath) => {
  if (fileWatchers.has(filePath)) {
    fileWatchers.get(filePath).close();
    fileWatchers.delete(filePath);
  }
  return { success: true };
});

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

app.on('before-quit', () => {
  // Clean up file watchers
  fileWatchers.forEach(watcher => watcher.close());
  fileWatchers.clear();
}); 