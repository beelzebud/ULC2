/****************************************************************************
** Meta object code from reading C++ file 'github_updater.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/github_updater.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'github_updater.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS = QtMocHelpers::stringData(
    "GitHubUpdater",
    "log",
    "",
    "msg",
    "progressMax",
    "max",
    "progressInc",
    "done",
    "updated",
    "newTag"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS_t {
    uint offsetsAndSizes[20];
    char stringdata0[14];
    char stringdata1[4];
    char stringdata2[1];
    char stringdata3[4];
    char stringdata4[12];
    char stringdata5[4];
    char stringdata6[12];
    char stringdata7[5];
    char stringdata8[8];
    char stringdata9[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS_t qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "GitHubUpdater"
        QT_MOC_LITERAL(14, 3),  // "log"
        QT_MOC_LITERAL(18, 0),  // ""
        QT_MOC_LITERAL(19, 3),  // "msg"
        QT_MOC_LITERAL(23, 11),  // "progressMax"
        QT_MOC_LITERAL(35, 3),  // "max"
        QT_MOC_LITERAL(39, 11),  // "progressInc"
        QT_MOC_LITERAL(51, 4),  // "done"
        QT_MOC_LITERAL(56, 7),  // "updated"
        QT_MOC_LITERAL(64, 6)   // "newTag"
    },
    "GitHubUpdater",
    "log",
    "",
    "msg",
    "progressMax",
    "max",
    "progressInc",
    "done",
    "updated",
    "newTag"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSGitHubUpdaterENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    1 /* Public */,
       4,    1,   41,    2, 0x06,    3 /* Public */,
       6,    0,   44,    2, 0x06,    5 /* Public */,
       7,    2,   45,    2, 0x06,    6 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    8,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject GitHubUpdater::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSGitHubUpdaterENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GitHubUpdater, std::true_type>,
        // method 'log'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'progressMax'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'progressInc'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'done'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void GitHubUpdater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GitHubUpdater *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->log((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->progressMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->progressInc(); break;
        case 3: _t->done((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GitHubUpdater::*)(const QString & );
            if (_t _q_method = &GitHubUpdater::log; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GitHubUpdater::*)(int );
            if (_t _q_method = &GitHubUpdater::progressMax; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GitHubUpdater::*)();
            if (_t _q_method = &GitHubUpdater::progressInc; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GitHubUpdater::*)(bool , const QString & );
            if (_t _q_method = &GitHubUpdater::done; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *GitHubUpdater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GitHubUpdater::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSGitHubUpdaterENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GitHubUpdater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void GitHubUpdater::log(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GitHubUpdater::progressMax(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GitHubUpdater::progressInc()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GitHubUpdater::done(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
