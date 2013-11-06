CMDLINE_VAR_INFO_FILE=cmdline_var_info.txt
CMDLINE_FILE=.mailserverrc

AM_CPPFLAGS=-I$(includedir)

lib_LTLIBRARIES = libmailserver.la
libmailserver_la_SOURCES = cmdline.cpp mailserver.cpp

cmdline.cpp : cmdline.hpp

cmdline.hpp : $(CONFIG_VAR_INFO_FILE)
	python ../../utils/src/generate_cmdline.py $(PWD)/$(CMDLINE_VAR_INFO_FILE) $(CMDLINE_FILE)

include_HEADERS = mailserver.hpp

bin_PROGRAMS = mailserver
mailserver_SOURCES = mailserver-main.cpp
mailserver_LDFLAGS=-L$(libdir) -lmailserver -lutils

MOSTLYCLEANFILES = cmdline.cpp cmdline.hpp
