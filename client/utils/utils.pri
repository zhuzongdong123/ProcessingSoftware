include(httpVisit/httpVisit.pri)
include(httpserver/httpserver.pri)
include(systemmessage/systemmessage.pri)
include(xlsx/qtxlsx.pri)

HEADERS += \
    $$PWD/base/batchdownloader.h \
    $$PWD/base/clickedlabel.h \
    $$PWD/base/gridlayout.h \
    $$PWD/base/mymask.h \
    $$PWD/base/myscrollarea.h \
    $$PWD/base/mysqlite.h \
    $$PWD/base/scaletextitem.h \
    $$PWD/base/widgetbase.h \
    $$PWD/downloadmanager/aimappingmanager.h \
    $$PWD/downloadmanager/downloadmanger.h

SOURCES += \
    $$PWD/base/batchdownloader.cpp \
    $$PWD/base/clickedlabel.cpp \
    $$PWD/base/gridlayout.cpp \
    $$PWD/base/mymask.cpp \
    $$PWD/base/myscrollarea.cpp \
    $$PWD/base/mysqlite.cpp \
    $$PWD/base/scaletextitem.cpp \
    $$PWD/base/widgetbase.cpp \
    $$PWD/downloadmanager/aimappingmanager.cpp \
    $$PWD/downloadmanager/downloadmanger.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/base
INCLUDEPATH += $$PWD/httpserver

FORMS += \
    $$PWD/base/gridlayout.ui