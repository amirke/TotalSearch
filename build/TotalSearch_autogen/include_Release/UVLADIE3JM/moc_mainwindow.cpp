/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "sequentialReadCompleted",
        "",
        "fileContent",
        "sequentialReadError",
        "error",
        "openFile2",
        "KFindInFile",
        "KKMap",
        "KClockTest",
        "KTestPane",
        "KDisplayFile_BulkRead",
        "KDisplayFile_SequentialRead_Optimized",
        "KDisplayFile_SequentialRead_Sync",
        "KScrollToLine",
        "lineNumber",
        "openFile",
        "saveFile",
        "searchText",
        "stopSearch",
        "clearSearch",
        "updateScrollPosition",
        "value",
        "showPreferences",
        "showSearchParameters",
        "loadSettings",
        "saveToSearchHistory",
        "searchTerm",
        "saveToFolderHistory",
        "folderPath",
        "getSearchHistory",
        "getFolderHistory",
        "setupAutoComplete",
        "addToPatternHistory",
        "pattern",
        "addToPathHistory",
        "path",
        "updatePatternCompleter",
        "updatePathCompleter",
        "saveHistoryToFile",
        "loadHistoryFromFile",
        "saveSettings",
        "searchResultsPosition",
        "showLogWindow",
        "searchEngine",
        "caseSensitive",
        "useRegex",
        "wholeWord",
        "ignoreHidden",
        "followSymlinks",
        "fileTypes",
        "maxDepth",
        "excludePatterns",
        "smartCase",
        "multiline",
        "dotAll",
        "noIgnore",
        "tabWidth",
        "useSpacesForTabs",
        "contextLines",
        "highlightColor",
        "chunkSize",
        "firstChunkSize",
        "showLineNumbers",
        "saveFindInFileSettings",
        "engine",
        "type",
        "inverse",
        "boolean",
        "plainText",
        "autoRefresh",
        "startLine",
        "endLine",
        "applyFont",
        "font",
        "applyLayoutSettings",
        "applyViewerSettings",
        "onSearchResultSelected",
        "QListWidgetItem*",
        "item",
        "performRipgrepSearch",
        "startTime",
        "performBuiltinSearch",
        "logDebugMessage",
        "message",
        "onFileLoaded",
        "filePath",
        "onScintillaFileLoadError",
        "onFileLineNumberChanged",
        "fileLine",
        "onLoadingProgress",
        "chunksLoaded",
        "totalChunks",
        "onCollapsibleResultSelected"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sequentialReadCompleted'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'sequentialReadError'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'openFile2'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KFindInFile'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KKMap'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KClockTest'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KTestPane'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KDisplayFile_BulkRead'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KDisplayFile_SequentialRead_Optimized'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KDisplayFile_SequentialRead_Sync'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'KScrollToLine'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Slot 'KScrollToLine'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void),
        // Slot 'openFile'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveFile'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'searchText'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'stopSearch'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'clearSearch'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateScrollPosition'
        QtMocHelpers::SlotData<void(int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Slot 'showPreferences'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showSearchParameters'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'loadSettings'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveToSearchHistory'
        QtMocHelpers::SlotData<void(const QString &)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 27 },
        }}),
        // Slot 'saveToFolderHistory'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Slot 'getSearchHistory'
        QtMocHelpers::SlotData<QStringList()>(30, 2, QMC::AccessPrivate, QMetaType::QStringList),
        // Slot 'getFolderHistory'
        QtMocHelpers::SlotData<QStringList()>(31, 2, QMC::AccessPrivate, QMetaType::QStringList),
        // Slot 'setupAutoComplete'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addToPatternHistory'
        QtMocHelpers::SlotData<void(const QString &)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 34 },
        }}),
        // Slot 'addToPathHistory'
        QtMocHelpers::SlotData<void(const QString &)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 36 },
        }}),
        // Slot 'updatePatternCompleter'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updatePathCompleter'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveHistoryToFile'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'loadHistoryFromFile'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveSettings'
        QtMocHelpers::SlotData<void(const QString &, bool, const QString &, bool, bool, bool, bool, bool, const QString &, int, const QString &, bool, bool, bool, bool, int, bool, int, QColor, int, int, bool)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 42 }, { QMetaType::Bool, 43 }, { QMetaType::QString, 44 }, { QMetaType::Bool, 45 },
            { QMetaType::Bool, 46 }, { QMetaType::Bool, 47 }, { QMetaType::Bool, 48 }, { QMetaType::Bool, 49 },
            { QMetaType::QString, 50 }, { QMetaType::Int, 51 }, { QMetaType::QString, 52 }, { QMetaType::Bool, 53 },
            { QMetaType::Bool, 54 }, { QMetaType::Bool, 55 }, { QMetaType::Bool, 56 }, { QMetaType::Int, 57 },
            { QMetaType::Bool, 58 }, { QMetaType::Int, 59 }, { QMetaType::QColor, 60 }, { QMetaType::Int, 61 },
            { QMetaType::Int, 62 }, { QMetaType::Bool, 63 },
        }}),
        // Slot 'saveFindInFileSettings'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &, bool, bool, bool, bool, bool, int, int, QColor)>(64, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 34 }, { QMetaType::QString, 65 }, { QMetaType::QString, 66 }, { QMetaType::Bool, 45 },
            { QMetaType::Bool, 67 }, { QMetaType::Bool, 68 }, { QMetaType::Bool, 69 }, { QMetaType::Bool, 70 },
            { QMetaType::Int, 71 }, { QMetaType::Int, 72 }, { QMetaType::QColor, 60 },
        }}),
        // Slot 'applyFont'
        QtMocHelpers::SlotData<void(const QFont &)>(73, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QFont, 74 },
        }}),
        // Slot 'applyLayoutSettings'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'applyViewerSettings'
        QtMocHelpers::SlotData<void()>(76, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSearchResultSelected'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(77, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 78, 79 },
        }}),
        // Slot 'performRipgrepSearch'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(80, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { QMetaType::QString, 81 },
        }}),
        // Slot 'performBuiltinSearch'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(82, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { QMetaType::QString, 81 },
        }}),
        // Slot 'logDebugMessage'
        QtMocHelpers::SlotData<void(const QString &)>(83, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 84 },
        }}),
        // Slot 'onFileLoaded'
        QtMocHelpers::SlotData<void(const QString &)>(85, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 86 },
        }}),
        // Slot 'onScintillaFileLoadError'
        QtMocHelpers::SlotData<void(const QString &)>(87, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Slot 'onFileLineNumberChanged'
        QtMocHelpers::SlotData<void(int)>(88, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 89 },
        }}),
        // Slot 'onLoadingProgress'
        QtMocHelpers::SlotData<void(int, int)>(90, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 91 }, { QMetaType::Int, 92 },
        }}),
        // Slot 'onCollapsibleResultSelected'
        QtMocHelpers::SlotData<void(const QString &, int)>(93, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 86 }, { QMetaType::Int, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sequentialReadCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->sequentialReadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->openFile2(); break;
        case 3: _t->KFindInFile(); break;
        case 4: _t->KKMap(); break;
        case 5: _t->KClockTest(); break;
        case 6: _t->KTestPane(); break;
        case 7: _t->KDisplayFile_BulkRead(); break;
        case 8: _t->KDisplayFile_SequentialRead_Optimized(); break;
        case 9: _t->KDisplayFile_SequentialRead_Sync(); break;
        case 10: _t->KScrollToLine((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->KScrollToLine(); break;
        case 12: _t->openFile(); break;
        case 13: _t->saveFile(); break;
        case 14: _t->searchText(); break;
        case 15: _t->stopSearch(); break;
        case 16: _t->clearSearch(); break;
        case 17: _t->updateScrollPosition((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->showPreferences(); break;
        case 19: _t->showSearchParameters(); break;
        case 20: _t->loadSettings(); break;
        case 21: _t->saveToSearchHistory((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->saveToFolderHistory((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: { QStringList _r = _t->getSearchHistory();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 24: { QStringList _r = _t->getFolderHistory();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 25: _t->setupAutoComplete(); break;
        case 26: _t->addToPatternHistory((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 27: _t->addToPathHistory((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: _t->updatePatternCompleter(); break;
        case 29: _t->updatePathCompleter(); break;
        case 30: _t->saveHistoryToFile(); break;
        case 31: _t->loadHistoryFromFile(); break;
        case 32: _t->saveSettings((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[9])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[10])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[11])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[12])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[13])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[14])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[15])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[16])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[17])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[18])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[19])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[20])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[21])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[22]))); break;
        case 33: _t->saveFindInFileSettings((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[8])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[9])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[10])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[11]))); break;
        case 34: _t->applyFont((*reinterpret_cast< std::add_pointer_t<QFont>>(_a[1]))); break;
        case 35: _t->applyLayoutSettings(); break;
        case 36: _t->applyViewerSettings(); break;
        case 37: _t->onSearchResultSelected((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 38: _t->performRipgrepSearch((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 39: _t->performBuiltinSearch((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 40: _t->logDebugMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 41: _t->onFileLoaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 42: _t->onScintillaFileLoadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 43: _t->onFileLineNumberChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 44: _t->onLoadingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 45: _t->onCollapsibleResultSelected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MainWindow::*)(const QString & )>(_a, &MainWindow::sequentialReadCompleted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MainWindow::*)(const QString & )>(_a, &MainWindow::sequentialReadError, 1))
            return;
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 46)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 46;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 46)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 46;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::sequentialReadCompleted(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void MainWindow::sequentialReadError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
