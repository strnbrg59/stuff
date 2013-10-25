libcommon_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = nothing.c
libcommon_la_SOURCES -= nothing.c

if THREADS
    libcommon_la_SOURCES += lock.c
else
    libcommon_la_SOURCES += lock_nothreads.c
endif !THREADS

$(warning @@ libcommon_la_SOURCES = $(libcommon_la_SOURCES))
$(warning @@ THREADS = $(THREADS))
$(warning @@ PROD = $(PROD))

libcommondir = $(pkglibdir)
