
OV = 0.56

OBTCL_DIST = COPYRIGHT README Makefile \
	demo obtcl.tcl inherit.tcl base.tcl utils.tcl \
	Tabs.tcl ScrText.tcl PanedW.tcl TkManText.tcl Layout.tcl \
	wrap tabs docs panedw obtcl_mkindex \
	obTcl.n obTcl.cat obTcl.ascii obTcl.tkman \
	crunch_spec skip.awk man.macros \
	test.tst obtcl.c

OBTCL_IM = images/teapot.ppm

TCL_CRUNCH = \
	 ./tcl_cruncher/tcl_cruncher.man \
	 ./tcl_cruncher/tcl_cruncher.c \
	 ./tcl_cruncher/README \
	 ./tcl_cruncher/CHANGES \
	 ./tcl_cruncher/LICENSE \
	 ./tcl_cruncher/Makefile \
	 ./tcl_cruncher/tcl_compiler_crash_tests

all: tclIndex test

TCL_INCL= -I../../tcl7.5b1/generic

# Change to reflect

LD_SHARED=	cc -shared

lib: obtcl$(OV).so
obtcl$(OV).so: obtcl.c
	$(LD_SHARED) -o obtcl$(OV).so \
		-DOBTCL_VERSION='"$(OV)"' $(TCL_INCL) obtcl.c

test:
	cat obtcl.tcl test.tst | tclsh7.4

./tcl_cruncher/tcl_cruncher:
	( cd tcl_cruncher ; make )

crunch: ./tcl_cruncher/tcl_cruncher
	./tcl_cruncher/tcl_cruncher -d crunch_spec \
		utils.tcl obtcl.tcl inherit.tcl base.tcl | \
		./skip.awk > obtcl.crunched

strip:
	cat	utils.tcl obtcl.tcl inherit.tcl base.tcl | \
		./skip.awk > obtcl.stripped

man: obTcl.n
	groff -man obTcl.n -Tascii > obTcl.ascii
	cat obTcl.ascii | rman -k -f TkMan > obTcl.tkman

tclIndex: $(OBTCL_DIST)
	./obtcl_mkindex

distrib: tclIndex
	rm -rf obTcl-$(OV)
	mkdir obTcl-$(OV)
	cp README obTcl-$(OV).README
	tar cf - tclIndex $(OBTCL_DIST) $(OBTCL_IM) $(TCL_CRUNCH) \
		| (cd obTcl-$(OV); tar xkvf -)
	tar cvf obTcl-$(OV).tar obTcl-$(OV)
	gzip -f obTcl-$(OV).tar

clean:

install:
