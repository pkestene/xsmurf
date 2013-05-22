
#----------------------------------------------------------------------
# Some generic utility functions
#

proc cmt args {}
proc Nop {} {}

proc setIfNew { var val } {
	global $var
	if ![info exists $var] {
		set $var $val
	}
}

proc setIfNewArray { var val } {
    global $var
    if ![info exists $var] {
	array set $var {
	    nothing $val
	}
    }
}

proc crunch_skip args {}

crunch_skip begin

cmt {
proc o_push { v val } {
	upvar 1 $v l
	lappend l $val
}
proc o_pop v {
	upvar 1 $v l
	set tmp [lindex $l end]
	catch {set l [lreplace $l end end]}
	return $tmp
}
proc o_peek v {
	upvar 1 $v l
	return [lindex $l end]
}
}

crunch_skip end

proc lappendUniq { v val } {
	upvar $v var

	if { [lsearch $var $val] != -1 } { return }
	lappend var $val
}
proc listMinus { a b } {
	set ret {}
	foreach i $a { set ArrA($i) 1 }
	foreach i $b { set ArrB($i) 1 }
	foreach i [array names ArrA] {
		if ![info exists ArrB($i)] {
			lappend ret $i
		}
	}
	return $ret
}

#----------------------------------------------------------------------
# 
# StrictMotif:  Redefine look-and-feel to be more Motif like.
#	This routine disables scrollbar from being pushed in (sunken),
#	as well as sets the tk_strictMotif variable.

# `_otReferenceSBD' is only for string comparison with currently used routine.
# DO NOT ALTER IN ANY WAY!
#
set _otReferenceSBD {
    global tkPriv
    set tkPriv(relief) [$w cget -activerelief]
    $w configure -activerelief sunken
    set element [$w identify $x $y]
    if {$element == "slider"} {
	tkScrollStartDrag $w $x $y
    } else {
	tkScrollSelect $w $element initial
    }
}
proc otTkScrollButtonDown {w x y} {
    global tkPriv
    set tkPriv(relief) [$w cget -activerelief]
    set element [$w identify $x $y]
    if [string compare "slider" $element] {
	$w configure -activerelief sunken
	tkScrollSelect $w $element initial
    } else {
	tkScrollStartDrag $w $x $y
    }
}

proc StrictMotif {} {
    global tk_version tk_strictMotif _otReferenceSBD
    set tk_strictMotif 1
    # if { $tk_version >= 7.0 ||
    # 	![string compare [info body tkScrollButtonDown] \
    # 		[set _otReferenceSBD]] } {
    # 	if [string compare "" [info procs otTkScrollButtonDown]] {
    # 		rename tkScrollButtonDown {}
    # 		rename otTkScrollButtonDown tkScrollButtonDown
    # 	}
    # }
    rename otTkScrollButtonDown tkScrollButtonDown
}

proc dbputs s {}

# Dummy to allow crunched obtcl processing normal obTcl-scripts
proc DOC { name rest } {}
proc DOC_get_list {} {}

crunch_skip begin

setIfNew db_debug 0
proc db_debug {} {
	global db_debug
	set db_debug [expr !$db_debug]
}
proc dbputs s {
	global db_debug
	if { $db_debug != 0 } {
		puts stderr $s
	}
}

#----------------------------------------------------------------------
# DOCS

setIfNewArray _uPriv_DOCS ""

proc DOC_get_list {} {
	global _uPriv_DOCS
    	return [array names _uPriv_DOCS]
}

proc DOC { name rest } {
	global _uPriv_DOCS
	set _uPriv_DOCS($name) $rest
}

proc PrDOCS {} {
	global _uPriv_DOCS
	set names [lsort [array names _uPriv_DOCS]]
	foreach i $names {
		puts "$_uPriv_DOCS($i)"
		puts "----------------------------------------------------------------------"
	}
}
proc GetDOCS {} {
	global _uPriv_DOCS
	set names [lsort [array names _uPriv_DOCS]]
	set all ""
	foreach i $names {
		append all "$_uPriv_DOCS($i)"
		append all "----------------------------------------------------------------------"
	}
	return $all
}
proc GetDOC name {
	global _uPriv_DOCS
	return $_uPriv_DOCS($name)
}
proc ohelp args {
	global _uPriv_DOCS
	set names [lsort [array names _uPriv_DOCS "${args}*"]]
	
	if { [llength $names] > 1 } {
		puts "Select one of: "
		set n 1
		foreach i $names {
			puts "	${n}) $i  "
			incr n 1
		}
		puts -nonewline ">> "
		set answ [gets stdin]
		append tmp [lindex $names [expr $answ-1]]
		eval help $tmp
	}
	if { [llength $names] == 1 } {
		eval set tmp $names
		puts $_uPriv_DOCS($tmp)
	}
	if { [llength $names] < 1 } {
		puts "No help on: $args"
	}
}

#----------------------------------------------------------------------
DOC "Tcl-debugger" {

 NAME
	Tcldb	- A Tcl debugger

 SYNOPSIS
	bp ?ID?

 DESCRIPTION
	A simple debugger for Tcl-script.  Breakpoints are set by calling
	`bp' from your Tcl-code.  Selecting where to break is done by
	string-matching.

 USAGE
	Use by putting calls to `bp' in the Tcl-code.  If `ID' is specified,
	it will be displayed when the breakpoint is reached.

 	Example of using two breakpoints with different IDs:

	func say { a } {

		bp say_A

		puts "You said: $a!"
	
		bp say_B
	}

	Call `bpOff' to disable all breakpoints, `bpOn' to enable all,
	`bpOn <funcname>' to enable breakpoints in functions matching
	<funcname>, and finally `bpID <ID>' to enable breakpoints
	matching <ID>.  Matching is done according to Tcl's `string match'
	function.

	When in the break-point handler, type "?" for help.

 ACKNOWLEDGEMENTS
	This simple debugger is based on Stephen Uhler's article
	"Debugging Tcl Scripts" from the Oct-95 issue of Linux Journal.
}

proc bpGetHelp {} {
	puts stderr \
"------------------------------- Tcldb help ------------------------------------

    Set breakpoints by adding calls to `bp' in your Tcl-code. Example:

	bp Func1	;# bp followed by the identifier `Func1'

    Commands available when in `bp':

	+	Move down in call-stack
	-	Move up in call stack
	.	Show current proc name and params

	v	Show names of variables currently in scope
	V	Show names and values of variables currently in scope
	l	Show names of variables that are local (transient)
	L	Show names and values of variables that are local (transient)
	g	Show names of variables that are declared global
	G	Show names and values of variables that are declared global
	t	Show a call chain trace, terse mode
	T	Show a call chain trace, verbose mode

	b	Show body of current proc
	c	Continue execution
	h,?	Print this help

	You can also enter any Tcl command (even multi-line) and it will be
	executed in the currently selected stack frame.

    Available at any time:

	bpOff	Turn off all breakpoints
	bpOn	Turn on all breakpoints
	bpOn <match>
		Enable breakpoints in functions with names matching <match>
	bpID <match>
		Enable breakpoints whose ID matches <match>
"
}
setIfNew _bp_ON 1
setIfNew _bp_ID *

proc bpOn { {func 1} } { global _bp_ON _bp_ID; set _bp_ID *; set _bp_ON $func }
proc bpID id { global _bp_ON _bp_ID; set _bp_ON 1; set _bp_ID $id }
proc bpOff {} { global _bp_ON; set _bp_ON 0 }

proc bp args {
	global _bp_ON _bp_ID
	if { $_bp_ON == 0 } { return }
	set max [expr [info level] - 1]
	set current $max
	set fName [lindex [info level $current] 0]
	if { "$_bp_ON" == "1" || "$fName" == "$_bp_ON" || \
		("$_bp_ON" == "top" && $current == 0) || \
		[string match $_bp_ON $fName] } {
		if ![string match $_bp_ID $args] {
			return
		}
	} else {
		return
	}
	bpShow VERBOSE $current
	while {1} {
		if { "$args" != "" } { puts "bp: $args" }
		puts -nonewline stderr "#${current}:"
		gets stdin line
		while {![info complete $line]} {
			puts -nonewline "> "
			append line "\n[gets stdin]"
		}
		switch -- $line {
		"+"	{if {$current < $max} {bpShow VERBOSE [incr current]}}
		"-"	{if {$current > 0} {bpShow VERBOSE [incr current -1]}}
		"b"	{bpBody $current}
		"c"	{puts stderr "Continuing"; return}
		"v"	{bpVisibleVars NAMES $current}
		"V"	{bpVisibleVars VALUES $current}
		"l"	{bpLocalVars NAMES $current}
		"L"	{bpLocalVars VALUES $current}
		"g"	{bpGlobalVars NAMES $current}
		"G"	{bpGlobalVars VALUES $current}
		"t"	{bpTraceCalls TERSE $current}
		"T"	{bpTraceCalls VERBOSE $current}
		"."	{bpShow VERBOSE $current}
		"h"	-
		"?"	{bpGetHelp}
		default	{
			catch {uplevel #$current $line } result
			puts stderr $result
			}
		}
	}
}

proc bpPrVar { level mode name } {
	upvar #$level $name var
	if { $mode == "NAMES" } {
		puts "  $name"
		return
	}
	if { [array exists var] == 1 } {
		puts "  Array ${name} :"
		foreach i [array names var] {
			puts "    ${name}($i) = [set var($i)]"
		}
	} else {
		if {[info exists var] != 1 } {
			puts "  $name : Declared but uninitialized"
		} else {
			puts "  $name = $var"
		}
	}
}

proc bpBody current {
	uplevel #$current {
		catch {puts [info body [lindex [info level [info level]] 0]]}
	}
}
proc bpVisibleVars { mode curr } {
	puts "#$curr visible vars:"
	foreach i [uplevel #$curr {lsort [info vars]}] {
		bpPrVar $curr $mode $i
	}
}
proc bpLocalVars { mode curr } {
	puts "#$curr local vars:"
	foreach i [uplevel #$curr {lsort [info locals]}] {
		bpPrVar $curr $mode $i
	}
}
proc bpGlobalVars { mode curr } {
	puts "#$curr global visible vars:"
	set Vis [uplevel #$curr {info vars}]
	set Loc [uplevel #$curr {info locals}]
	foreach i [lsort [listMinus $Vis $Loc]] {
		bpPrVar 0 $mode $i
	}

}
proc bpTraceCalls { mode curr } {
	for {set i 1} {$i <= $curr} {incr i} {
		bpShow $mode $i
	}
}
proc bpShow { mode curr } {
    if { $curr > 0 } {
	set info [info level $curr]
	set proc [lindex $info 0]
	if {"$mode" == "TERSE"} {
		puts stderr "$curr: $proc [lrange $info 1 end]"
		return
	}
	puts stderr "$curr: Proc= $proc \
		{[info args $proc]}"
	set idx 0	
	foreach arg [info args $proc] {
		if { "$arg" == "args" } {
			puts stderr "\t$arg = [lrange $info [incr idx] end]"
			break;
		} else {
			puts stderr "\t$arg = [lindex $info [incr idx]]"
		}
	}
    } else {
	puts stderr "Top level"
    }
}

crunch_skip end


