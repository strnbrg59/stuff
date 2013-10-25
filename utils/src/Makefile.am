DASH_I = -I$(includedir)
AM_CPPFLAGS = $(KPPFLAGS) $(DASH_I)
AM_CXXFLAGS =
AM_CFLAGS =
CPPFLAGS =
CXXFLAGS =
CFLAGS =


# I can't compiler OpNewTracker since upgrading to g++4.3.
#lib_LTLIBRARIES = libutils.la libopnewtracker.la
lib_LTLIBRARIES = libutils.la
libutils_la_SOURCES = trace.cpp \
                      file_collector.cpp \
                      network-utils.cpp \
                      hsvrgb.cpp \
                      utils.cpp \
                      ols.cpp \
                      myregexp.cpp \
                      exceptions.cpp \
                      cmdline_base.cpp \
                      argmap.cpp \
                      timer.cpp \
                      cgiutils.cpp \
                      printable.cpp \
                      tokenizer.cpp
libutils_la_LDFLAGS = $(KDFLAGS)

#libopnewtracker_la_SOURCES = OpNewTracker.cpp

include_HEADERS =     argmap.hpp \
                      StaticMgr.hpp \
                      cmdline_base.hpp \
                      exceptions.hpp \
                      hsvrgb.hpp \
                      file_collector.hpp \
                      myregexp.hpp \
                      ols.hpp \
                      network-utils.hpp \
                      status-codes.hpp  \
                      trace.hpp  \
                      trace.cpp  \
                      printable.hpp  \
                      utils.hpp \
                      timer.hpp \
                      tokenizer.hpp \
                      cgiutils.hpp \
                      GenericArithmetic.hpp \
                      GenericArithmeticI.hpp \
                      IndexTM.hpp \
                      IndexTMI.hpp \
                      Metaprograms.hpp


cmdlinegenerator_SCRIPTS = generate_cmdline.py
cmdlinegeneratordir = $(libexecdir)

#
# Denial-of-service and buffer-overflow attack.
#
if HELLO
    bin_PROGRAMS = overflowtest hello
    hello_SOURCES = hello.cpp
    hello_CPPFLAGS = $(DASH_I)
    hello_LDFLAGS=-L$(libdir) -lutils
else !HELLO
    bin_PROGRAMS = overflowtest
endif !HELLO
overflowtest_SOURCES = overflowtest.cpp
overflowtest_CPPFLAGS = $(DASH_I)
overflowtest_LDFLAGS=-L$(libdir) -lutils
