/****************************************************************************
** Meta object code from reading C++ file 'httpapi.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../httpapi.h"
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'httpapi.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSHttpApiENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSHttpApiENDCLASS = QtMocHelpers::stringData(
    "HttpApi",
    "emptyToken",
    "",
    "available",
    "unavailable",
    "accountDataUpdated",
    "callsignsUpdated",
    "callsignStatus",
    "synced",
    "syncerror",
    "error",
    "QNetworkReply::NetworkError",
    "modesUpdated",
    "bandsUpdated",
    "HamDefsUploaded",
    "HamDefsError"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSHttpApiENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x06,    1 /* Public */,
       3,    0,   93,    2, 0x06,    2 /* Public */,
       4,    0,   94,    2, 0x06,    3 /* Public */,
       5,    0,   95,    2, 0x06,    4 /* Public */,
       6,    0,   96,    2, 0x06,    5 /* Public */,
       7,    1,   97,    2, 0x06,    6 /* Public */,
       8,    1,  100,    2, 0x06,    8 /* Public */,
       9,    1,  103,    2, 0x06,   10 /* Public */,
      10,    1,  106,    2, 0x06,   12 /* Public */,
      12,    0,  109,    2, 0x06,   14 /* Public */,
      13,    0,  110,    2, 0x06,   15 /* Public */,
      14,    0,  111,    2, 0x06,   16 /* Public */,
      15,    0,  112,    2, 0x06,   17 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, 0x80000000 | 11,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject HttpApi::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSHttpApiENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSHttpApiENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSHttpApiENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HttpApi, std::true_type>,
        // method 'emptyToken'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'available'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'unavailable'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'accountDataUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'callsignsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'callsignStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'synced'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'syncerror'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QNetworkReply::NetworkError, std::false_type>,
        // method 'modesUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'bandsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HamDefsUploaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HamDefsError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void HttpApi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HttpApi *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->emptyToken(); break;
        case 1: _t->available(); break;
        case 2: _t->unavailable(); break;
        case 3: _t->accountDataUpdated(); break;
        case 4: _t->callsignsUpdated(); break;
        case 5: _t->callsignStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->synced((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->syncerror((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->error((*reinterpret_cast< std::add_pointer_t<QNetworkReply::NetworkError>>(_a[1]))); break;
        case 9: _t->modesUpdated(); break;
        case 10: _t->bandsUpdated(); break;
        case 11: _t->HamDefsUploaded(); break;
        case 12: _t->HamDefsError(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply::NetworkError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::emptyToken; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::available; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::unavailable; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::accountDataUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::callsignsUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)(int );
            if (_t _q_method = &HttpApi::callsignStatus; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)(int );
            if (_t _q_method = &HttpApi::synced; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)(int );
            if (_t _q_method = &HttpApi::syncerror; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)(QNetworkReply::NetworkError );
            if (_t _q_method = &HttpApi::error; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::modesUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::bandsUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::HamDefsUploaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (HttpApi::*)();
            if (_t _q_method = &HttpApi::HamDefsError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
    }
}

const QMetaObject *HttpApi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HttpApi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSHttpApiENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int HttpApi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void HttpApi::emptyToken()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void HttpApi::available()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void HttpApi::unavailable()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void HttpApi::accountDataUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void HttpApi::callsignsUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void HttpApi::callsignStatus(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void HttpApi::synced(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void HttpApi::syncerror(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void HttpApi::error(QNetworkReply::NetworkError _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void HttpApi::modesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void HttpApi::bandsUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void HttpApi::HamDefsUploaded()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void HttpApi::HamDefsError()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}
QT_WARNING_POP
