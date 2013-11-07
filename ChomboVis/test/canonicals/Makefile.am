EXTRA_ppm_DATA = $(patsubst %.ppm.gz,%.ppm,$(wildcard *.ppm.gz))
EXTRA_ppmdir = $(pkgdatadir)/test/canonicals

EXTRA_txt_DATA = $(wildcard *.txt)
EXTRA_txtdir = $(pkgdatadir)/test/canonicals

%.ppm :
	$(srcdir)/../../data/gunzip.sh

EXTRA_DIST = *.ppm.gz *.txt
MOSTLYCLEANFILES = *.ppm
