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
    "launchStateChanged",
    "running",
    "stopOperation",
    "onCheckRA",
    "onDownloadRA",
    "onCheckCores",
    "onDownloadCores",
    "onLaunchRA",
    "startCheckCores",
    "startDownloadCores",
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
    uint offsetsAndSizes[62];
    char stringdata0[13];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[21];
    char stringdata5[19];
    char stringdata6[10];
    char stringdata7[6];
    char stringdata8[20];
    char stringdata9[19];
    char stringdata10[8];
    char stringdata11[14];
    char stringdata12[10];
    char stringdata13[13];
    char stringdata14[13];
    char stringdata15[16];
    char stringdata16[11];
    char stringdata17[16];
    char stringdata18[19];
    char stringdata19[11];
    char stringdata20[14];
    char stringdata21[10];
    char stringdata22[4];
    char stringdata23[11];
    char stringdata24[4];
    char stringdata25[12];
    char stringdata26[13];
    char stringdata27[16];
    char stringdata28[10];
    char stringdata29[19];
    char stringdata30[12];
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
        QT_MOC_LITERAL(120, 18),  // "launchStateChanged"
        QT_MOC_LITERAL(139, 7),  // "running"
        QT_MOC_LITERAL(147, 13),  // "stopOperation"
        QT_MOC_LITERAL(161, 9),  // "onCheckRA"
        QT_MOC_LITERAL(171, 12),  // "onDownloadRA"
        QT_MOC_LITERAL(184, 12),  // "onCheckCores"
        QT_MOC_LITERAL(197, 15),  // "onDownloadCores"
        QT_MOC_LITERAL(213, 10),  // "onLaunchRA"
        QT_MOC_LITERAL(224, 15),  // "startCheckCores"
        QT_MOC_LITERAL(240, 18),  // "startDownloadCores"
        QT_MOC_LITERAL(259, 10),  // "onBrowseRA"
        QT_MOC_LITERAL(270, 13),  // "onBrowseCores"
        QT_MOC_LITERAL(284, 9),  // "appendLog"
        QT_MOC_LITERAL(294, 3),  // "msg"
        QT_MOC_LITERAL(298, 10),  // "setProgMax"
        QT_MOC_LITERAL(309, 3),  // "max"
        QT_MOC_LITERAL(313, 11),  // "incProgress"
        QT_MOC_LITERAL(325, 12),  // "onWorkerDone"
        QT_MOC_LITERAL(338, 15),  // "onRACheckResult"
        QT_MOC_LITERAL(354, 9),  // "latestTag"
        QT_MOC_LITERAL(364, 18),  // "onCoresCheckResult"
        QT_MOC_LITERAL(383, 11)   // "needsUpdate"
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
    "launchStateChanged",
    "running",
    "stopOperation",
    "onCheckRA",
    "onDownloadRA",
    "onCheckCores",
    "onDownloadCores",
    "onLaunchRA",
    "startCheckCores",
    "startDownloadCores",
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
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  140,    2, 0x06,    1 /* Public */,
       4,    0,  143,    2, 0x06,    3 /* Public */,
       5,    2,  144,    2, 0x06,    4 /* Public */,
       8,    0,  149,    2, 0x06,    7 /* Public */,
       9,    1,  150,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,  153,    2, 0x0a,   10 /* Public */,
      12,    0,  154,    2, 0x0a,   11 /* Public */,
      13,    0,  155,    2, 0x0a,   12 /* Public */,
      14,    0,  156,    2, 0x0a,   13 /* Public */,
      15,    0,  157,    2, 0x0a,   14 /* Public */,
      16,    0,  158,    2, 0x0a,   15 /* Public */,
      17,    0,  159,    2, 0x0a,   16 /* Public */,
      18,    0,  160,    2, 0x0a,   17 /* Public */,
      19,    0,  161,    2, 0x08,   18 /* Private */,
      20,    0,  162,    2, 0x08,   19 /* Private */,
      21,    1,  163,    2, 0x08,   20 /* Private */,
      23,    1,  166,    2, 0x08,   22 /* Private */,
      25,    0,  169,    2, 0x08,   24 /* Private */,
      26,    0,  170,    2, 0x08,   25 /* Private */,
      27,    2,  171,    2, 0x08,   26 /* Private */,
      29,    2,  176,    2, 0x08,   29 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    3,   28,
    QMetaType::Void, QMetaType::QStringList, QMetaType::Int,   30,    7,

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
        // method 'launchStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
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
        // method 'onLaunchRA'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startCheckCores'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'startDownloadCores'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
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
        case 4: _t->launchStateChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->stopOperation(); break;
        case 6: _t->onCheckRA(); break;
        case 7: _t->onDownloadRA(); break;
        case 8: _t->onCheckCores(); break;
        case 9: _t->onDownloadCores(); break;
        case 10: _t->onLaunchRA(); break;
        case 11: { bool _r = _t->startCheckCores();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { bool _r = _t->startDownloadCores();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 13: _t->onBrowseRA(); break;
        case 14: _t->onBrowseCores(); break;
        case 15: _t->appendLog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->setProgMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->incProgress(); break;
        case 18: _t->onWorkerDone(); break;
        case 19: _t->onRACheckResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: _t->onCoresCheckResult((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
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
        {
            using _t = void (RetroArchTab::*)(bool );
            if (_t _q_method = &RetroArchTab::launchStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
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
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 21;
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

// SIGNAL 4
void RetroArchTab::launchStateChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
