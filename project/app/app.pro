include(../defaults.pri)

TEMPLATE = app
#DEFINES += EXPORT_APPLICATION

 LIBS += -L../src/

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

CONFIG(debug, debug|release) {
   # message(Debug)
}

CONFIG( debug, debug|release ) {
    # debug
message(Debug)
     LIBS +=  -lqTestProjectTemplateAppd
} else {
    # release
   message(Release)
    LIBS +=  -lqTestProjectTemplateApp

}
#LIBS +=  -lqTestProjectTemplateApp
