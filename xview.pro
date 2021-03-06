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
    exchange-details.cpp \
    dialogopen.cpp \
    dialogabout.cpp \
    widgetbindings.cpp \
    widgetexchanges.cpp \
    widgetqueues.cpp \
    widgetsubscriptions.cpp \
    dialogexchanges.cpp \
    object-details.cpp \
    object-model.cpp \
    dialogobjects.cpp \
    widgetqmfobject.cpp \
    related-model.cpp \
    chart.cpp \
    sample.cpp \
    widgetsessions.cpp \
    commandlinkbutton.cpp \
    widgetconnections.cpp \
    fisheyelayout.cpp \
    propertydelegate.cpp \
    relatedheaderview.cpp

HEADERS  += xview.h \
    qmf-thread.h \
    exchange-model.h \
    exchange-details.h \
    dialogopen.h \
    dialogabout.h \
    widgetbindings.h \
    widgetexchanges.h \
    widgetqueues.h \
    widgetsubscriptions.h \
    dialogexchanges.h \
    object-details.h \
    object-model.h \
    dialogobjects.h \
    widgetqmfobject.h \
    related-model.h \
    chart.h \
    sample.h \
    widgetsessions.h \
    commandlinkbutton.h \
    widgetconnections.h \
    fisheyelayout.h \
    propertydelegate.h \
    relatedheaderview.h

FORMS    += xview.ui \
    dialogopen.ui \
    dialogabout.ui \
    dialogobjects.ui \
    widgetqmfobject.ui \
    chart.ui

OTHER_FILES += \
    license.txt \
    README.txt \
    CMakeLists.txt

RESOURCES += \
    xview.qrc
