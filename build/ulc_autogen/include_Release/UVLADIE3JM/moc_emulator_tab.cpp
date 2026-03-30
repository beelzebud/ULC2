/****************************************************************************
** Meta object code from reading C++ file 'emulator_tab.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/emulator_tab.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'emulator_tab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
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
struct qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSEmulatorTabENDCLASS = QtMocHelpers::stringData(
    "EmulatorTab",
    "versionChanged",
    "",
    "stopOperation",
    "onUpdate",
    "onCheckForUpdate",
    "onBrowse",
    "appendLog",
    "msg",
    "setProgMax",
    "max",
    "incProgress",
    "onDone",
    "updated",
    "newTag"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t {
    uint offsetsAndSizes[30];
    char stringdata0[12];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[9];
    char stringdata5[17];
    char stringdata6[9];
    char stringdata7[10];
    char stringdata8[4];
    char stringdata9[11];
    char stringdata10[4];
    char stringdata11[12];
    char stringdata12[7];
    char stringdata13[8];
    char stringdata14[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t qt_meta_stringdata_CLASSEmulatorTabENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "EmulatorTab"
        QT_MOC_LITERAL(12, 14),  // "versionChanged"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 13),  // "stopOperation"
        QT_MOC_LITERAL(42, 8),  // "onUpdate"
        QT_MOC_LITERAL(51, 16),  // "onCheckForUpdate"
        QT_MOC_LITERAL(68, 8),  // "onBrowse"
        QT_MOC_LITERAL(77, 9),  // "appendLog"
        QT_MOC_LITERAL(87, 3),  // "msg"
        QT_MOC_LITERAL(91, 10),  // "setProgMax"
        QT_MOC_LITERAL(102, 3),  // "max"
        QT_MOC_LITERAL(106, 11),  // "incProgress"
        QT_MOC_LITERAL(118, 6),  // "onDone"
        QT_MOC_LITERAL(125, 7),  // "updated"
        QT_MOC_LITERAL(133, 6)   // "newTag"
    },
    "EmulatorTab",
    "versionChanged",
    "",
    "stopOperation",
    "onUpdate",
    "onCheckForUpdate",
    "onBrowse",
    "appendLog",
    "msg",
    "setProgMax",
    "max",
    "incProgress",
    "onDone",
    "updated",
    "newTag"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSEmulatorTabENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   69,    2, 0x0a,    2 /* Public */,
       4,    0,   70,    2, 0x08,    3 /* Private */,
       5,    0,   71,    2, 0x08,    4 /* Private */,
       6,    0,   72,    2, 0x08,    5 /* Private */,
       7,    1,   73,    2, 0x08,    6 /* Private */,
       9,    1,   76,    2, 0x08,    8 /* Private */,
      11,    0,   79,    2, 0x08,   10 /* Private */,
      12,    2,   80,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   13,   14,

       0        // eod
};

Q_CONSTINIT const QMetaObject EmulatorTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSEmulatorTabENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSEmulatorTabENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<EmulatorTab, std::true_type>,
        // method 'versionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopOperation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUpdate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCheckForUpdate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBrowse'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'appendLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setProgMax'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'incProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDone'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void EmulatorTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EmulatorTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->versionChanged(); break;
        case 1: _t->stopOperation(); break;
        case 2: _t->onUpdate(); break;
        case 3: _t->onCheckForUpdate(); break;
        case 4: _t->onBrowse(); break;
        case 5: _t->appendLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->setProgMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->incProgress(); break;
        case 8: _t->onDone((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EmulatorTab::*)();
            if (_t _q_method = &EmulatorTab::versionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *EmulatorTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EmulatorTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSEmulatorTabENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int EmulatorTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void EmulatorTab::versionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
