TEMPLATE = app
CONFIG += static
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

win32{
LIBS += -lws2_32
}

SOURCES += \
    $$_PRO_FILE_PWD_/src/aidcon.cpp \
    $$_PRO_FILE_PWD_/src/aid_client.cpp

HEADERS += \
    $$_PRO_FILE_PWD_/inc/aidcon.h

