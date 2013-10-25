CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.servertestrc

AM_CPPFLAGS=-I$(includedir)

bin_PROGRAMS = servertest clienttest
servertest_SOURCES = cmdline.cpp servertest.cpp 
servertest_LDFLAGS=-L$(libdir) -lutils
clienttest_SOURCES = cmdline.cpp clienttest.cpp 
clienttest_LDFLAGS=-L$(libdir) -lutils

cmdline.cpp : cmdline.hpp
cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python ../../utils/src/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
