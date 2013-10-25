lib1_LTLIBRARIES = lib1.la
lib1_la_SOURCES = lib1src.c
lib1dir = $(pkglibdir)
lib1_la_LIBADD = lib2.la

lib2_LTLIBRARIES = lib2.la
lib2_la_SOURCES = lib2src.c
lib2dir = $(pkglibdir)

bin_PROGRAMS = myprog
myprog_SOURCES = main.c
myprog_LDADD = lib1.la
myprogdir = $(bindir)
