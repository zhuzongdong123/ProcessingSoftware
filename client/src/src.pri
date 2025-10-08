FORMS += \
    $$PWD/workarea/datamanager/annotationdatapage.ui \
    $$PWD/workarea/datamanager/datamanager.ui \
    $$PWD/login/loginwidget.ui \
    $$PWD/mainwindow/mainwindowwidget.ui \
    $$PWD/menubar/menubar.ui \
    $$PWD/titlebar/titlebar.ui

HEADERS += \
    $$PWD/common/appcommonbase.h \
    $$PWD/common/appconfigbase.h \
    $$PWD/common/appdatabasebase.h \
    $$PWD/common/appeventbase.h \
    $$PWD/workarea/datamanager/annotationdatapage.h \
    $$PWD/workarea/datamanager/datamanager.h \
    $$PWD/login/loginwidget.h \
    $$PWD/mainwindow/mainwindowwidget.h \
    $$PWD/menubar/menubar.h \
    $$PWD/titlebar/titlebar.h \
    $$PWD/workarea/datamanager/imagepreviewwidget.h

SOURCES += \
    $$PWD/common/appcommonbase.cpp \
    $$PWD/common/appconfigbase.cpp \
    $$PWD/common/appdatabasebase.cpp \
    $$PWD/common/appeventbase.cpp \
    $$PWD/workarea/datamanager/annotationdatapage.cpp \
    $$PWD/workarea/datamanager/datamanager.cpp \
    $$PWD/login/loginwidget.cpp \
    $$PWD/mainwindow/mainwindowwidget.cpp \
    $$PWD/menubar/menubar.cpp \
    $$PWD/titlebar/titlebar.cpp \
    $$PWD/workarea/datamanager/imagepreviewwidget.cpp

INCLUDEPATH += $$PWD\
$$PWD/common\
$$PWD/login\
$$PWD/mainwindow\
$$PWD/menubar\
$$PWD/titlebar\
$$PWD/workarea\
$$PWD/workarea/datamanager\
$$PWD/workarea/sysmanager\
$$PWD/workarea/usermanager\