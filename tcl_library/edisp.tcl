# edisp.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: edisp.tcl,v 1.6 1999/01/08 15:44:19 decoster Exp $
#

# edisp --
# usage : edisp str [int] [int] [int] [int]
#
#   Display an ext image.
#
# Parameters :
#   string  - The name of the ext image.
#   integer - Width to display.
#   integer - Heigth to display.
#   integer - Horizontal offset.
#   integer - Vertical offset.
#
# Return value :
#   Name of the created window.

proc edisp {eiName args} {
    global viewNb

    mylassign {gah cvwidth cvheight} [einfo [lindex $eiName 0]]
    set eihoffset 0
    set eivoffset 0
    if {$cvwidth >= 400} {
	set zoom [expr { 400.0/$cvwidth }]
    } else {
	set zoom 1
    }

    # Get args.

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -width {
		set cvwidth [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -height {
		set cvheight [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -eihoffset {
		set eihoffset [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -eivoffset {
		set eivoffset [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -zoom {
		set zoom [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    --  {
		set args [lreplace $args 0 0]
		break
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    set cvwidth [expr { int($cvwidth * $zoom) }]
    set cvheight [expr { int($cvheight * $zoom) }]

    incr cvwidth 80
    incr cvheight 60

    set edId $viewNb
    toplevel .ed$edId
    set w [canvas .ed$edId.c -width $cvwidth -height $cvheight]
    pack $w
    incr viewNb
    bind .ed$edId <Control-x><Control-d> [list destroy .ed$edId]

    myEiWin .ed${edId}.eiw $w $cvwidth $cvheight 0 0 $eihoffset $eivoffset $zoom $eiName

    label .ed$edId.msg -text "ha ha ha !"
    pack .ed$edId.msg

    bind .ed$edId <Right>        ".ed${edId}.eiw mult_zoom 1.2"
    bind .ed$edId <Left>         ".ed${edId}.eiw mult_zoom 0.8"
    bind .ed$edId <Up>           ".ed${edId}.eiw next_signal"
    bind .ed$edId <Down>         ".ed${edId}.eiw prev_signal"
    bind .ed$edId <l>            ".ed${edId}.eiw linear_coding"
    bind .ed$edId <b>            ".ed${edId}.eiw binary_coding"

    bind .ed$edId <Control-l>    ".ed${edId}.eiw reinit"
    bind .ed$edId <2>            "+ .ed${edId}.eiw cancel"

    bind .ed$edId <1> ".ed${edId}.eiw set_box %x %y"
    #bind .ed$edId <1> ".ed${edId}.eiw end_box %x %y"
    bind .ed$edId <Motion> ".ed${edId}.eiw motion_box %x %y"
    bind .ed$edId <Motion> "+ .ed${edId}.msg configure -text \[.ed${edId}.eiw the_motion %x %y\]"

    bind .ed$edId <d>     ".ed${edId}.eiw disableRefresh"
    bind .ed$edId <e>     ".ed${edId}.eiw enableRefresh"

    return $w
}
