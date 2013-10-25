CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.astrorc

AM_CPPFLAGS=-I$(includedir) -I$(HOME)/usr/include/boost_1_33_0

lib_LTLIBRARIES = libastro.la
libastro_la_SOURCES = cmdline.cpp \
                      stretch_params.cpp \
                      point.cpp \
                      sample.cpp \
                      star_accreter.cpp \
                      misc.cpp \
                      rawdata.cpp \
                      triangle.cpp \
                      ols.cpp

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python $(libexecdir)/generate_cmdline.py $(srcdir)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

#include_HEADERS = astro.hpp ols.hpp

bin_PROGRAMS = astro
astro_SOURCES = astro-main.cpp
astro_LDFLAGS=-L$(libdir) -lastro -lutils

EXTRA_DIST = cmdline_var_info.txt

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
