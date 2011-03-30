# lwPkg.tcl --
#
#       Interface with last wave.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: lwPkg.tcl,v 1.5 1999/06/25 19:58:43 decoster Exp $
#

package provide lwPkg 0.0

namespace eval lwPkg {
    variable lwPath /home/pkestene/install/LastWave/LastWave-1.7/bin/lw
    #variable lwPath /home/kestener/install/LastWave//LastWave_nicolas/bin/lw
    variable tmpFile /home/pkestene/tmp/_delete_me_scr4lw_[pid]
}

# scr4lw --
# usage : scr4lw script
#
#   Create a file that contains a script and execute it with LastWave ( (c) E.
# Bacry ).
#
# Arguments:
#   script - The script to execute. This script is parsed before creating the
#            file. Any occurence of "@$varName@" is replaced by the value of the
#            variable "varName".
#
# Return value:
#   None.

proc scr4lw {scr} {
    set zePath $lwPkg::lwPath
    set zeFile $lwPkg::tmpFile

    if {[file exists $zeFile] == 1} {
	return -code error "temporary file for the script allready exists ($zeFile)"
    }
    if {[file exists $zePath] == 0} {
	return -code error "can't find last wave binary ($zePath)"
    }

    # Parse the script to replace key words by there value.

    while {[regexp {@\$([a-zA-Z0-9_]|{[.]})*@} $scr string] == 1} {
	set firstI 2
	set lastI [expr { [string length $string] - 2 }]
	set varName [string range $string $firstI $lastI]
	upvar $varName theVar
	if {[info exists theVar] == 0} {
	    return -code error "can't read \"$varName\": no such variable"
	}
	set exp [format "@\\$%s@" $varName]
	regsub -all $exp $scr $theVar scr
    }

    # Create the macro file.

    set f [open $zeFile w]
    puts $f $scr
    puts $f "\n\nterminal eof\n"
    close $f

    # Launch lw.

    catch [concat exec $zePath -b < $zeFile] res

    file delete $zeFile

    return
}

