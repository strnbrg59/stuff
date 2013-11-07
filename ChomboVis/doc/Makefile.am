#
#   _______              __
#  / ___/ /  ___  __ _  / /  ___
# / /__/ _ \/ _ \/  ' \/ _ \/ _ \
# \___/_//_/\___/_/_/_/_.__/\___/ 
#
# This software is copyright (C) by the Lawrence Berkeley
# National Laboratory.  Permission is granted to reproduce
# this software for non-commercial purposes provided that
# this notice is left intact.
# 
# It is acknowledged that the U.S. Government has rights to
# this software under Contract DE-AC03-765F00098 between
# the U.S. Department of Energy and the University of
# California.
#
# This software is provided as a professional and academic
# contribution for joint exchange.  Thus it is experimental,
# is provided ``as is'', with no warranties of any kind
# whatsoever, no support, no promise of updates, or printed
# documentation.  By using this software, you acknowledge
# that the Lawrence Berkeley National Laboratory and
# Regents of the University of California shall have no
# liability with respect to the infringement of other
# copyrights by any part of this software.
#

EXTRA_html_DATA = $(patsubst %.htsrc,%.html,$(wildcard *.htsrc)) $(wildcard *.html)
EXTRA_htmldir = $(prefix)/doc/ChomboVis

EXTRA_images_DATA = $(wildcard images/*)
EXTRA_imagesdir = $(prefix)/doc/ChomboVis/images

%.html : %.htsrc
	$(srcdir)/make_doc.sh

EXTRA_DIST = big_picture.dot \
             python_big_picture.dot \
             digraph.py \
             digraph_test.sh \
             gen_frame1.py \
             text2html.py \
             upcaseStyle.py \
             make_doc.sh \
             frame1.html \
             *.htsrc \
             images/*.gif images/*.jpg images/*.ps images/*.fig

frame1.html :
	python gen_frame1.py > frame1.html

MOSTLYCLEANFILES = *.html *.pyc index.html_insert


#
# Regular make stuff.
#
# Must have dot (Graphviz program) installed to make digraphs:
digraphs:
	./make-digraphs.sh
