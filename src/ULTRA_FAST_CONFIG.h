#ifndef ULTRA_FAST_CONFIG_H
#define ULTRA_FAST_CONFIG_H

// Ultra-Fast Performance Configuration for Strong Computers
// Adjust these settings based on your system capabilities

// File I/O Optimizations
#define ULTRA_FAST_BLOCK_SIZE (100 * 1024 * 1024)  // 100MB blocks for ultra-fast indexing
#define ULTRA_FAST_BUFFER_SIZE (50 * 1024 * 1024)  // 50MB read buffers

// Logging Optimizations
#define ULTRA_FAST_LOGGING 1  // Enable ultra-minimal logging
#define LOG_EVERY_N_LINES 1000000  // Only log every 1M lines
#define LOG_PROGRESS_EVERY_PERCENT 100  // Only log at 100% completion

// Memory Optimizations
#define ULTRA_FAST_MEMORY_ALLOCATION 1  // Use larger memory chunks
#define PRE_ALLOCATE_LARGE_BUFFERS 1    // Pre-allocate buffers for large files

// Threading Optimizations
#define ULTRA_FAST_THREAD_COUNT 0       // Use all available CPU cores (0 = auto-detect)
#define ULTRA_FAST_ASYNC_LOADING 1      // Enable async file loading

// Search Optimizations
#define ULTRA_FAST_SEARCH_HISTORY 1     // Enable search history
#define MAX_SEARCH_HISTORY_ENTRIES 1000 // Store up to 1000 search history entries

// UI Optimizations
#define ULTRA_FAST_UI_UPDATES 1         // Reduce UI update frequency
#define BATCH_UI_UPDATES 1              // Batch UI updates for better performance

#endif // ULTRA_FAST_CONFIG_H
