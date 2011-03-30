#
# Minimal makefile for tcl_cruncher
#   a Tcl pseudo compiler & syntax checker
#   by Laurent Demailly <dl@hplyot.obspm.fr>
#
# see man page & README

VERS = 1.11

# It compiles with gcc -ansi -pedantic -Wall -O without any problem
# if you don't have an ansi compiler, grab gcc, or ansi2kr, or
# remove prototypes & const...

# To override default compiler flags :
# for HPUX hp's CC :
#CFLAGS=-Aa +O3 -s -D_HPUX_SOURCE
# for HPUX gnu cc :
#CFLAGS=-ansi -pedantic -Wall -O2 -s -D_HPUX_SOURCE
# for linux and most gcc's
CFLAGS=-ansi -pedantic -Wall -O2 -s 

# To change default compiler
#CC=gcc

RM = rm -f

LDFLAGS= $(CFLAGS)

OBJECTS= tcl_cruncher.o
TARGET = tcl_cruncher

$(TARGET):  $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

tgz: clean
	cd .. ; tar cvf tcl_cruncher-$(VERS).tar tcl_cruncher-$(VERS)/?[IHaEc]* ; gzip tcl_cruncher-$(VERS).tar

clean:
	$(RM) $(OBJECTS) $(TARGET) core *~

