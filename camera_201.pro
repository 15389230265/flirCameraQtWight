#-------------------------------------------------
#
# Project created by QtCreator 2018-10-19T16:49:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = camera_201
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    myflircamera.cpp \
    mylogging.cpp

HEADERS += \
        mainwindow.h \
    myflircamera.h \
    mylogging.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#INCLUDEPATH+=D:/Spinnaker/include
#LIBS += D:\Spinnaker\lib64\vs2015\Spinnaker_v140.lib \
#        D:\Spinnaker\lib64\vs2015\Spinnakerd_v140.lib

#INCLUDEPATH+=D:/Spinnaker/1.13include
#LIBS += D:\Spinnaker\lib64\vs2013\Spinnaker_v120.lib \
#        D:\Spinnaker\lib64\vs2013\Spinnakerd_v120.lib
#DEFINES += _flir_spin_113

INCLUDEPATH+=D:/Spinnaker/newinclude
LIBS += D:\Spinnaker\lib\newvs2015\Spinnaker_v140.lib \
        D:\Spinnaker\lib\newvs2015\Spinnakerd_v140.lib
DEFINES += _flir_spin_new

INCLUDEPATH+=D:/OpenCV/opencv3.4/build/include/ \
                D:/OpenCV/opencv3.4/build/include/opencv \
                D:/OpenCV/opencv3.4/build/include/opencv2

LIBS += D:\OpenCV\opencv3.4\build\x64\vc15\lib\opencv_world340.lib \
        D:\OpenCV\opencv3.4\build\x64\vc15\lib\opencv_world340d.lib
