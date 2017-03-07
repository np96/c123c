QT += qml quick sql concurrent

CONFIG += c++11

SOURCES += main.cpp \
    sqlgroupmodel.cpp \
    serverconnector.cpp \
    sqlcontactmodel.cpp \
    sqlabstractmodel.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    sqlgroupmodel.h \
    serverconnector.h \
    sqlcontactmodel.h \
    sqlabstractmodel.h
