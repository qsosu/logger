/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSMainWindowENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSMainWindowENDCLASS = QtMocHelpers::stringData(
    "MainWindow",
    "onSettingsChanged",
    "",
    "CallsignToUppercase",
    "arg",
    "RefreshRecords",
    "SaveQso",
    "ClearQso",
    "UpdateFormDateTime",
    "onCallsignsUpdated",
    "onStationCallsignChanged",
    "onOperatorChanged",
    "onUdpLogged",
    "fillDefaultFreq",
    "customMenuRequested",
    "pos",
    "onQsoSynced",
    "dbid",
    "on_modeCombo_currentIndexChanged",
    "index",
    "LoadHamDefs",
    "setModesList",
    "setBandsList",
    "HamDefsUploaded",
    "HamDefsError"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMainWindowENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  128,    2, 0x08,    1 /* Private */,
       3,    1,  129,    2, 0x08,    2 /* Private */,
       5,    0,  132,    2, 0x08,    4 /* Private */,
       6,    0,  133,    2, 0x08,    5 /* Private */,
       7,    0,  134,    2, 0x08,    6 /* Private */,
       8,    0,  135,    2, 0x08,    7 /* Private */,
       9,    0,  136,    2, 0x08,    8 /* Private */,
      10,    0,  137,    2, 0x08,    9 /* Private */,
      11,    0,  138,    2, 0x08,   10 /* Private */,
      12,    0,  139,    2, 0x08,   11 /* Private */,
      13,    0,  140,    2, 0x08,   12 /* Private */,
      14,    1,  141,    2, 0x08,   13 /* Private */,
      16,    1,  144,    2, 0x08,   15 /* Private */,
      18,    1,  147,    2, 0x08,   17 /* Private */,
      20,    0,  150,    2, 0x08,   19 /* Private */,
      21,    0,  151,    2, 0x08,   20 /* Private */,
      22,    0,  152,    2, 0x08,   21 /* Private */,
      23,    0,  153,    2, 0x08,   22 /* Private */,
      24,    0,  154,    2, 0x08,   23 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   15,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSMainWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMainWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMainWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onSettingsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'CallsignToUppercase'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'RefreshRecords'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'SaveQso'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ClearQso'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'UpdateFormDateTime'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCallsignsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStationCallsignChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOperatorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUdpLogged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fillDefaultFreq'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'customMenuRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QPoint, std::false_type>,
        // method 'onQsoSynced'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_modeCombo_currentIndexChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'LoadHamDefs'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setModesList'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setBandsList'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HamDefsUploaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HamDefsError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSettingsChanged(); break;
        case 1: _t->CallsignToUppercase((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->RefreshRecords(); break;
        case 3: _t->SaveQso(); break;
        case 4: _t->ClearQso(); break;
        case 5: _t->UpdateFormDateTime(); break;
        case 6: _t->onCallsignsUpdated(); break;
        case 7: _t->onStationCallsignChanged(); break;
        case 8: _t->onOperatorChanged(); break;
        case 9: _t->onUdpLogged(); break;
        case 10: _t->fillDefaultFreq(); break;
        case 11: _t->customMenuRequested((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 12: _t->onQsoSynced((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->on_modeCombo_currentIndexChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->LoadHamDefs(); break;
        case 15: _t->setModesList(); break;
        case 16: _t->setBandsList(); break;
        case 17: _t->HamDefsUploaded(); break;
        case 18: _t->HamDefsError(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMainWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    return _id;
}
QT_WARNING_POP
