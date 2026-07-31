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
    "checkComplete",
    "hasUpdate",
    "updateComplete",
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
    uint offsetsAndSizes[36];
    char stringdata0[12];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[10];
    char stringdata5[15];
    char stringdata6[14];
    char stringdata7[9];
    char stringdata8[17];
    char stringdata9[9];
    char stringdata10[10];
    char stringdata11[4];
    char stringdata12[11];
    char stringdata13[4];
    char stringdata14[12];
    char stringdata15[7];
    char stringdata16[8];
    char stringdata17[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSEmulatorTabENDCLASS_t qt_meta_stringdata_CLASSEmulatorTabENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "EmulatorTab"
        QT_MOC_LITERAL(12, 14),  // "versionChanged"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 13),  // "checkComplete"
        QT_MOC_LITERAL(42, 9),  // "hasUpdate"
        QT_MOC_LITERAL(52, 14),  // "updateComplete"
        QT_MOC_LITERAL(67, 13),  // "stopOperation"
        QT_MOC_LITERAL(81, 8),  // "onUpdate"
        QT_MOC_LITERAL(90, 16),  // "onCheckForUpdate"
        QT_MOC_LITERAL(107, 8),  // "onBrowse"
        QT_MOC_LITERAL(116, 9),  // "appendLog"
        QT_MOC_LITERAL(126, 3),  // "msg"
        QT_MOC_LITERAL(130, 10),  // "setProgMax"
        QT_MOC_LITERAL(141, 3),  // "max"
        QT_MOC_LITERAL(145, 11),  // "incProgress"
        QT_MOC_LITERAL(157, 6),  // "onDone"
        QT_MOC_LITERAL(164, 7),  // "updated"
        QT_MOC_LITERAL(172, 6)   // "newTag"
    },
    "EmulatorTab",
    "versionChanged",
    "",
    "checkComplete",
    "hasUpdate",
    "updateComplete",
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
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    1 /* Public */,
       3,    1,   81,    2, 0x06,    2 /* Public */,
       5,    0,   84,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   85,    2, 0x0a,    5 /* Public */,
       7,    0,   86,    2, 0x0a,    6 /* Public */,
       8,    0,   87,    2, 0x0a,    7 /* Public */,
       9,    0,   88,    2, 0x08,    8 /* Private */,
      10,    1,   89,    2, 0x08,    9 /* Private */,
      12,    1,   92,    2, 0x08,   11 /* Private */,
      14,    0,   95,    2, 0x08,   13 /* Private */,
      15,    2,   96,    2, 0x08,   14 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   16,   17,

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
        // method 'checkComplete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'updateComplete'
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
        case 1: _t->checkComplete((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->updateComplete(); break;
        case 3: _t->stopOperation(); break;
        case 4: _t->onUpdate(); break;
        case 5: _t->onCheckForUpdate(); break;
        case 6: _t->onBrowse(); break;
        case 7: _t->appendLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->setProgMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->incProgress(); break;
        case 10: _t->onDone((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
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
        {
            using _t = void (EmulatorTab::*)(bool );
            if (_t _q_method = &EmulatorTab::checkComplete; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (EmulatorTab::*)();
            if (_t _q_method = &EmulatorTab::updateComplete; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
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
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void EmulatorTab::versionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void EmulatorTab::checkComplete(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void EmulatorTab::updateComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
