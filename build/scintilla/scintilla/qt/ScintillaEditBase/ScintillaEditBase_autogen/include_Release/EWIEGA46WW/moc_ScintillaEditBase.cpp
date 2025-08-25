/****************************************************************************
** Meta object code from reading C++ file 'ScintillaEditBase.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../scintilla/scintilla/qt/ScintillaEditBase/ScintillaEditBase.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScintillaEditBase.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17ScintillaEditBaseE_t {};
} // unnamed namespace

template <> constexpr inline auto ScintillaEditBase::qt_create_metaobjectdata<qt_meta_tag_ZN17ScintillaEditBaseE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScintillaEditBase",
        "horizontalScrolled",
        "",
        "value",
        "verticalScrolled",
        "horizontalRangeChanged",
        "max",
        "page",
        "verticalRangeChanged",
        "notifyChange",
        "linesAdded",
        "Scintilla::Position",
        "aboutToCopy",
        "QMimeData*",
        "data",
        "styleNeeded",
        "position",
        "charAdded",
        "ch",
        "savePointChanged",
        "dirty",
        "modifyAttemptReadOnly",
        "key",
        "doubleClick",
        "line",
        "updateUi",
        "Scintilla::Update",
        "updated",
        "modified",
        "Scintilla::ModificationFlags",
        "type",
        "length",
        "text",
        "Scintilla::FoldLevel",
        "foldNow",
        "foldPrev",
        "macroRecord",
        "Scintilla::Message",
        "message",
        "Scintilla::uptr_t",
        "wParam",
        "Scintilla::sptr_t",
        "lParam",
        "marginClicked",
        "Scintilla::KeyMod",
        "modifiers",
        "margin",
        "textAreaClicked",
        "needShown",
        "painted",
        "userListSelection",
        "uriDropped",
        "uri",
        "dwellStart",
        "x",
        "y",
        "dwellEnd",
        "zoom",
        "hotSpotClick",
        "hotSpotDoubleClick",
        "callTipClick",
        "autoCompleteSelection",
        "autoCompleteCancelled",
        "focusChanged",
        "focused",
        "notify",
        "Scintilla::NotificationData*",
        "pscn",
        "command",
        "buttonPressed",
        "QMouseEvent*",
        "event",
        "buttonReleased",
        "keyPressed",
        "QKeyEvent*",
        "resized",
        "scrollHorizontal",
        "scrollVertical",
        "notifyParent",
        "Scintilla::NotificationData",
        "scn",
        "event_command"
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
        // Signal 'notifyChange'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'linesAdded'
        QtMocHelpers::SignalData<void(Scintilla::Position)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 10 },
        }}),
        // Signal 'aboutToCopy'
        QtMocHelpers::SignalData<void(QMimeData *)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Signal 'styleNeeded'
        QtMocHelpers::SignalData<void(Scintilla::Position)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 },
        }}),
        // Signal 'charAdded'
        QtMocHelpers::SignalData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Signal 'savePointChanged'
        QtMocHelpers::SignalData<void(bool)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 20 },
        }}),
        // Signal 'modifyAttemptReadOnly'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'key'
        QtMocHelpers::SignalData<void(int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 22 },
        }}),
        // Signal 'doubleClick'
        QtMocHelpers::SignalData<void(Scintilla::Position, Scintilla::Position)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { 0x80000000 | 11, 24 },
        }}),
        // Signal 'updateUi'
        QtMocHelpers::SignalData<void(Scintilla::Update)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Signal 'modified'
        QtMocHelpers::SignalData<void(Scintilla::ModificationFlags, Scintilla::Position, Scintilla::Position, Scintilla::Position, const QByteArray &, Scintilla::Position, Scintilla::FoldLevel, Scintilla::FoldLevel)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 30 }, { 0x80000000 | 11, 16 }, { 0x80000000 | 11, 31 }, { 0x80000000 | 11, 10 },
            { QMetaType::QByteArray, 32 }, { 0x80000000 | 11, 24 }, { 0x80000000 | 33, 34 }, { 0x80000000 | 33, 35 },
        }}),
        // Signal 'macroRecord'
        QtMocHelpers::SignalData<void(Scintilla::Message, Scintilla::uptr_t, Scintilla::sptr_t)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 37, 38 }, { 0x80000000 | 39, 40 }, { 0x80000000 | 41, 42 },
        }}),
        // Signal 'marginClicked'
        QtMocHelpers::SignalData<void(Scintilla::Position, Scintilla::KeyMod, int)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { 0x80000000 | 44, 45 }, { QMetaType::Int, 46 },
        }}),
        // Signal 'textAreaClicked'
        QtMocHelpers::SignalData<void(Scintilla::Position, int)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 24 }, { QMetaType::Int, 45 },
        }}),
        // Signal 'needShown'
        QtMocHelpers::SignalData<void(Scintilla::Position, Scintilla::Position)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { 0x80000000 | 11, 31 },
        }}),
        // Signal 'painted'
        QtMocHelpers::SignalData<void()>(49, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'userListSelection'
        QtMocHelpers::SignalData<void()>(50, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'uriDropped'
        QtMocHelpers::SignalData<void(const QString &)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 52 },
        }}),
        // Signal 'dwellStart'
        QtMocHelpers::SignalData<void(int, int)>(53, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 54 }, { QMetaType::Int, 55 },
        }}),
        // Signal 'dwellEnd'
        QtMocHelpers::SignalData<void(int, int)>(56, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 54 }, { QMetaType::Int, 55 },
        }}),
        // Signal 'zoom'
        QtMocHelpers::SignalData<void(int)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 57 },
        }}),
        // Signal 'hotSpotClick'
        QtMocHelpers::SignalData<void(Scintilla::Position, Scintilla::KeyMod)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { 0x80000000 | 44, 45 },
        }}),
        // Signal 'hotSpotDoubleClick'
        QtMocHelpers::SignalData<void(Scintilla::Position, Scintilla::KeyMod)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { 0x80000000 | 44, 45 },
        }}),
        // Signal 'callTipClick'
        QtMocHelpers::SignalData<void()>(60, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoCompleteSelection'
        QtMocHelpers::SignalData<void(Scintilla::Position, const QString &)>(61, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 16 }, { QMetaType::QString, 32 },
        }}),
        // Signal 'autoCompleteCancelled'
        QtMocHelpers::SignalData<void()>(62, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'focusChanged'
        QtMocHelpers::SignalData<void(bool)>(63, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 64 },
        }}),
        // Signal 'notify'
        QtMocHelpers::SignalData<void(Scintilla::NotificationData *)>(65, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 66, 67 },
        }}),
        // Signal 'command'
        QtMocHelpers::SignalData<void(Scintilla::uptr_t, Scintilla::sptr_t)>(68, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 39, 40 }, { 0x80000000 | 41, 42 },
        }}),
        // Signal 'buttonPressed'
        QtMocHelpers::SignalData<void(QMouseEvent *)>(69, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 70, 71 },
        }}),
        // Signal 'buttonReleased'
        QtMocHelpers::SignalData<void(QMouseEvent *)>(72, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 70, 71 },
        }}),
        // Signal 'keyPressed'
        QtMocHelpers::SignalData<void(QKeyEvent *)>(73, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 74, 71 },
        }}),
        // Signal 'resized'
        QtMocHelpers::SignalData<void()>(75, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'scrollHorizontal'
        QtMocHelpers::SlotData<void(int)>(76, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'scrollVertical'
        QtMocHelpers::SlotData<void(int)>(77, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'notifyParent'
        QtMocHelpers::SlotData<void(Scintilla::NotificationData)>(78, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 79, 80 },
        }}),
        // Slot 'event_command'
        QtMocHelpers::SlotData<void(Scintilla::uptr_t, Scintilla::sptr_t)>(81, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 39, 40 }, { 0x80000000 | 41, 42 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScintillaEditBase, qt_meta_tag_ZN17ScintillaEditBaseE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScintillaEditBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractScrollArea::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScintillaEditBaseE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScintillaEditBaseE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17ScintillaEditBaseE_t>.metaTypes,
    nullptr
} };

void ScintillaEditBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScintillaEditBase *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->horizontalScrolled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->verticalScrolled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->horizontalRangeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->verticalRangeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->notifyChange(); break;
        case 5: _t->linesAdded((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1]))); break;
        case 6: _t->aboutToCopy((*reinterpret_cast< std::add_pointer_t<QMimeData*>>(_a[1]))); break;
        case 7: _t->styleNeeded((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1]))); break;
        case 8: _t->charAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->savePointChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->modifyAttemptReadOnly(); break;
        case 11: _t->key((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->doubleClick((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[2]))); break;
        case 13: _t->updateUi((*reinterpret_cast< std::add_pointer_t<Scintilla::Update>>(_a[1]))); break;
        case 14: _t->modified((*reinterpret_cast< std::add_pointer_t<Scintilla::ModificationFlags>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<Scintilla::FoldLevel>>(_a[7])),(*reinterpret_cast< std::add_pointer_t<Scintilla::FoldLevel>>(_a[8]))); break;
        case 15: _t->macroRecord((*reinterpret_cast< std::add_pointer_t<Scintilla::Message>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::uptr_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<Scintilla::sptr_t>>(_a[3]))); break;
        case 16: _t->marginClicked((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::KeyMod>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 17: _t->textAreaClicked((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->needShown((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[2]))); break;
        case 19: _t->painted(); break;
        case 20: _t->userListSelection(); break;
        case 21: _t->uriDropped((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->dwellStart((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 23: _t->dwellEnd((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 24: _t->zoom((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->hotSpotClick((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::KeyMod>>(_a[2]))); break;
        case 26: _t->hotSpotDoubleClick((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::KeyMod>>(_a[2]))); break;
        case 27: _t->callTipClick(); break;
        case 28: _t->autoCompleteSelection((*reinterpret_cast< std::add_pointer_t<Scintilla::Position>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 29: _t->autoCompleteCancelled(); break;
        case 30: _t->focusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 31: _t->notify((*reinterpret_cast< std::add_pointer_t<Scintilla::NotificationData*>>(_a[1]))); break;
        case 32: _t->command((*reinterpret_cast< std::add_pointer_t<Scintilla::uptr_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::sptr_t>>(_a[2]))); break;
        case 33: _t->buttonPressed((*reinterpret_cast< std::add_pointer_t<QMouseEvent*>>(_a[1]))); break;
        case 34: _t->buttonReleased((*reinterpret_cast< std::add_pointer_t<QMouseEvent*>>(_a[1]))); break;
        case 35: _t->keyPressed((*reinterpret_cast< std::add_pointer_t<QKeyEvent*>>(_a[1]))); break;
        case 36: _t->resized(); break;
        case 37: _t->scrollHorizontal((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 38: _t->scrollVertical((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 39: _t->notifyParent((*reinterpret_cast< std::add_pointer_t<Scintilla::NotificationData>>(_a[1]))); break;
        case 40: _t->event_command((*reinterpret_cast< std::add_pointer_t<Scintilla::uptr_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Scintilla::sptr_t>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QMimeData* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int )>(_a, &ScintillaEditBase::horizontalScrolled, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int )>(_a, &ScintillaEditBase::verticalScrolled, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int , int )>(_a, &ScintillaEditBase::horizontalRangeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int , int )>(_a, &ScintillaEditBase::verticalRangeChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::notifyChange, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position )>(_a, &ScintillaEditBase::linesAdded, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(QMimeData * )>(_a, &ScintillaEditBase::aboutToCopy, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position )>(_a, &ScintillaEditBase::styleNeeded, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int )>(_a, &ScintillaEditBase::charAdded, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(bool )>(_a, &ScintillaEditBase::savePointChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::modifyAttemptReadOnly, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int )>(_a, &ScintillaEditBase::key, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , Scintilla::Position )>(_a, &ScintillaEditBase::doubleClick, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Update )>(_a, &ScintillaEditBase::updateUi, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::ModificationFlags , Scintilla::Position , Scintilla::Position , Scintilla::Position , const QByteArray & , Scintilla::Position , Scintilla::FoldLevel , Scintilla::FoldLevel )>(_a, &ScintillaEditBase::modified, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Message , Scintilla::uptr_t , Scintilla::sptr_t )>(_a, &ScintillaEditBase::macroRecord, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , Scintilla::KeyMod , int )>(_a, &ScintillaEditBase::marginClicked, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , int )>(_a, &ScintillaEditBase::textAreaClicked, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , Scintilla::Position )>(_a, &ScintillaEditBase::needShown, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::painted, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::userListSelection, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(const QString & )>(_a, &ScintillaEditBase::uriDropped, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int , int )>(_a, &ScintillaEditBase::dwellStart, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int , int )>(_a, &ScintillaEditBase::dwellEnd, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(int )>(_a, &ScintillaEditBase::zoom, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , Scintilla::KeyMod )>(_a, &ScintillaEditBase::hotSpotClick, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , Scintilla::KeyMod )>(_a, &ScintillaEditBase::hotSpotDoubleClick, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::callTipClick, 27))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::Position , const QString & )>(_a, &ScintillaEditBase::autoCompleteSelection, 28))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::autoCompleteCancelled, 29))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(bool )>(_a, &ScintillaEditBase::focusChanged, 30))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::NotificationData * )>(_a, &ScintillaEditBase::notify, 31))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(Scintilla::uptr_t , Scintilla::sptr_t )>(_a, &ScintillaEditBase::command, 32))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(QMouseEvent * )>(_a, &ScintillaEditBase::buttonPressed, 33))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(QMouseEvent * )>(_a, &ScintillaEditBase::buttonReleased, 34))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)(QKeyEvent * )>(_a, &ScintillaEditBase::keyPressed, 35))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScintillaEditBase::*)()>(_a, &ScintillaEditBase::resized, 36))
            return;
    }
}

const QMetaObject *ScintillaEditBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScintillaEditBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17ScintillaEditBaseE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractScrollArea::qt_metacast(_clname);
}

int ScintillaEditBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 41)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 41;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 41)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 41;
    }
    return _id;
}

// SIGNAL 0
void ScintillaEditBase::horizontalScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ScintillaEditBase::verticalScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ScintillaEditBase::horizontalRangeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void ScintillaEditBase::verticalRangeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void ScintillaEditBase::notifyChange()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ScintillaEditBase::linesAdded(Scintilla::Position _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ScintillaEditBase::aboutToCopy(QMimeData * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void ScintillaEditBase::styleNeeded(Scintilla::Position _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void ScintillaEditBase::charAdded(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void ScintillaEditBase::savePointChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void ScintillaEditBase::modifyAttemptReadOnly()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void ScintillaEditBase::key(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void ScintillaEditBase::doubleClick(Scintilla::Position _t1, Scintilla::Position _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2);
}

// SIGNAL 13
void ScintillaEditBase::updateUi(Scintilla::Update _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void ScintillaEditBase::modified(Scintilla::ModificationFlags _t1, Scintilla::Position _t2, Scintilla::Position _t3, Scintilla::Position _t4, const QByteArray & _t5, Scintilla::Position _t6, Scintilla::FoldLevel _t7, Scintilla::FoldLevel _t8)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1, _t2, _t3, _t4, _t5, _t6, _t7, _t8);
}

// SIGNAL 15
void ScintillaEditBase::macroRecord(Scintilla::Message _t1, Scintilla::uptr_t _t2, Scintilla::sptr_t _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1, _t2, _t3);
}

// SIGNAL 16
void ScintillaEditBase::marginClicked(Scintilla::Position _t1, Scintilla::KeyMod _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1, _t2, _t3);
}

// SIGNAL 17
void ScintillaEditBase::textAreaClicked(Scintilla::Position _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1, _t2);
}

// SIGNAL 18
void ScintillaEditBase::needShown(Scintilla::Position _t1, Scintilla::Position _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1, _t2);
}

// SIGNAL 19
void ScintillaEditBase::painted()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void ScintillaEditBase::userListSelection()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void ScintillaEditBase::uriDropped(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}

// SIGNAL 22
void ScintillaEditBase::dwellStart(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1, _t2);
}

// SIGNAL 23
void ScintillaEditBase::dwellEnd(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1, _t2);
}

// SIGNAL 24
void ScintillaEditBase::zoom(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1);
}

// SIGNAL 25
void ScintillaEditBase::hotSpotClick(Scintilla::Position _t1, Scintilla::KeyMod _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1, _t2);
}

// SIGNAL 26
void ScintillaEditBase::hotSpotDoubleClick(Scintilla::Position _t1, Scintilla::KeyMod _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1, _t2);
}

// SIGNAL 27
void ScintillaEditBase::callTipClick()
{
    QMetaObject::activate(this, &staticMetaObject, 27, nullptr);
}

// SIGNAL 28
void ScintillaEditBase::autoCompleteSelection(Scintilla::Position _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 28, nullptr, _t1, _t2);
}

// SIGNAL 29
void ScintillaEditBase::autoCompleteCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 29, nullptr);
}

// SIGNAL 30
void ScintillaEditBase::focusChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 30, nullptr, _t1);
}

// SIGNAL 31
void ScintillaEditBase::notify(Scintilla::NotificationData * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 31, nullptr, _t1);
}

// SIGNAL 32
void ScintillaEditBase::command(Scintilla::uptr_t _t1, Scintilla::sptr_t _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 32, nullptr, _t1, _t2);
}

// SIGNAL 33
void ScintillaEditBase::buttonPressed(QMouseEvent * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 33, nullptr, _t1);
}

// SIGNAL 34
void ScintillaEditBase::buttonReleased(QMouseEvent * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 34, nullptr, _t1);
}

// SIGNAL 35
void ScintillaEditBase::keyPressed(QKeyEvent * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 35, nullptr, _t1);
}

// SIGNAL 36
void ScintillaEditBase::resized()
{
    QMetaObject::activate(this, &staticMetaObject, 36, nullptr);
}
QT_WARNING_POP
