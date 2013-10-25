EXTRA_txt_DATA = $(wildcard *.txt)
EXTRA_txtdir = $(pkgdatadir)/test/canonicals
EXTRA_ppm_DATA = $(patsubst %.ppm.gz,%.ppm,$(wildcard *.ppm.gz))
EXTRA_ppmdir = $(pkgdatadir)/test/canonicals

%.ppm :
	$(srcdir)/gunzip.sh

EXTRA_DIST = *.txt *.ppm.gz gunzip.sh
MOSTLYCLEANFILES = *.ppm
