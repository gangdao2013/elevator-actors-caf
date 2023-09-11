TEMPLATE = app
LANGUAGE = C++
CONFIG	+= warn_on thread console c++17 debug
QMAKE_CXXFLAGS *= -wd4530 -wd4251 -wd4275


DESTDIR		=	../bin
OBJECTS_DIR =   .obj

AFCDIR = G:/opensource/actor-framework-master
INCLUDEPATH = $$AFCDIR/libcaf_core \
				$$AFCDIR/libcaf_io \
				$$AFCDIR/libcaf_net \
				$$AFCDIR/build/libcaf_core \
				$$AFCDIR/build/libcaf_io \
				$$AFCDIR/build/libcaf_net \
				../elevator_core

CONFIG(debug, debug|release) {
	TARGET = elevator_d
	
	LIBS *= \
			-L$$AFCDIR/lib \
			-lcaf_core \
			-lcaf_io \
			-lcaf_net \
			-L../lib \
			-lelevator_core_d
}
				
SOURCES	=	main.cpp 
