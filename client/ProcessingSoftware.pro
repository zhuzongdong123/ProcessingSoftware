QT       += core gui
QT       += network sql
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \

include(src/src.pri)
include(utils/utils.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    app.rc \
    tubiao.ico
RC_FILE += app.rc
RESOURCES += \
    myicon.qrc

#临时文件存放位置
MOC_DIR         = ./temp/moc
UI_DIR          = ./temp/UI
OBJECTS_DIR     = ./temp/obj

#LIBS += -lDbgHelp
DEFINES += QT_MESSAGELOGCONTEXT
#QMAKE_CXXFLAGS_EXCEPTIONS_ON = /EHa
#QMAKE_CXXFLAGS_STL_ON = /EHa
# 生成pdb文件
#QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG

msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

include(httpVisit/httpVisit.pri)
include(common/common.pri)
