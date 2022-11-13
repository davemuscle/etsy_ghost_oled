
BITMAPS = $(shell ls bitmaps | grep ".bmp")

all: clean $(addsuffix .h, $(addprefix include/, $(basename $(BITMAPS))))

.SECONDEXPANSION:
include/%.h: bitmaps/$$*.bmp
	./bmp_func.py bitmaps $* > $@
	cat $@ >> include/img.h

.PHONY:
clean:
	rm -rf include/*
