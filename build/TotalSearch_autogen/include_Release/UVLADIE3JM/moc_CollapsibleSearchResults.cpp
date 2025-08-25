/****************************************************************************
** Meta object code from reading C++ file 'CollapsibleSearchResults.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/CollapsibleSearchResults.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CollapsibleSearchResults.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN24CollapsibleSearchResultsE_t {};
} // unnamed namespace

template <> constexpr inline auto CollapsibleSearchResults::qt_create_metaobjectdata<qt_meta_tag_ZN24CollapsibleSearchResultsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CollapsibleSearchResults",
        "resultSelected",
        "",
        "filePath",
        "lineNumber",
        "parsingStarted",
        "parsingProgress",
        "percentage",
        "files",
        "parsingCompleted",
        "totalMatches",
        "totalFiles",
        "parsingError",
        "error",
        "startParsing",
        "pattern",
        "jsonData",
        "searchPath",
        "querySearchStateFromMain",
        "onItemClicked",
        "QTreeWidgetItem*",
        "item",
        "column",
        "onItemExpanded",
        "onItemCollapsed",
        "onParsingStarted",
        "onParsingProgress",
        "onParsingCompleted",
        "onParsingError",
        "onFileItemCreated",
        "displayText",
        "onMatchItemCreated",
        "lineText",
        "onFileStatsUpdated",
        "elapsedTime",
        "matchedLines",
        "onSummaryParsed",
        "summaryText",
        "totalMatchedLines"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'resultSelected'
        QtMocHelpers::SignalData<void(const QString &, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 4 },
        }}),
        // Signal 'parsingStarted'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'parsingProgress'
        QtMocHelpers::SignalData<void(int, int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::Int, 8 },
        }}),
        // Signal 'parsingCompleted'
        QtMocHelpers::SignalData<void(int, int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 }, { QMetaType::Int, 11 },
        }}),
        // Signal 'parsingError'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'startParsing'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 }, { QMetaType::QString, 16 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'querySearchStateFromMain'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onItemClicked'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 }, { QMetaType::Int, 22 },
        }}),
        // Slot 'onItemExpanded'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
        // Slot 'onItemCollapsed'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
        // Slot 'onParsingStarted'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onParsingProgress'
        QtMocHelpers::SlotData<void(int, int)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'onParsingCompleted'
        QtMocHelpers::SlotData<void(int, int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 }, { QMetaType::Int, 11 },
        }}),
        // Slot 'onParsingError'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Slot 'onFileItemCreated'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 30 },
        }}),
        // Slot 'onMatchItemCreated'
        QtMocHelpers::SlotData<void(const QString &, int, const QString &)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 4 }, { QMetaType::QString, 32 },
        }}),
        // Slot 'onFileStatsUpdated'
        QtMocHelpers::SlotData<void(const QString &, const QString &, int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 34 }, { QMetaType::Int, 35 },
        }}),
        // Slot 'onSummaryParsed'
        QtMocHelpers::SlotData<void(const QString &, int)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 37 }, { QMetaType::Int, 38 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CollapsibleSearchResults, qt_meta_tag_ZN24CollapsibleSearchResultsE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CollapsibleSearchResults::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24CollapsibleSearchResultsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24CollapsibleSearchResultsE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN24CollapsibleSearchResultsE_t>.metaTypes,
    nullptr
} };

void CollapsibleSearchResults::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CollapsibleSearchResults *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->resultSelected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->parsingStarted(); break;
        case 2: _t->parsingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->parsingCompleted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->parsingError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->startParsing((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 6: _t->querySearchStateFromMain(); break;
        case 7: _t->onItemClicked((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->onItemExpanded((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1]))); break;
        case 9: _t->onItemCollapsed((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1]))); break;
        case 10: _t->onParsingStarted(); break;
        case 11: _t->onParsingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 12: _t->onParsingCompleted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 13: _t->onParsingError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->onFileItemCreated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 15: _t->onMatchItemCreated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 16: _t->onFileStatsUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 17: _t->onSummaryParsed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)(const QString & , int )>(_a, &CollapsibleSearchResults::resultSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)()>(_a, &CollapsibleSearchResults::parsingStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)(int , int )>(_a, &CollapsibleSearchResults::parsingProgress, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)(int , int )>(_a, &CollapsibleSearchResults::parsingCompleted, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)(const QString & )>(_a, &CollapsibleSearchResults::parsingError, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)(const QString & , const QString & , const QString & )>(_a, &CollapsibleSearchResults::startParsing, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollapsibleSearchResults::*)()>(_a, &CollapsibleSearchResults::querySearchStateFromMain, 6))
            return;
    }
}

const QMetaObject *CollapsibleSearchResults::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CollapsibleSearchResults::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24CollapsibleSearchResultsE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CollapsibleSearchResults::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void CollapsibleSearchResults::resultSelected(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void CollapsibleSearchResults::parsingStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CollapsibleSearchResults::parsingProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void CollapsibleSearchResults::parsingCompleted(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void CollapsibleSearchResults::parsingError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void CollapsibleSearchResults::startParsing(const QString & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3);
}

// SIGNAL 6
void CollapsibleSearchResults::querySearchStateFromMain()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
