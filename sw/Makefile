SOURCE ?= app.c
FPGA = ../fpga
FLASH = $(FPGA)/flash.tcl
LOADER = $(FPGA)/loader.py

main.o: $(SOURCE)
	avr-gcc -Wall -Os -DF_CPU=16000000 -mmcu=attiny85 -I../oled/include $^ -o $@

main.log: main.o
	avr-objdump -D $^ > $@

main.hex: main.o
	avr-objcopy -O binary $< $@.tmp
	xxd -e -a -c 1 -o 0 $@.tmp \
		| sed 's/://' \
		| sed 's/^\*//' \
		| sed '/^$$/d' \
		| awk {'print $$1"\n"$$2'} \
		> $@
	rm -rf $@.tmp

$(FLASH): main.hex
	$(LOADER) $^ > $@

.DEFAULT_GOAL := all

.PHONY: all
all: main.log main.hex $(FLASH)

.PHONY: clean
clean:
	rm -rf $(addprefix main., log o hex)
	rm -rf $(FLASH)
