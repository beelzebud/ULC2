/****************************************************************************
** Meta object code from reading C++ file 'retroarch_tab.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/retroarch_tab.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'retroarch_tab.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSRetroArchTabENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSRetroArchTabENDCLASS = QtMocHelpers::stringData(
    "RetroArchTab",
    "binaryCheckFinished",
    "",
    "hasUpdate",
    "binaryUpdateFinished",
    "coresCheckFinished",
    "needCount",
    "total",
    "coresUpdateFinished",
    "stopOperation",
    "onCheckRA",
    "onDownloadRA",
    "onCheckCores",
    "onDownloadCores",
    "onBrowseRA",
    "onBrowseCores",
    "appendLog",
    "msg",
    "setProgMax",
    "max",
    "incProgress",
    "onWorkerDone",
    "onRACheckResult",
    "latestTag",
    "onCoresCheckResult",
    "needsUpdate"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSRetroArchTabENDCLASS_t {
    uint offsetsAndSizes[52];
    char stringdata0[13];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[21];
    char stringdata5[19];
    char stringdata6[10];
    char stringdata7[6];
    char stringdata8[20];
    char stringdata9[14];
    char stringdata10[10];
    char stringdata11[13];
    char stringdata12[13];
    char stringdata13[16];
    char stringdata14[11];
    char stringdata15[14];
    char stringdata16[10];
    char stringdata17[4];
    char stringdata18[11];
    char stringdata19[4];
    char stringdata20[12];
    char stringdata21[13];
    char stringdata22[16];
    char stringdata23[10];
    char stringdata24[19];
    char stringdata25[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSRetroArchTabENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSRetroArchTabENDCLASS_t qt_meta_stringdata_CLASSRetroArchTabENDCLASS = {
    {
        QT_MOC_LITERAL(0, 12),  // "RetroArchTab"
        QT_MOC_LITERAL(13, 19),  // "binaryCheckFinished"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 9),  // "hasUpdate"
        QT_MOC_LITERAL(44, 20),  // "binaryUpdateFinished"
        QT_MOC_LITERAL(65, 18),  // "coresCheckFinished"
        QT_MOC_LITERAL(84, 9),  // "needCount"
        QT_MOC_LITERAL(94, 5),  // "total"
        QT_MOC_LITERAL(100, 19),  // "coresUpdateFinished"
        QT_MOC_LITERAL(120, 13),  // "stopOperation"
        QT_MOC_LITERAL(134, 9),  // "onCheckRA"
        QT_MOC_LITERAL(144, 12),  // "onDownloadRA"
        QT_MOC_LITERAL(157, 12),  // "onCheckCores"
        QT_MOC_LITERAL(170, 15),  // "onDownloadCores"
        QT_MOC_LITERAL(186, 10),  // "onBrowseRA"
        QT_MOC_LITERAL(197, 13),  // "onBrowseCores"
        QT_MOC_LITERAL(211, 9),  // "appendLog"
        QT_MOC_LITERAL(221, 3),  // "msg"
        QT_MOC_LITERAL(225, 10),  // "setProgMax"
        QT_MOC_LITERAL(236, 3),  // "max"
        QT_MOC_LITERAL(240, 11),  // "incProgress"
        QT_MOC_LITERAL(252, 12),  // "onWorkerDone"
        QT_MOC_LITERAL(265, 15),  // "onRACheckResult"
        QT_MOC_LITERAL(281, 9),  // "latestTag"
        QT_MOC_LITERAL(291, 18),  // "onCoresCheckResult"
        QT_MOC_LITERAL(310, 11)   // "needsUpdate"
    },
    "RetroArchTab",
    "binaryCheckFinished",
    "",
    "hasUpdate",
    "binaryUpdateFinished",
    "coresCheckFinished",
    "needCount",
    "total",
    "coresUpdateFinished",
    "stopOperation",
    "onCheckRA",
    "onDownloadRA",
    "onCheckCores",
    "onDownloadCores",
    "onBrowseRA",
    "onBrowseCores",
    "appendLog",
    "msg",
    "setProgMax",
    "max",
    "incProgress",
    "onWorkerDone",
    "onRACheckResult",
    "latestTag",
    "onCoresCheckResult",
    "needsUpdate"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSRetroArchTabENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  116,    2, 0x06,    1 /* Public */,
       4,    0,  119,    2, 0x06,    3 /* Public */,
       5,    2,  120,    2, 0x06,    4 /* Public */,
       8,    0,  125,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    0,  126,    2, 0x0a,    8 /* Public */,
      10,    0,  127,    2, 0x0a,    9 /* Public */,
      11,    0,  128,    2, 0x0a,   10 /* Public */,
      12,    0,  129,    2, 0x0a,   11 /* Public */,
      13,    0,  130,    2, 0x0a,   12 /* Public */,
      14,    0,  131,    2, 0x08,   13 /* Private */,
      15,    0,  132,    2, 0x08,   14 /* Private */,
      16,    1,  133,    2, 0x08,   15 /* Private */,
      18,    1,  136,    2, 0x08,   17 /* Private */,
      20,    0,  139,    2, 0x08,   19 /* Private */,
      21,    0,  140,    2, 0x08,   20 /* Private */,
      22,    2,  141,    2, 0x08,   21 /* Private */,
      24,    2,  146,    2, 0x08,   24 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    7,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    3,   23,
    QMetaType::Void, QMetaType::QStringList, QMetaType::Int,   25,    7,

       0        // eod
};

Q_CONSTINIT const QMetaObject RetroArchTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSRetroArchTabENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSRetroArchTabENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSRetroArchTabENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RetroArchTab, std::true_type>,
        // method 'binaryCheckFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'binaryUpdateFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'coresCheckFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'coresUpdateFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopOperation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCheckRA'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDownloadRA'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCheckCores'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDownloadCores'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBrowseRA'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBrowseCores'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'appendLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setProgMax'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'incProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWorkerDone'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRACheckResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onCoresCheckResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void RetroArchTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RetroArchTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->binaryCheckFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->binaryUpdateFinished(); break;
        case 2: _t->coresCheckFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->coresUpdateFinished(); break;
        case 4: _t->stopOperation(); break;
        case 5: _t->onCheckRA(); break;
        case 6: _t->onDownloadRA(); break;
        case 7: _t->onCheckCores(); break;
        case 8: _t->onDownloadCores(); break;
        case 9: _t->onBrowseRA(); break;
        case 10: _t->onBrowseCores(); break;
        case 11: _t->appendLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->setProgMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->incProgress(); break;
        case 14: _t->onWorkerDone(); break;
        case 15: _t->onRACheckResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 16: _t->onCoresCheckResult((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RetroArchTab::*)(bool );
            if (_t _q_method = &RetroArchTab::binaryCheckFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (RetroArchTab::*)();
            if (_t _q_method = &RetroArchTab::binaryUpdateFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (RetroArchTab::*)(int , int );
            if (_t _q_method = &RetroArchTab::coresCheckFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (RetroArchTab::*)();
            if (_t _q_method = &RetroArchTab::coresUpdateFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *RetroArchTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RetroArchTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSRetroArchTabENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RetroArchTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void RetroArchTab::binaryCheckFinished(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RetroArchTab::binaryUpdateFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void RetroArchTab::coresCheckFinished(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RetroArchTab::coresUpdateFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
