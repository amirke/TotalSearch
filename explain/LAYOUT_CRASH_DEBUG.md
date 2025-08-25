# Layout Switching Crash - Debug Analysis

## 🚨 **Issue Summary**
The application crashes when switching from "Side by Side" to "Up/Down" layout in the Configuration dialog.

## 🔍 **Debug Information Added**

### **Debug Logging Added**
- Added existence checks for `searchResultsPane` and `fileViewerPane`
- Added logging before detachable pane parent reference updates
- Added detailed logging in both `applyVerticalLayout()` and `applyHorizontalLayout()`

### **Debug Messages**
```
MainWindow: Debug - searchResultsPane exists: true/false
MainWindow: Debug - fileViewerPane exists: true/false
MainWindow: Debug - About to update detachable pane references
MainWindow: Debug - Updating searchResultsPane parent reference
MainWindow: Debug - Updating fileViewerPane parent reference
```

## 🐛 **Potential Crash Causes**

### **1. Detachable Pane Parent Reference Issues**
- **Problem**: Setting `setOriginalParent(widgets[1], nullptr)` with `nullptr` layout
- **Risk**: Detachable pane loses layout reference, causing crashes on re-attach
- **Evidence**: `DetachablePane::attachToMain()` requires valid `m_originalLayout`

### **2. Widget Lifecycle Problems**
- **Problem**: Widgets are moved between containers during layout change
- **Risk**: Parent-child relationships become invalid
- **Evidence**: `setParent(nullptr)` followed by `addWidget()` can cause issues

### **3. Memory Management Issues**
- **Problem**: Splitters are deleted and recreated
- **Risk**: Dangling pointers to deleted objects
- **Evidence**: `delete splitter` operations in layout functions

### **4. Signal/Slot Connection Problems**
- **Problem**: Detachable panes have signal/slot connections
- **Risk**: Connections become invalid when parent changes
- **Evidence**: Qt's parent-child relationship affects signal delivery

## 🔧 **Current Debug Strategy**

### **Step 1: Verify Pane Existence**
- Check if detachable panes exist before operations
- Log pane existence status

### **Step 2: Monitor Parent Reference Updates**
- Log when parent references are being updated
- Track which widgets are being assigned as parents

### **Step 3: Test Layout Switching**
- Try switching layouts and observe debug output
- Check for specific crash points in logs

## 🎯 **Recommended Debug Steps**

### **Immediate Actions**
1. **Run Application**: Start the app and check debug logs
2. **Test Layout Switch**: Try switching from Side by Side to Up/Down
3. **Monitor Logs**: Look for debug messages and crash points
4. **Check Pane State**: Verify if panes are detached when crash occurs

### **Advanced Debugging**
1. **Add More Logging**: Add logging in `DetachablePane::setOriginalParent()`
2. **Check Widget Validity**: Verify widgets before setting as parents
3. **Test Without Detachable Panes**: Temporarily disable detachable pane functionality
4. **Use Qt Debug Tools**: Enable Qt's debug output for widget operations

## 🛠️ **Potential Solutions**

### **Solution 1: Safe Parent Reference Handling**
```cpp
// Before setting parent reference, verify widget validity
if (searchResultsPane && widgets.size() >= 2 && widgets[1]) {
    // Verify widget is still valid
    if (widgets[1]->parent()) {
        LOG_INFO("MainWindow: Debug - Widget 1 has parent: " + widgets[1]->parent()->objectName());
    }
    searchResultsPane->setOriginalParent(widgets[1], nullptr);
}
```

### **Solution 2: Disable Detachable Panes During Layout Change**
```cpp
// Temporarily disable detachable pane functionality
if (searchResultsPane) {
    searchResultsPane->setEnabled(false);
}
if (fileViewerPane) {
    fileViewerPane->setEnabled(false);
}
// Perform layout change
// Re-enable after layout change
```

### **Solution 3: Force Re-attach Before Layout Change**
```cpp
// Force re-attach any detached panes before layout change
if (searchResultsPane && searchResultsPane->isDetached()) {
    searchResultsPane->attachToMain();
}
if (fileViewerPane && fileViewerPane->isDetached()) {
    fileViewerPane->attachToMain();
}
```

## 📋 **Next Steps**

### **Immediate**
1. **Test Current Debug Version**: Run app and try layout switching
2. **Collect Debug Logs**: Gather all debug output during crash
3. **Identify Crash Point**: Determine exact location of crash

### **Short-term**
1. **Implement Safe Handling**: Add widget validity checks
2. **Test Alternative Approaches**: Try different parent reference strategies
3. **Add Crash Prevention**: Implement graceful error handling

### **Long-term**
1. **Redesign Detachable Pane System**: Consider alternative approaches
2. **Improve Layout Management**: Implement more robust layout switching
3. **Add Comprehensive Testing**: Create automated tests for layout changes

## 🔍 **Debug Commands**

### **To Enable Qt Debug Output**
```bash
# Set environment variable for Qt debug output
set QT_LOGGING_RULES="qt.widgets.*=true"
```

### **To Monitor Application Logs**
- Check the application's log output during layout switching
- Look for debug messages starting with "MainWindow: Debug"
- Monitor for any error messages or warnings

---

*This debug analysis will be updated based on findings from testing.*
