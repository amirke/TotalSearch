/****************************************************************************
** Meta object code from reading C++ file 'ScintillaQt.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../scintilla/scintilla/qt/ScintillaEditBase/ScintillaQt.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScintillaQt.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t {};
} // unnamed namespace

template <> constexpr inline auto Scintilla::Internal::ScintillaQt::qt_create_metaobjectdata<qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Scintilla::Internal::ScintillaQt",
        "horizontalScrolled",
        "",
        "value",
        "verticalScrolled",
        "horizontalRangeChanged",
        "max",
        "page",
        "verticalRangeChanged",
        "notifyParent",
        "Scintilla::NotificationData",
        "scn",
        "notifyChange",
        "aboutToCopy",
        "QMimeData*",
        "data",
        "command",
        "Scintilla::uptr_t",
        "wParam",
        "Scintilla::sptr_t",
        "lParam",
        "onIdle",
        "execCommand",
        "QAction*",
        "action",
        "SelectionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'horizontalScrolled'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'verticalScrolled'
        QtMocHelpers::SignalData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'horizontalRangeChanged'
        QtMocHelpers::SignalData<void(int, int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'verticalRangeChanged'
        QtMocHelpers::SignalData<void(int, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 },
        }}),
        // Signal 'notifyParent'
        QtMocHelpers::SignalData<void(Scintilla::NotificationData)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Signal 'notifyChange'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'aboutToCopy'
        QtMocHelpers::SignalData<void(QMimeData *)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 14, 15 },
        }}),
        // Signal 'command'
        QtMocHelpers::SignalData<void(Scintilla::uptr_t, Scintilla::sptr_t)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 17, 18 }, { 0x80000000 | 19, 20 },
        }}),
        // Slot 'onIdle'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'execCommand'
        QtMocHelpers::SlotData<void(QAction *)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
        // Slot 'SelectionChanged'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScintillaQt, qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Scintilla::Internal::ScintillaQt::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>.metaTypes,
    nullptr
} };

void Scintilla::Internal::ScintillaQt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScintillaQt *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->horizontalScrolled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->verticalScrolled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->horizontalRangeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->verticalRangeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->notifyParent((*reinterpret_cast< std::add_pointer_t<Scintilla::NotificationData>>(_a[1]))); break;
        case 5: _t->notifyChange(); break;
        case 6: _t->aboutToCopy((*reinterpret_cast< std::add_pointer_t<QMimeData*>>(_a[1]))); break;
        case 7: _t->command((*reinterpret_cast< std::add_pointer_t<Scintilla::uptr_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::sptr_t>>(_a[2]))); break;
        case 8: _t->onIdle(); break;
        case 9: _t->execCommand((*reinterpret_cast< std::add_pointer_t<QAction*>>(_a[1]))); break;
        case 10: _t->SelectionChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAction* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(int )>(_a, &ScintillaQt::horizontalScrolled, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(int )>(_a, &ScintillaQt::verticalScrolled, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(int , int )>(_a, &ScintillaQt::horizontalRangeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(int , int )>(_a, &ScintillaQt::verticalRangeChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(Scintilla::NotificationData )>(_a, &ScintillaQt::notifyParent, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)()>(_a, &ScintillaQt::notifyChange, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(QMimeData * )>(_a, &ScintillaQt::aboutToCopy, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaQt::*)(Scintilla::uptr_t , Scintilla::sptr_t )>(_a, &ScintillaQt::command, 7))
            return;
    }
}

const QMetaObject *Scintilla::Internal::ScintillaQt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Scintilla::Internal::ScintillaQt::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9Scintilla8Internal11ScintillaQtE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "ScintillaBase"))
        return static_cast< ScintillaBase*>(this);
    return QObject::qt_metacast(_clname);
}

int Scintilla::Internal::ScintillaQt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void Scintilla::Internal::ScintillaQt::horizontalScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Scintilla::Internal::ScintillaQt::verticalScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void Scintilla::Internal::ScintillaQt::horizontalRangeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void Scintilla::Internal::ScintillaQt::verticalRangeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void Scintilla::Internal::ScintillaQt::notifyParent(Scintilla::NotificationData _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void Scintilla::Internal::ScintillaQt::notifyChange()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Scintilla::Internal::ScintillaQt::aboutToCopy(QMimeData * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void Scintilla::Internal::ScintillaQt::command(Scintilla::uptr_t _t1, Scintilla::sptr_t _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}
QT_WARNING_POP
