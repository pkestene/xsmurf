# eiFiles.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: eiFiles.tcl,v 1.4 1999/05/06 11:59:26 decoster Exp $
#

package provide eiFiles 0.0

# chains2dat --
# usage: chains2dat str [... options ...]
#
#   From an ext image, create .dat files usable by Greg.
#
# Parameters:
#   string - The ext image name.
#
# Options:
#   -max: Save gradients of maxima that are on vertical chains.
#      [string] - Name of the resulting file. Default is mEI.dat if EI is the
#                 name of the ext image.
#   -gr: Save gradients of maxima that are local maxim along a contour line.
#      [string] - Name of the resulting file. Default is gEI.dat if EI is the
#                 name of the ext image.
#   -line: Save position of each point of each contour line.
#      [string] - Name of the resulting file. Default is lEI.dat if EI is the
#                 name of the ext image.
#   -mult: Apply a multiplicative value to each gradient size.
#      float - The value.
#   -zoom: Apply a zoom to all the outputs.
#      float - value of the zoom.
#   -offset: Apply an offset to all positions.
#      integer|list - If one value is given, take this value for x and y
#                     offsets. Otherwise list of these 2 values.
#   -center: Center all positions.
#   -onlyposition: Used with "gradient" options (-max or -gr). Save only the
#      position of the gradient.
#
# Return value:
#   None.

proc chains2dat {eiName args} {

    set isMax no
    set mFileName m$eiName.dat
    set isLine no
    set lFileName l$eiName.dat
    set isGr no
    set gFileName g$eiName.dat
    set mult 1
    set xOffset 0
    set yOffset 0
    set zoom 1
    set isCenter no
    set isOnlyposition no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -max {
		set isMax yes
		set nextArg [lindex $args 1]
		if {$nextArg == "" || [string match -* $nextArg]} {
		    set args [lreplace $args 0 0]
		} else {
		    set mFileName $nextArg
		    set args [lreplace $args 0 1]
		}
	    }
	    -line {
		set isLine yes
		set nextArg [lindex $args 1]
		if {$nextArg == "" || [string match -* $nextArg]} {
		    set args [lreplace $args 0 0]
		} else {
		    set lFileName $nextArg
		    set args [lreplace $args 0 1]
		}
	    }
	    -gr {
		set isGr yes
		set nextArg [lindex $args 1]
		if {$nextArg == "" || [string match -* $nextArg]} {
		    set args [lreplace $args 0 0]
		} else {
		    set gFileName $nextArg
		    set args [lreplace $args 0 1]
		}
	    }
	    -mult {
		set mult [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -offset {
		set offset [lindex $args 1]
		set args [lreplace $args 0 1]
		if {[llength $offset] == 1} {
		    set xOffset $offset
		    set yOffset $offset
		} else {
		    mylassign {xOffset yOffset} $offset
		}
	    }
	    -zoom {
		set zoom [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -center {
		set isCenter yes
		set args [lreplace $args 0 0]
	    }
	    -onlyposition {
		set isOnlyposition yes
		set args [lreplace $args 0 0]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    mylassign {scale lx ly extrNb chainNb nbOfLines stamp} [einfo $eiName]
    mylassign {min max} [egetextr $eiName]

    if {$isCenter == "yes"} {
	set xOffset [expr { -$lx/2 }]
	set yOffset [expr { -$ly/2 }]
    }

    set mult [expr { $mult }]

    if {$isLine == "yes"} {
	set lFileId [open $lFileName w]

	eiloop $eiName {
	    set zex [expr { ($xOffset + $x) * $zoom }]
	    set zey [expr { ($yOffset + $y) * $zoom }]
	    puts $lFileId "$zex $zey"
	}

	close $lFileId
    }

    if {$isGr == "yes"} {
	set gFileId [open $gFileName w]
    }
    if {$isMax == "yes"} {
	set mFileId [open $mFileName w]
    }

    if {$isGr == "yes" || $isMax == "yes"} {
	eigrloop $eiName {
	    if {$isMax == "yes" && $type == "tag vc"} {
		set zex [expr { ($xOffset + $xx) * $zoom }]
		set zey [expr { ($yOffset + $yy) * $zoom }]
		puts $mFileId "$zex $zey"
		if {$isOnlyposition == "no"} {
		    set x1 [expr $zex+$mult*$mod*cos($arg)]
		    set y1 [expr $zey+$mult*$mod*sin($arg)]
		    puts $mFileId "$x1 $y1"
		}
	    }
	    if {$isGr == "yes"} {
		set zex [expr { ($xOffset + $xx) * $zoom }]
		set zey [expr { ($yOffset + $yy) * $zoom }]
		puts $gFileId "$zex $zey"
		if {$isOnlyposition == "no"} {
		    set x1 [expr $zex+$mult*$mod*cos($arg)]
		    set y1 [expr $zey+$mult*$mod*sin($arg)]
		    puts $gFileId "$x1 $y1"
		}
	    }
	}
    }

    if {$isGr == "yes"} {
	close $gFileId
    }
    if {$isMax == "yes"} {
	close $mFileId
    }

    return
}

