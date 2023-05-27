QT -= gui
QT += core
TEMPLATE = lib
DEFINES += LOG_LIBRARY

CONFIG += c++11
win32{
    CONFIG -= debug_and_release debug_and_release_target
    CONFIG += skip_target_version_ext
    DESTDIR = $$OUT_PWD/../../lib/
}
linux{
    DESTDIR = $$OUT_PWD/../../lib/
}
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Log.cpp

HEADERS += \
    Log.h \
    Log_global.h

MODULE = SanasoftLog
VERSION = 1.0.0


include(../lib_module.pri)
CONFIG -= c++1z
