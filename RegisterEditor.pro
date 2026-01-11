QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Application source files
SOURCES += \
    src/registertreeviewhandler.cpp \
    src/devicemanager.cpp \
    src/registercontroller.cpp \
    src/clickablerectitem.cpp \
    src/draggableview.cpp \
    src/loginwindow.cpp \
    src/main.cpp \
    src/mainwindow.cpp

# Header files
HEADERS += \
    include/clickablerectitem.h \
    include/devicemanager.h \
    include/draggableview.h \
    include/loginwindow.h \
    include/mainwindow.h \
    include/registercontroller.h \
    include/registertreeviewhandler.h

# Resource files (UI, Stylesheets, etc.)
RESOURCES += resources/resources.qrc
DISTFILES += resources/style.qss

# UHAL IPBus library
INCLUDEPATH += /online/soft/opt/cactus/latest/include/
DEPENDPATH += /online/soft/opt/cactus/latest/include/
# LIBS += -L/online/soft/opt/cactus/latest/lib/ -lpugixml
LIBS += -L/online/soft/opt/cactus/latest/lib/ -lcactus_uhal_log
LIBS += -L/online/soft/opt/cactus/latest/lib/ -lcactus_uhal_uhal
LIBS += -L/online/soft/opt/cactus/latest/lib/ -lcactus_uhal_grammars


# Deployment rules (for Unix systems)
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Boost library
LIBS += -lboost_system

