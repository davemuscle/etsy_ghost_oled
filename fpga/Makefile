
.PHONY: build
build:
	mkdir -p $@
	cd $@ && \
		quartus_sh -t ../board.qsf && \
		quartus_map build && \
		quartus_fit build  && \
		quartus_asm build

.PHONY: prog
prog:
	quartus_pgm -m jtag -o "p;build/build.sof"

.PHONY: console
console:
	system-console -cli -rc_script=./console.tcl

.PHONY: clean
clean:
	rm -rf build
	rm -rf flash.tcl
