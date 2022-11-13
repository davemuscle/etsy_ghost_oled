
STYLE=sobel
FOLDER=bitmaps
BITMAPS = $(shell ls $(FOLDER)/$(STYLE) | grep ".bmp")

all: clean $(addsuffix .h, $(addprefix include/, $(basename $(BITMAPS))))

.SECONDEXPANSION:
include/%.h: $(FOLDER)/$(STYLE)/$$*.bmp
	./bmp_func.py $(FOLDER)/$(STYLE) $* > $@
	cat $@ >> include/img.h

.PHONY:
clean:
	rm -rf include/*
