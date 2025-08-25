# Layout Switching Crash Issue - Analysis and Solutions

## Problem Description
The application crashes when switching between "Side by Side" and "Up/Down" layouts due to detachable pane parent reference issues.

## Root Cause
- Detachable panes store parent widget and layout references
- Layout changes disrupt these references
- Re-attaching panes causes crashes when references are invalid

## Current Solution
- Added user warnings in status bar
- Layout changes work for basic functionality
- Detachable panes may need app restart for full functionality

## Recommended Approach
1. Re-attach detached panes before layout change
2. Restart application after layout change
3. Use status bar warnings to inform users

## Future Improvements
- Implement safer parent reference handling
- Add confirmation dialogs before layout changes
- Consider alternative detachable pane approaches
