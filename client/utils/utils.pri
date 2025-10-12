include(httpVisit/httpVisit.pri)
include(systemmessage/systemmessage.pri)

HEADERS += \
    $$PWD/base/batchdownloader.h \
    $$PWD/base/clickedlabel.h \
    $$PWD/base/gridlayout.h \
    $$PWD/base/mymask.h \
    $$PWD/base/myscrollarea.h \
    $$PWD/base/mysqlite.h \
    $$PWD/base/scaletextitem.h \
    $$PWD/base/widgetbase.h

SOURCES += \
    $$PWD/base/batchdownloader.cpp \
    $$PWD/base/clickedlabel.cpp \
    $$PWD/base/gridlayout.cpp \
    $$PWD/base/mymask.cpp \
    $$PWD/base/myscrollarea.cpp \
    $$PWD/base/mysqlite.cpp \
    $$PWD/base/scaletextitem.cpp \
    $$PWD/base/widgetbase.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/base

FORMS += \
    $$PWD/base/gridlayout.ui