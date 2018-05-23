include(../../../qmakeroot.pri)
TEMPLATE = lib
HEADERS += opengl_helper.h \
    opengl_shader.h \
    fbowrapper.h \
    preferences.h
SOURCES += opengl_helper.cpp \
    opengl_utils.cpp \
    opengl_shader.cpp \
    fbowrapper.cpp \
    preferences.cpp
CONFIG += staticlib \
    opengl

# CONFIG -= qt
DESTDIR = ../../../lib/
LIBS = ../../../lib/libarrays.a \
    ../../../lib/libqhasmath.a \
    ../../../lib/libglew.a
win32:LIBS += -lglu32 \
    -lopengl32
OBJECTS_DIR = ../../../obj
MOC_DIR = ../../../obj
RCC_DIR = ../../../obj
UI_DIR = .
TRANSLATIONS = ../../../ru.ts
