WEBSITE = $(HOME)/public_html/maps

$(WEBSITE)/home_burglary_%.html : ../data/home_burglary_%.coords make_map.py \
                                  javascript.header javascript.footer
	python make_map.py $< -o $@

TARGETS = $(WEBSITE)/home_burglary_may.html  \
          $(WEBSITE)/home_burglary_june.html \
          $(WEBSITE)/home_burglary_july.html \
          $(WEBSITE)/home_burglary_august.html \
          $(WEBSITE)/home_burglary_september.html \
          $(WEBSITE)/home_burglary_october.html \
          $(WEBSITE)/home_burglary_november.html

all : $(TARGETS)
