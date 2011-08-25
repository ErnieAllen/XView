#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T11:21:14
#
#-------------------------------------------------

QT       += core gui

TARGET = xview
TEMPLATE = app


SOURCES += main.cpp\
        xview.cpp \
    qmf-thread.cpp \
    exchange-model.cpp \
    exchange-details.cpp

HEADERS  += xview.h \
    qmf-thread.h \
    exchange-model.h \
    exchange-details.h

FORMS    += xview.ui

OTHER_FILES += \
    license.txt
