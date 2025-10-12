INCLUDEPATH += $$PWD/dbpool
INCLUDEPATH += $$PWD/formoptions/systemsetting
INCLUDEPATH += $$PWD
HEADERS += \
    $$PWD/dbpool/dao.h \
    $$PWD/dbpool/dbpool.h \
    $$PWD/formoptions/systemsetting/annotitiondao.h \
    $$PWD/formoptions/systemsetting/identitydao.h

SOURCES += \
    $$PWD/dbpool/dao.cpp \
    $$PWD/dbpool/dbpool.cpp \
    $$PWD/formoptions/systemsetting/annotitiondao.cpp \
    $$PWD/formoptions/systemsetting/identitydao.cpp
