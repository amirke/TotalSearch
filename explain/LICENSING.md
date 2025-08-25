# TotalSearch Licensing Guide

## 🎯 **Licensing Strategy Options**

### **Option 1: Open Source (Recommended for Community Building)**

#### **MIT License** (Most Popular)
```text
MIT License

Copyright (c) 2024 TotalSearch

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

**Pros:**
- ✅ Very permissive and business-friendly
- ✅ Simple and widely understood
- ✅ Allows commercial use
- ✅ Minimal legal restrictions
- ✅ Great for community adoption

**Cons:**
- ❌ No revenue protection
- ❌ Others can create commercial versions
- ❌ No attribution requirements for modifications

#### **GPL v3 License** (Copyleft)
```text
GNU General Public License v3.0

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

**Pros:**
- ✅ Ensures derivatives remain open source
- ✅ Strong community protection
- ✅ Prevents proprietary forks

**Cons:**
- ❌ More restrictive for businesses
- ❌ Complex legal requirements
- ❌ May limit commercial adoption

---

### **Option 2: Dual Licensing (Commercial + Open Source)**

#### **Community Edition (GPL) + Commercial License**
- **Community Edition**: Free under GPL v3
- **Commercial Edition**: Paid license for proprietary use
- **Enterprise Features**: Advanced features only in commercial version

**Pros:**
- ✅ Revenue generation from commercial users
- ✅ Community benefits from open source version
- ✅ Flexibility for different user types

**Cons:**
- ❌ Complex to manage
- ❌ Potential for feature confusion
- ❌ Requires clear feature separation

---

### **Option 3: Freemium Model**

#### **Free Version + Premium Features**
- **Free**: Basic search functionality
- **Premium**: Advanced features, priority support, cloud sync

**Features by Tier:**
```
Free Version:
- Basic text search
- File viewer
- Standard highlighting
- Community support

Premium Version ($29/year):
- Advanced regex search
- Search history analytics
- Export to multiple formats
- Priority support
- Cloud search templates
- Batch operations
- Custom themes
```

**Pros:**
- ✅ Clear value proposition
- ✅ Low barrier to entry
- ✅ Scalable revenue model
- ✅ User choice

**Cons:**
- ❌ Feature gating complexity
- ❌ Potential user frustration
- ❌ Requires ongoing development

---

### **Option 4: Commercial Licensing**

#### **Perpetual License Model**
```
Personal License: $49 (one-time)
- Single user
- Personal use only
- Free updates for 1 year
- Community support

Professional License: $149 (one-time)
- Single user
- Commercial use
- Free updates for 2 years
- Email support
- Priority bug fixes

Enterprise License: $499 (one-time)
- Unlimited users within organization
- Commercial use
- Free updates for 3 years
- Phone support
- Custom features
- Training materials
```

**Pros:**
- ✅ Predictable revenue
- ✅ Clear pricing structure
- ✅ Professional positioning

**Cons:**
- ❌ Higher barrier to adoption
- ❌ Limited community growth
- ❌ Requires sales infrastructure

---

## 🛡️ **License Implementation**

### **1. License File Creation**
Create `LICENSE` file in project root:
```bash
# Choose appropriate license text from above
# Add copyright notice with current year
# Include any additional terms specific to your project
```

### **2. Source Code Headers**
Add to all source files:
```cpp
/*
 * TotalSearch - Advanced File Search Tool
 * Copyright (c) 2024 [Your Name/Company]
 * 
 * This file is part of TotalSearch.
 * 
 * TotalSearch is free software: you can redistribute it and/or modify
 * it under the terms of the [LICENSE NAME] as published by
 * the [LICENSE AUTHORITY], either version [VERSION] of the License, or
 * (at your option) any later version.
 * 
 * TotalSearch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * [LICENSE NAME] for more details.
 * 
 * You should have received a copy of the [LICENSE NAME] along with
 * TotalSearch.  If not, see <[LICENSE URL]>.
 */
```

### **3. Application License Display**
Add to About dialog:
```cpp
void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "About TotalSearch",
        "TotalSearch v1.0\n\n"
        "Advanced file search and text analysis tool\n\n"
        "Copyright (c) 2024 [Your Name/Company]\n"
        "Licensed under [LICENSE NAME]\n\n"
        "For more information, visit:\n"
        "https://github.com/yourusername/totalsearch");
}
```

### **4. Third-Party Licenses**
Create `THIRD_PARTY_LICENSES.md`:
```markdown
# Third-Party Licenses

TotalSearch uses the following third-party libraries:

## Qt Framework
- License: LGPL v3 / Commercial
- Website: https://www.qt.io/
- Used for: GUI framework

## Scintilla
- License: MIT License
- Website: https://www.scintilla.org/
- Used for: Text editing component

## Ripgrep
- License: MIT License
- Website: https://github.com/BurntSushi/ripgrep
- Used for: Fast text search engine

## [Add other dependencies...]
```

---

## 📋 **Recommended Approach**

### **For Maximum Adoption: MIT License**
1. **Start with MIT License** for community building
2. **Build user base** and gather feedback
3. **Consider dual licensing** later if commercial demand exists
4. **Focus on professional features** to differentiate from competitors

### **Implementation Steps:**
1. [ ] Choose license type (recommend MIT)
2. [ ] Create LICENSE file
3. [ ] Add copyright headers to source files
4. [ ] Update About dialog
5. [ ] Document third-party licenses
6. [ ] Add license information to README
7. [ ] Consider trademark registration for "TotalSearch"

### **Legal Considerations:**
- [ ] Consult with legal professional for final license choice
- [ ] Ensure compliance with third-party licenses
- [ ] Consider trademark protection
- [ ] Plan for license enforcement if needed

---

## 🎯 **Revenue Models (If Commercial)**

### **Option A: Donation-Based**
- GitHub Sponsors
- PayPal donations
- Patreon support

### **Option B: Support Services**
- Priority support ($99/year)
- Custom development ($150/hour)
- Training and consulting

### **Option C: Enterprise Features**
- Advanced analytics
- Team collaboration
- Cloud integration
- API access

---

*This guide provides general information only. Consult with a legal professional for specific licensing advice.*
