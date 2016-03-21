include(../defaults.pri)

QPROTOCOL_INTERPRETER_PATH=../libs/qRPCRuntimeParser
include($$QPROTOCOL_INTERPRETER_PATH/qProtocollInterpreter_static.pri)

CONFIG( debug, debug|release ) {
    # debug
     TARGET = qTestProjectTemplateAppd
} else {
    # release
    TARGET = qTestProjectTemplateApp

}



TEMPLATE = lib

DEFINES += EXPORT_LIBRARY

SOURCES += main.cpp
SOURCES += mainwindow.cpp
SOURCES += serialnode.cpp

HEADERS += mainwindow.h
HEADERS += serialnode.h



FORMS    += mainwindow.ui



#INCLUDEPATH += $$(PYTHON_PATH)/include

#exists( $$(PYTHON_PATH)/include/python.h ) {
#    #message(found python windows)
#    INCLUDEPATH += $$(PYTHON_PATH)/include
#    PYTHON_FOUND = 1
#}

#exists( $$(PYTHON_PATH)/Python.h ) {
    #message(found python linux)crystalTestFrameworkApp
#    INCLUDEPATH += $$(PYTHON_PATH)
#    PYTHON_FOUND = 1
#}

#!equals( PYTHON_FOUND , 1){
#    error (Python directory needs to be configured in environment variable PYTHON_PATH. eg. C:/Python27 )
#}
