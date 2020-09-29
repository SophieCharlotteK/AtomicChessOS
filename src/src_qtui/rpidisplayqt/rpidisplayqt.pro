QT += quick \
    widgets

# USING C++ 17 IS REQUIRED
CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
#QMAKE_CXXFLAGS += -std:c++17
#WORKAROUND FOR BUILDROOT COMPILE https://stackoverflow.com/questions/51196806/tell-qmake-std-c17-qmake-cxxflags-overridden
#QMAKE_CXXFLAG  += c++1z


DEFINES += USES_QT

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    SHARED/PROTOCOL_MSG.pb.cc \
    SHARED/guicommunicator.cpp \
    main.cpp \
    menumanager.cpp

RESOURCES += qml.qrc



# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    SHARED/PROTOCOL_MSG.pb.h \
    SHARED/guicommunicator.h \
    SHARED/magic_enum-master/include/magic_enum.hpp


LIBS += -pthread -lprotobuf -lpthread

DISTFILES += \
    SHARED/PROTOCOL_MSG.proto