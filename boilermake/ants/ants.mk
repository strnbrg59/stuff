SUBMAKEFILES = cmdline.mk

TARGET := antsprog

TGT_LDFLAGS := -L${TARGET_DIR} -L$(OPENGL_HOME)/lib -L$(X11_HOME)/lib
TGT_LDLIBS := -lutils -lGL -lGLU -lglut -lXmu
TGT_PREREQS := libutils.so

SRC_CXXFLAGS=-Wall -c -g -I$(OPENGL_HOME)/include -I$(X11_HOME)/include -I$(HOME)/usr/include/boost_1_33_0 -Iutils

TARGET_LDFLAGS= 

SOURCES :=  cmdline.cpp \
			display.cpp \
			field.cpp \
			ant.cpp \
			antutils.cpp \
			main.cpp
			
