/****************************************************************************
** Meta object code from reading C++ file 'LogDataWorker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/LogDataWorker.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogDataWorker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13LogDataWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto LogDataWorker::qt_create_metaobjectdata<qt_meta_tag_ZN13LogDataWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "LogDataWorker",
        "indexingProgressed",
        "",
        "percent",
        "indexingFinished",
        "success",
        "viewportUpdateRequested",
        "progressMessage",
        "message",
        "startIndexingRequested",
        "searchProgressed",
        "searchFinished",
        "resultCount",
        "startSearchRequested",
        "doIndexing",
        "doSearch"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'indexingProgressed'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'indexingFinished'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 },
        }}),
        // Signal 'viewportUpdateRequested'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressMessage'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'startIndexingRequested'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchProgressed'
        QtMocHelpers::SignalData<void(int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'searchFinished'
        QtMocHelpers::SignalData<void(bool, int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 }, { QMetaType::Int, 12 },
        }}),
        // Signal 'startSearchRequested'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'doIndexing'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'doSearch'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<LogDataWorker, qt_meta_tag_ZN13LogDataWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject LogDataWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13LogDataWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13LogDataWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13LogDataWorkerE_t>.metaTypes,
    nullptr
} };

void LogDataWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LogDataWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->indexingProgressed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->indexingFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->viewportUpdateRequested(); break;
        case 3: _t->progressMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->startIndexingRequested(); break;
        case 5: _t->searchProgressed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->searchFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->startSearchRequested(); break;
        case 8: _t->doIndexing(); break;
        case 9: _t->doSearch(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)(int )>(_a, &LogDataWorker::indexingProgressed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)(bool )>(_a, &LogDataWorker::indexingFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)()>(_a, &LogDataWorker::viewportUpdateRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)(const QString & )>(_a, &LogDataWorker::progressMessage, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)()>(_a, &LogDataWorker::startIndexingRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)(int )>(_a, &LogDataWorker::searchProgressed, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)(bool , int )>(_a, &LogDataWorker::searchFinished, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (LogDataWorker::*)()>(_a, &LogDataWorker::startSearchRequested, 7))
            return;
    }
}

const QMetaObject *LogDataWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogDataWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13LogDataWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LogDataWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void LogDataWorker::indexingProgressed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void LogDataWorker::indexingFinished(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void LogDataWorker::viewportUpdateRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LogDataWorker::progressMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void LogDataWorker::startIndexingRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void LogDataWorker::searchProgressed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void LogDataWorker::searchFinished(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void LogDataWorker::startSearchRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
