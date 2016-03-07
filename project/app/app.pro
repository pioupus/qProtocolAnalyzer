include(../defaults.pri)

TEMPLATE = app
#DEFINES += EXPORT_APPLICATION


 message($$LIBS)
#CONFIG( debug, debug|release ) {
    # debug
#     LIBS += -L../debug/src/
#} else {
    # release
#    LIBS += -L../release/src/

#}

CONFIG(release, debug|release) {
    #message(Release)
}

SOURCES +=  main.cpp

CONFIG(debug, debug|release) {
   # message(Debug)
}

CONFIG( debug, debug|release ) {
    # debug
     #message(Debug)


     LIBS += -L../src/debug/
     LIBS += -L../src/
     LIBS +=  -lqTestProjectTemplateAppd
} else {
    # release
    #message(Release)
    LIBS += -L../src/release/
    LIBS += -L../src/
    LIBS +=  -lqTestProjectTemplateApp

}
#LIBS +=  -lqTestProjectTemplateApp
