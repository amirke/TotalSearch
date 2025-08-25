/****************************************************************************
** Meta object code from reading C++ file 'scintillaedit.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/scintillaedit.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scintillaedit.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13ScintillaEditE_t {};
} // unnamed namespace

template <> constexpr inline auto ScintillaEdit::qt_create_metaobjectdata<qt_meta_tag_ZN13ScintillaEditE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScintillaEdit",
        "fileLoaded",
        "",
        "filePath",
        "fileLoadError",
        "error",
        "debugMessage",
        "message",
        "lineClicked",
        "lineNumber",
        "fileLineNumberChanged",
        "fileLineNumber",
        "verticalScrolled",
        "textChanged",
        "loadingProgress",
        "chunksLoaded",
        "totalChunks",
        "backgroundHighlightProgress",
        "percentage",
        "backgroundHighlightCompleted",
        "onDoubleClick",
        "Scintilla::Position",
        "position",
        "line",
        "onModified",
        "Scintilla::ModificationFlags",
        "type",
        "length",
        "linesAdded",
        "text",
        "Scintilla::FoldLevel",
        "foldNow",
        "foldPrev",
        "onScrolled",
        "onScrollStopped",
        "onBackgroundHighlightChunk",
        "onUserActivity",
        "onUserIdle"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'fileLoaded'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'fileLoadError'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'debugMessage'
        QtMocHelpers::SignalData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'lineClicked'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Signal 'fileLineNumberChanged'
        QtMocHelpers::SignalData<void(int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 11 },
        }}),
        // Signal 'verticalScrolled'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'textChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loadingProgress'
        QtMocHelpers::SignalData<void(int, int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 }, { QMetaType::Int, 16 },
        }}),
        // Signal 'backgroundHighlightProgress'
        QtMocHelpers::SignalData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Signal 'backgroundHighlightCompleted'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onDoubleClick'
        QtMocHelpers::SlotData<void(Scintilla::Position, Scintilla::Position)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 }, { 0x80000000 | 21, 23 },
        }}),
        // Slot 'onModified'
        QtMocHelpers::SlotData<void(Scintilla::ModificationFlags, Scintilla::Position, Scintilla::Position, Scintilla::Position, const QByteArray &, Scintilla::Position, Scintilla::FoldLevel, Scintilla::FoldLevel)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 25, 26 }, { 0x80000000 | 21, 22 }, { 0x80000000 | 21, 27 }, { 0x80000000 | 21, 28 },
            { QMetaType::QByteArray, 29 }, { 0x80000000 | 21, 23 }, { 0x80000000 | 30, 31 }, { 0x80000000 | 30, 32 },
        }}),
        // Slot 'onScrolled'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollStopped'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBackgroundHighlightChunk'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onUserActivity'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onUserIdle'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScintillaEdit, qt_meta_tag_ZN13ScintillaEditE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScintillaEdit::staticMetaObject = { {
    QMetaObject::SuperData::link<ScintillaEditBase::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScintillaEditE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScintillaEditE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13ScintillaEditE_t>.metaTypes,
    nullptr
} };

void ScintillaEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScintillaEdit *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->fileLoaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->fileLoadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->debugMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->lineClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->fileLineNumberChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->verticalScrolled(); break;
        case 6: _t->textChanged(); break;
        case 7: _t->loadingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->backgroundHighlightProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->backgroundHighlightCompleted(); break;
        case 10: _t->onDoubleClick((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[2]))); break;
        case 11: _t->onModified((*reinterpret_cast< std::add_pointer_t<Scintilla::ModificationFlags>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<Scintilla::FoldLevel>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<Scintilla::FoldLevel>>(_a[8]))); break;
        case 12: _t->onScrolled(); break;
        case 13: _t->onScrollStopped(); break;
        case 14: _t->onBackgroundHighlightChunk(); break;
        case 15: _t->onUserActivity(); break;
        case 16: _t->onUserIdle(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(const QString & )>(_a, &ScintillaEdit::fileLoaded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(const QString & )>(_a, &ScintillaEdit::fileLoadError, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(const QString & )>(_a, &ScintillaEdit::debugMessage, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(int )>(_a, &ScintillaEdit::lineClicked, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(int )>(_a, &ScintillaEdit::fileLineNumberChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)()>(_a, &ScintillaEdit::verticalScrolled, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)()>(_a, &ScintillaEdit::textChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(int , int )>(_a, &ScintillaEdit::loadingProgress, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)(int )>(_a, &ScintillaEdit::backgroundHighlightProgress, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEdit::*)()>(_a, &ScintillaEdit::backgroundHighlightCompleted, 9))
            return;
    }
}

const QMetaObject *ScintillaEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScintillaEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScintillaEditE_t>.strings))
        return static_cast<void*>(this);
    return ScintillaEditBase::qt_metacast(_clname);
}

int ScintillaEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ScintillaEditBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void ScintillaEdit::fileLoaded(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ScintillaEdit::fileLoadError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ScintillaEdit::debugMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ScintillaEdit::lineClicked(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ScintillaEdit::fileLineNumberChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void ScintillaEdit::verticalScrolled()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void ScintillaEdit::textChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ScintillaEdit::loadingProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}

// SIGNAL 8
void ScintillaEdit::backgroundHighlightProgress(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void ScintillaEdit::backgroundHighlightCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
