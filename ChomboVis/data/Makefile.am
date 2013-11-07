# We say "EXTRA" because otherwise the list of files has to be static.
# On the other hand, maybe this is why "make clean" doesn't get rid of
# the hdf5 files.
EXTRA_hdf5_DATA = $(patsubst %.hdf5.gz,%.hdf5,$(wildcard *.hdf5.gz))
EXTRA_hdf5dir = $(pkgdatadir)/data

EXTRA_ppm_DATA = $(patsubst %.ppm.gz,%.ppm,$(wildcard *.ppm.gz))
EXTRA_ppmdir = $(pkgdatadir)/data

EXTRA_stuff_DATA = $(wildcard $(srcdir)/*.cmap) switch_arrows.gif
EXTRA_stuffdir = $(pkgdatadir)/data

%.hdf5 :
	./gunzip.sh

%.ppm :
	./gunzip.sh

EXTRA_DIST = *.hdf5.gz *.ppm.gz gunzip.sh *.cmap switch_arrows.gif
MOSTLYCLEANFILES = *.hdf5 *.ppm
