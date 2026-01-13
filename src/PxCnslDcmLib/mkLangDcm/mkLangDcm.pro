#-------------------------------------------------
#
# Project created by QtCreator 2015-08-09T23:03:29
#
#-------------------------------------------------

QT       += core gui
QT += widgets
 
 
TARGET = mkLangDcm
TEMPLATE = app

DEFINES +=  _CRT_SECURE_NO_WARNINGS

#for debug DB
QMAKE_CXXFLAGS_RELEASE += -Zi
QMAKE_CXXFLAGS += -openmp
QMAKE_LFLAGS +=  -openmp


SOURCES += testDcmAPI.cpp testPxDcmProc.cpp stdafx.cpp mainwindow.cpp
	     

HEADERS  += testPxDcmProc.h mainwindow.h
 
FORMS    +=  mainwindow.ui
#RESOURCES += Resources/rc/QCmnFooter.qrc \ 
#TRANSLATIONS += Resources/translations/PX1_en.ts \         
#RC_FILE     = PX2Console.rc
INCLUDEPATH += ../include
     
LIBS    += -L../lib PxCnslDcmLib.lib
 
DESTDIR = ../bin
 

