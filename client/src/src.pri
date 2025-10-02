FORMS += \
    $$PWD/login/loginwidget.ui \
    $$PWD/mainwindow/mainwindowwidget.ui

HEADERS += \
    $$PWD/common/appcommonbase.h \
    $$PWD/common/appconfigbase.h \
    $$PWD/common/appdatabasebase.h \
    $$PWD/common/appeventbase.h \
    $$PWD/login/loginwidget.h \
    $$PWD/mainwindow/mainwindowwidget.h

SOURCES += \
    $$PWD/common/appcommonbase.cpp \
    $$PWD/common/appconfigbase.cpp \
    $$PWD/common/appdatabasebase.cpp \
    $$PWD/common/appeventbase.cpp \
    $$PWD/login/loginwidget.cpp \
    $$PWD/mainwindow/mainwindowwidget.cpp

INCLUDEPATH += $$PWD\
$$PWD/common\
$$PWD/login\
$$PWD/mainwindow\
$$PWD/menubar\
$$PWD/titlebar\
$$PWD/workarea\