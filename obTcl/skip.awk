#!/bin/sh

# Remove unwanted sections of Tcl-code

awk '
BEGIN { no_output = 0; }
/^[ \t]*source[ \t]+/ { next }
/^[ \t]*crunch_skip begin[ \t]*$/ { no_output++; next }
/^[ \t]*crunch_skip end[ \t]*$/ {
	if (no_output > 0) {
		no_output--;
	} else {
		printf( "Warning unmatched \"crunch_skip end\" in file %s\n", \
			FILENAME ) | "cat 1>&2";
	}
	next
}
	{ if ( no_output == 0 ) { print } }
END {	if (no_output > 0) {
		printf( "Warning unmatched \"crunch_skip begin\" in file %s\n", \
			FILENAME ) | "cat 1>&2";
	}
}
'
