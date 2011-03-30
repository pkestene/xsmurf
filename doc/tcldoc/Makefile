# $Id: Makefile,v 1.1.1.1 2004/10/23 00:38:29 tang Exp $

TCL=tclsh
FICKLE=~/fickle/fickle.tcl

all: tcldoc_scanner.tcl

tcldoc: tcldoc_scanner.tcl
	$(TCL) tcldoc.tcl \
	  -f --overview tcldoc.html --doc-files Makefile \
	  --title "TclDoc - The Tcl API Documentation Generator" tcldoc \
	  tcldoc.tcl tcldoc_scanner.tcl

%.tcl: %.fcl
	$(TCL) $(FICKLE) $<

clean:
	-rm -f tcldoc_scanner.tcl

.PHONY: clean tcldoc