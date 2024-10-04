/****************************************************************************
** Meta object code from reading C++ file 'callsigns.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../callsigns.h"
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'callsigns.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.2. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCallsignsENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCallsignsENDCLASS = QtMocHelpers::stringData(
    "Callsigns",
    "updated",
    "",
    "onCallsignsUpdated",
    "onAddPressed",
    "onRemovePressed",
    "onSavePressed",
    "onCancelPressed",
    "requestQsosu",
    "addCallsigng",
    "on_checkCallsignBtn_clicked",
    "callsignStatus",
    "status"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCallsignsENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   75,    2, 0x0a,    2 /* Public */,
       4,    0,   76,    2, 0x08,    3 /* Private */,
       5,    0,   77,    2, 0x08,    4 /* Private */,
       6,    0,   78,    2, 0x08,    5 /* Private */,
       7,    0,   79,    2, 0x08,    6 /* Private */,
       8,    0,   80,    2, 0x08,    7 /* Private */,
       9,    0,   81,    2, 0x08,    8 /* Private */,
      10,    0,   82,    2, 0x08,    9 /* Private */,
      11,    1,   83,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,

       0        // eod
};

Q_CONSTINIT const QMetaObject Callsigns::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSCallsignsENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCallsignsENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCallsignsENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Callsigns, std::true_type>,
        // method 'updated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCallsignsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAddPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRemovePressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSavePressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestQsosu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'addCallsigng'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_checkCallsignBtn_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'callsignStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void Callsigns::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Callsigns *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updated(); break;
        case 1: _t->onCallsignsUpdated(); break;
        case 2: _t->onAddPressed(); break;
        case 3: _t->onRemovePressed(); break;
        case 4: _t->onSavePressed(); break;
        case 5: _t->onCancelPressed(); break;
        case 6: _t->requestQsosu(); break;
        case 7: _t->addCallsigng(); break;
        case 8: _t->on_checkCallsignBtn_clicked(); break;
        case 9: _t->callsignStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Callsigns::*)();
            if (_t _q_method = &Callsigns::updated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *Callsigns::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Callsigns::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCallsignsENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int Callsigns::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Callsigns::updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
