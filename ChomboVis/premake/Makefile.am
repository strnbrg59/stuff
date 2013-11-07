# This directory has only one purpose.  It is to house a Makefile that is run
# before any of the other Makefiles, and which copies certain files (user.make,
# for example) from the source tree to the build tree.  (If
# those trees are the same then copy_stuff.sh exits before doing any copying.)

things_to_copy_SCRIPTS = foo
things_to_copydir = $(pkgdatadir)/premake
foo :
	$(srcdir)/checkmacosxenv.sh $(host)
	touch foo
	$(srcdir)/copy_stuff.sh $(top_srcdir) $(top_builddir)
EXTRA_DIST = copy_stuff.sh checkmacosxenv.sh
MOSTLYCLEANFILES = foo
