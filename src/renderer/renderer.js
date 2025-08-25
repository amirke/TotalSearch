const { ipcRenderer } = require('electron');

// Global state
let currentFile = null;
let currentDirectory = null;
let searchResults = [];
let fileContent = '';
let isWatchingFile = false;
let autoScroll = true;

// DOM elements
const elements = {
    // Buttons
    openFileBtn: document.getElementById('openFileBtn'),
    openFolderBtn: document.getElementById('openFolderBtn'),
    searchBtn: document.getElementById('searchBtn'),
    watchFileBtn: document.getElementById('watchFileBtn'),
    unwatchFileBtn: document.getElementById('unwatchFileBtn'),
    goToLineBtn: document.getElementById('goToLineBtn'),
    findInFileBtn: document.getElementById('findInFileBtn'),
    autoScrollBtn: document.getElementById('autoScrollBtn'),
    clearLogBtn: document.getElementById('clearLogBtn'),
    
    // Inputs
    searchInput: document.getElementById('searchInput'),
    caseSensitive: document.getElementById('caseSensitive'),
    useRegex: document.getElementById('useRegex'),
    
    // Displays
    currentFileInfo: document.getElementById('currentFileInfo'),
    searchResults: document.getElementById('searchResults'),
    fileViewer: document.getElementById('fileViewer'),
    logViewer: document.getElementById('logViewer'),
    resultsCount: document.getElementById('resultsCount'),
    searchTime: document.getElementById('searchTime'),
    statusText: document.getElementById('statusText'),
    filePath: document.getElementById('filePath'),
    
    // Modals
    goToLineModal: document.getElementById('goToLineModal'),
    findInFileModal: document.getElementById('findInFileModal'),
    lineNumberInput: document.getElementById('lineNumberInput'),
    findInFileInput: document.getElementById('findInFileInput'),
    findCaseSensitive: document.getElementById('findCaseSensitive'),
    findUseRegex: document.getElementById('findUseRegex'),
    
    // Modal buttons
    goToLineConfirm: document.getElementById('goToLineConfirm'),
    goToLineCancel: document.getElementById('goToLineCancel'),
    findInFileConfirm: document.getElementById('findInFileConfirm'),
    findInFileCancel: document.getElementById('findInFileCancel')
};

// Initialize the application
function init() {
    setupEventListeners();
    setupTabNavigation();
    updateStatus('Ready');
}

// Setup event listeners
function setupEventListeners() {
    // File operations
    elements.openFileBtn.addEventListener('click', () => {
        // This will be handled by the main process
    });
    
    elements.openFolderBtn.addEventListener('click', () => {
        // This will be handled by the main process
    });
    
    // Search functionality
    elements.searchBtn.addEventListener('click', performSearch);
    elements.searchInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            performSearch();
        }
    });
    
    // File watching
    elements.watchFileBtn.addEventListener('click', watchCurrentFile);
    elements.unwatchFileBtn.addEventListener('click', unwatchCurrentFile);
    
    // File viewer controls
    elements.goToLineBtn.addEventListener('click', showGoToLineModal);
    elements.findInFileBtn.addEventListener('click', showFindInFileModal);
    
    // Log viewer controls
    elements.autoScrollBtn.addEventListener('click', toggleAutoScroll);
    elements.clearLogBtn.addEventListener('click', clearLogViewer);
    
    // Modal controls
    elements.goToLineConfirm.addEventListener('click', goToLine);
    elements.goToLineCancel.addEventListener('click', hideGoToLineModal);
    elements.findInFileConfirm.addEventListener('click', findInFile);
    elements.findInFileCancel.addEventListener('click', hideFindInFileModal);
    
    // Modal backdrop clicks
    elements.goToLineModal.addEventListener('click', (e) => {
        if (e.target === elements.goToLineModal) {
            hideGoToLineModal();
        }
    });
    
    elements.findInFileModal.addEventListener('click', (e) => {
        if (e.target === elements.findInFileModal) {
            hideFindInFileModal();
        }
    });
}

// Setup tab navigation
function setupTabNavigation() {
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabPanes = document.querySelectorAll('.tab-pane');
    
    tabButtons.forEach(button => {
        button.addEventListener('click', () => {
            const targetTab = button.getAttribute('data-tab');
            
            // Update active tab button
            tabButtons.forEach(btn => btn.classList.remove('active'));
            button.classList.add('active');
            
            // Update active tab pane
            tabPanes.forEach(pane => pane.classList.remove('active'));
            document.getElementById(targetTab).classList.add('active');
        });
    });
}

// IPC event handlers
ipcRenderer.on('file-opened', async (event, filePath) => {
    await openFile(filePath);
});

ipcRenderer.on('folder-opened', async (event, folderPath) => {
    currentDirectory = folderPath;
    updateStatus(`Folder opened: ${folderPath}`);
    elements.filePath.textContent = folderPath;
});

ipcRenderer.on('file-changed', (event, filePath) => {
    if (filePath === currentFile) {
        refreshFileContent();
    }
    addLogEntry(`File changed: ${filePath}`, 'info');
});

// File operations
async function openFile(filePath) {
    try {
        updateStatus('Loading file...');
        
        const result = await ipcRenderer.invoke('read-file', filePath);
        
        if (result.success) {
            currentFile = filePath;
            fileContent = result.content;
            
            displayFileContent();
            updateFileInfo();
            updateStatus(`File loaded: ${filePath}`);
            elements.filePath.textContent = filePath;
            
            // Enable file watching controls
            elements.watchFileBtn.disabled = false;
            elements.unwatchFileBtn.disabled = false;
        } else {
            updateStatus(`Error loading file: ${result.error}`);
        }
    } catch (error) {
        updateStatus(`Error: ${error.message}`);
    }
}

function displayFileContent() {
    if (!fileContent) {
        elements.fileViewer.innerHTML = `
            <div class="empty-state">
                <i class="fas fa-file-alt"></i>
                <p>No file content to display</p>
            </div>
        `;
        return;
    }
    
    const lines = fileContent.split('\n');
    const lineElements = lines.map((line, index) => {
        const lineNumber = index + 1;
        const escapedContent = escapeHtml(line);
        return `
            <div class="file-line" data-line="${lineNumber}">
                <span class="line-number">${lineNumber}</span>
                <span class="line-content">${escapedContent}</span>
            </div>
        `;
    }).join('');
    
    elements.fileViewer.innerHTML = lineElements;
}

function updateFileInfo() {
    if (!currentFile) {
        elements.currentFileInfo.innerHTML = '<p>No file selected</p>';
        return;
    }
    
    const fileName = currentFile.split(/[/\\]/).pop();
    const fileSize = fileContent.length;
    const lineCount = fileContent.split('\n').length;
    
    elements.currentFileInfo.innerHTML = `
        <p><strong>Name:</strong> ${fileName}</p>
        <p><strong>Size:</strong> ${formatFileSize(fileSize)}</p>
        <p><strong>Lines:</strong> ${lineCount}</p>
        <p><strong>Path:</strong> ${currentFile}</p>
    `;
}

// Search functionality
async function performSearch() {
    if (!currentDirectory) {
        updateStatus('Please open a folder first');
        return;
    }
    
    const pattern = elements.searchInput.value.trim();
    if (!pattern) {
        updateStatus('Please enter a search pattern');
        return;
    }
    
    const caseSensitive = elements.caseSensitive.checked;
    const useRegex = elements.useRegex.checked;
    const fileTypes = getSelectedFileTypes();
    
    try {
        updateStatus('Searching files...');
        const startTime = Date.now();
        
        const result = await ipcRenderer.invoke('search-files', {
            directory: currentDirectory,
            pattern,
            caseSensitive,
            useRegex,
            fileTypes
        });
        
        const endTime = Date.now();
        const searchTime = endTime - startTime;
        
        if (result.success) {
            searchResults = result.results;
            displaySearchResults();
            updateSearchStats(searchResults.length, searchTime);
            updateStatus(`Search completed: ${searchResults.length} files found`);
        } else {
            updateStatus(`Search error: ${result.error}`);
        }
    } catch (error) {
        updateStatus(`Search error: ${error.message}`);
    }
}

function displaySearchResults() {
    if (searchResults.length === 0) {
        elements.searchResults.innerHTML = `
            <div class="empty-state">
                <i class="fas fa-search"></i>
                <p>No matches found</p>
            </div>
        `;
        return;
    }
    
    const resultsHtml = searchResults.map(result => {
        const fileName = result.file.split(/[/\\]/).pop();
        const lineMatches = result.lineMatches || [];
        
        const linesHtml = lineMatches.map(lineMatch => `
            <div class="result-line">
                <span class="line-number">${lineMatch.line}</span>
                <span class="line-content">${highlightMatches(lineMatch.content, lineMatch.matches)}</span>
            </div>
        `).join('');
        
        return `
            <div class="result-item">
                <div class="result-header">
                    <span class="result-file">${fileName}</span>
                    <span class="result-matches">${result.matches} matches</span>
                </div>
                <div class="result-lines">
                    ${linesHtml}
                </div>
            </div>
        `;
    }).join('');
    
    elements.searchResults.innerHTML = resultsHtml;
}

function updateSearchStats(count, time) {
    elements.resultsCount.textContent = `${count} results`;
    elements.searchTime.textContent = `(${time}ms)`;
}

function getSelectedFileTypes() {
    const checkboxes = document.querySelectorAll('.file-type-filters input[type="checkbox"]:checked');
    return Array.from(checkboxes).map(cb => cb.value);
}

// File watching
async function watchCurrentFile() {
    if (!currentFile) return;
    
    try {
        await ipcRenderer.invoke('watch-file', currentFile);
        isWatchingFile = true;
        elements.watchFileBtn.disabled = true;
        elements.unwatchFileBtn.disabled = false;
        updateStatus(`Watching file: ${currentFile}`);
        addLogEntry(`Started watching: ${currentFile}`, 'info');
    } catch (error) {
        updateStatus(`Error watching file: ${error.message}`);
    }
}

async function unwatchCurrentFile() {
    if (!currentFile) return;
    
    try {
        await ipcRenderer.invoke('unwatch-file', currentFile);
        isWatchingFile = false;
        elements.watchFileBtn.disabled = false;
        elements.unwatchFileBtn.disabled = true;
        updateStatus(`Stopped watching: ${currentFile}`);
        addLogEntry(`Stopped watching: ${currentFile}`, 'info');
    } catch (error) {
        updateStatus(`Error unwatching file: ${error.message}`);
    }
}

async function refreshFileContent() {
    if (currentFile) {
        await openFile(currentFile);
    }
}

// File viewer controls
function showGoToLineModal() {
    elements.goToLineModal.classList.add('active');
    elements.lineNumberInput.focus();
}

function hideGoToLineModal() {
    elements.goToLineModal.classList.remove('active');
    elements.lineNumberInput.value = '';
}

function goToLine() {
    const lineNumber = parseInt(elements.lineNumberInput.value);
    if (lineNumber && lineNumber > 0) {
        const lineElement = document.querySelector(`[data-line="${lineNumber}"]`);
        if (lineElement) {
            lineElement.scrollIntoView({ behavior: 'smooth', block: 'center' });
            lineElement.style.backgroundColor = '#fff3cd';
            setTimeout(() => {
                lineElement.style.backgroundColor = '';
            }, 2000);
        }
    }
    hideGoToLineModal();
}

function showFindInFileModal() {
    elements.findInFileModal.classList.add('active');
    elements.findInFileInput.focus();
}

function hideFindInFileModal() {
    elements.findInFileModal.classList.remove('active');
    elements.findInFileInput.value = '';
}

function findInFile() {
    const searchTerm = elements.findInFileInput.value.trim();
    if (!searchTerm) return;
    
    const caseSensitive = elements.findCaseSensitive.checked;
    const useRegex = elements.findUseRegex.checked;
    
    // Clear previous highlights
    const lines = document.querySelectorAll('.file-line');
    lines.forEach(line => {
        const content = line.querySelector('.line-content');
        content.innerHTML = escapeHtml(fileContent.split('\n')[parseInt(line.dataset.line) - 1]);
    });
    
    // Find and highlight matches
    const searchRegex = useRegex ? 
        new RegExp(searchTerm, caseSensitive ? 'g' : 'gi') :
        new RegExp(searchTerm.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), caseSensitive ? 'g' : 'gi');
    
    let matchCount = 0;
    lines.forEach(line => {
        const content = line.querySelector('.line-content');
        const text = content.textContent;
        const matches = text.match(searchRegex);
        
        if (matches) {
            matchCount += matches.length;
            content.innerHTML = text.replace(searchRegex, '<span class="highlight">$&</span>');
        }
    });
    
    updateStatus(`Found ${matchCount} matches in file`);
    hideFindInFileModal();
}

// Log viewer controls
function toggleAutoScroll() {
    autoScroll = !autoScroll;
    elements.autoScrollBtn.classList.toggle('active', autoScroll);
    updateStatus(`Auto scroll ${autoScroll ? 'enabled' : 'disabled'}`);
}

function clearLogViewer() {
    elements.logViewer.innerHTML = `
        <div class="empty-state">
            <i class="fas fa-stream"></i>
            <p>Log viewer cleared</p>
        </div>
    `;
}

function addLogEntry(message, level = 'info') {
    const timestamp = new Date().toLocaleTimeString();
    const logEntry = document.createElement('div');
    logEntry.className = 'log-entry';
    logEntry.innerHTML = `
        <span class="log-timestamp">${timestamp}</span>
        <span class="log-level ${level}">${level.toUpperCase()}</span>
        <span class="log-message">${escapeHtml(message)}</span>
    `;
    
    // Remove empty state if present
    const emptyState = elements.logViewer.querySelector('.empty-state');
    if (emptyState) {
        emptyState.remove();
    }
    
    elements.logViewer.appendChild(logEntry);
    
    if (autoScroll) {
        elements.logViewer.scrollTop = elements.logViewer.scrollHeight;
    }
}

// Utility functions
function updateStatus(message) {
    elements.statusText.textContent = message;
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

function highlightMatches(text, matchCount) {
    // Simple highlighting - in a real implementation, you'd want more sophisticated highlighting
    return escapeHtml(text);
}

function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', init); 