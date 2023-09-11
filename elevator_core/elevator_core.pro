
win32{
	DEFINES	*= WIN32
	DEFINES *= __ELEVATOR_CORE__
}

TEMPLATE = lib
LANGUAGE = C++
CONFIG	+= c++17 warn_off debug
QMAKE_CXXFLAGS *= -wd4530 -wd4251

DESTDIR		=	../lib
DLLDESTDIR	=	../bin
OBJECTS_DIR =   .obj

AFCDIR = G:/opensource/actor-framework-master
INCLUDEPATH = $$AFCDIR/libcaf_core \
				$$AFCDIR/libcaf_io \
				$$AFCDIR/libcaf_net \
				$$AFCDIR/build/libcaf_core \
				$$AFCDIR/build/libcaf_io \
				$$AFCDIR/build/libcaf_net

CONFIG(debug, debug|release) {
	TARGET = elevator_core_d
}
CONFIG(realse, debug|release) {
	TARGET = elevator_core
}
	
LIBS *= \
		-L$$AFCDIR/lib \
		-lcaf_core \
		-lcaf_io \
		-lcaf_net
				
SOURCES	= \
    src/controller_repl_actor.cpp \
    src/dispatcher_actor.cpp \
    src/repl_actor.cpp \
    src/controller_actor.cpp \
	src/elevator_actor.cpp \
    src/elevator_fsm.cpp \
    src/elevator_repl_actor.cpp \
    src/passenger_actor.cpp \
    src/passenger_fsm.cpp \
    src/passenger_repl_actor.cpp \
    src/string_util.cpp

HEADERS = \
    elevator/controller_actor.hpp \
    elevator/controller_repl_actor.hpp \
    elevator/dispatcher_actor.hpp \
    elevator/elevator.hpp \
    elevator/elevator_actor.hpp \
    elevator/elevator_fsm.hpp \
    elevator/elevator_repl_actor.hpp \
    elevator/passenger_actor.hpp \
    elevator/passenger_fsm.hpp \
    elevator/passenger_repl_actor.hpp \
    elevator/repl_actor.hpp \
    elevator/schedule.hpp \
    elevator/string_util.hpp
