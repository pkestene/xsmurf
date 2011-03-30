# target.tcl --
#
#       This file implements the Tcl code for a kind target system "a la" make.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: target.tcl,v 1.18 1999/03/24 15:18:59 decoster Exp $
#

# target --
# Package target :
#
#   This package handles a kind of target system "a la" make. Here is the list
# of the commands :
#   help
#   def
#   make
#   isMade
#   msg

# target example --
#   Here is an example of what you can do with the target package :
# 
# set vlo 0
# 
# tg def gah1 {
#     set f [open @tg@ w]
#     puts $f "hahaha"
#     close $f
# }
# 
# tg def gah2 {
#     tg make gah1
#     set f [open @tg@ w]
#     puts $f "hohoho $vlo"
#     close $f
# }
# 
# tg def {tag hag} {
#     tg make gah2
#     tg msg "@tg@ is mlaha"
# }
#
# for {set i 0} {$i < 4} {incr i} {
#     tg def blah$i {
# 	set f [open @tg@ w]
# 	puts $f @$i@
# 	close $f
#     }
# }
# 
# tg make hag
# tg make blah0
# tg make blah1
# tg make blah2
# tg make blah3

package provide target 0.2

namespace eval target {
    variable typeLst {file tag}
    variable msgCmd target::newPuts
    variable targetLevel 0
    variable nCol 80
    variable currCol 0
    variable isNoNewLine no
}


# tg --
# usage : tg args
#
#   Execute a script in the target namespace.
#
# Parameters :
#   args - a list of arg.
#
# Return value :
#   Result of the execution.

proc tg args {
    if {[llength $args] != 0} {
	set cmd [concat namespace inscope target $args]
	set code [catch {eval $cmd} result]
	if {$code != 0} {
	    return -code error $result
	} else {
	    return $result
	}
    }
}


# target::help --
# usage : target::help [proc]
#
#   Get help on target pakage.
#
# Parameters :
#   [proc] - help on a proc.
#
# Return value :
#   Help message.

proc target::help args {
    return {no help for now}
}


# target::CheckTid - PRIVATE
# usage : target::CheckTid tid
#
#   Check if a target id is valid.
#
# Parameters :
#   tid - the target id.
#
# Return value :
#   1 if valid, 0 otherwise.

proc target::CheckTid {tid} {
    if [array exists target::tg_$tid] {
	return 1
    } else {
	return 0
    }
}


# target::def --
# usage : target::def {[type] tid} script
#
#   Define a target. If this target is allready defined with exactly the same
# script, the target (and its state) is not changed. So if it is allready made
# subsequent calls of tg make will do nothing.
#
# Parameters :
#   [type] - Type of the target.
#   tid    - Identifier of the target.
#   script - The script to execute to make the target. Any "@tg@" sequence in
#            the script is replaced by the target identifier string. Any
#            "@$varName@" is replaced by the value of var "varName" in a higher
#            level (i.e. while calling target::def).
#
# Return value :
#   None.

proc target::def {tid scr} {

    # Extract the tid and the type.

    switch [llength $tid] {
	1 {
	    set type file
	}
	2 {
	    variable typeLst

	    set type [lindex $tid 0]
	    if {[lsearch $typeLst $type] == -1} {
		return -code error "wrong tid type"
	    }
	    set tid [lindex $tid 1]
	}
	default {
	    return -code error "wrong tid description"
	}
    }

    # Create the (array) variable associated to the target.

    variable tg_$tid
    upvar 0 tg_$tid target

    # Parse the script to replace key words by there value.

    regsub -all {@tg@} $scr $tid scr

    while {[regexp {@\$([a-zA-Z0-9_]|{[.]})*@} $scr string] == 1} {
	set firstI 2
	set lastI [expr { [string length $string] - 2 }]
	set varName [string range $string $firstI $lastI]
	upvar $varName theVar
	set exp [format "@\\$%s@" $varName]
	regsub -all $exp $scr $theVar scr
    }

    # Get the dependency list by looking to "tg make" in it.

    set allwaysMakeIt no
    set dependLst {}
    set tmpScr $scr

    # We remove comments from the script.

    set exp [format "#\[^%s.\]*" "\n"]
    regsub -all $exp $scr {} tmpScr

    # Loop on each occurence of the "tg make ..." in the script.

    set exp [format "(tg make \[^($|%s)\]*)" "\n"]
    while {[regexp $exp $tmpScr string] == 1} {
	regexp -indices $exp $tmpScr indices
	set firstI [lindex $indices 0]
	set lastI [lindex $indices 1]
	set tmpScr [string range $tmpScr [expr $lastI+1] end]
	set theTarget [lindex $string 2]

	# If there is a '$' in this dependency name, we can't figure what is the
	# real name of it. So by default, there will be no dependencies for the
	# current target, and we will allways make the current target.

	if {[regexp {\$} $theTarget] == 1} {
	    set allwaysMakeIt yes
	    set dependLst {}
	    break
	}
	set dependLst [concat $dependLst $theTarget]
    }

    if { ([info exists target(makeScr)] == 0) \
	    || ([string compare $target(makeScr) $scr] != 0) } {
	if {$type == "file" && [file exists $tid] == 1} {
	    set zeDate [file mtime $tid]
	} else {
	    set zeDate undefined
	}
	array set target [list \
		tid		$tid \
		date		$zeDate \
		type		$type \
		makeScr		$scr \
		allwaysMakeIt	$allwaysMakeIt ]
	if {$allwaysMakeIt == "no"} {
	    array set target [list dependLst $dependLst]
	}
    }

    return
}


# target::make --
# usage : target::make tidLst
#
#   Execute the script associated to a target, if the target is not allready made.
#
# Parameters :
#   tidLst - List of target's identifier.
#
# Return value :
#   Result of the script.

proc target::make {tidLst} {
    variable msgCmd
    variable targetLevel

    set result {}
    foreach tid $tidLst {
	if {[CheckTid $tid] == 0} {
	    return -code error "wrong target id ($tid)"
	}
	variable tg_$tid
	upvar 0 tg_$tid target

	if {[target::isMade $tid] == 0} {
	    msg -beginofline "make $tid..."
	    incr targetLevel
	    set code [catch {uplevel 3 $target(makeScr)} result]
	    incr targetLevel -1
	    if {$code == 0} {
		array set target [list date [clock seconds]]
		msg -beginofline "$tid ok."
	    } else {
		msg -beginofline "error while making $tid."
		return -code error $result
	    }
	} else {
	    msg -beginofline "$tid allready made."
	}
    }

    return $result
}


# target::isMade --
# usage : target::isMade tid date
#
#   Check if we must make a target or not (according to its dependencies).
#
# Parameters :
#   tid - Tid of the target.
#
# Return value :
#   1 if target is made, 0 otherwise.

proc target::isMade {tid {date ""}} {
    if {[CheckTid $tid] == 0} {
	return -code error "wrong target id ($tid)"
    }
    variable tg_$tid
    upvar 0 tg_$tid target

    if {$target(allwaysMakeIt) == "yes"} {
	return 0
    }
    switch $target(type) {
	file {
	    if {[file exists $target(tid)] == 0} {
		return 0
	    }
	    if {$date != ""} {
		if {[file mtime $target(tid)] >= $date} {
		    return 0
		}
	    }
	}
	tag {
	    if {$target(date) == "undefined"} {
		return 0
	    }
	    if {$date != ""} {
		if {$target(date) >= $date} {
		    return 0
		}
	    }
	}
    }

    # Check each dependency.

    if {[llength $target(dependLst)] != 0} {
	foreach newTid $target(dependLst) {
	    if {[isMade $newTid $target(date)] == 0} {
		return 0
	    }
	}
    } else {
	return 1
    }
}


# target::msg --
# usage : target::msg str
#
#   Set a message in a way determined by the "msgCmd" package variable.
#
# Parameters :
#   str - The string for the message.
#
# Return value :
#   None.

proc target::msg args {
    variable msgCmd

    set theCmdLine [concat $msgCmd $args]
    catch $theCmdLine
}


# target::newPuts -- PRIVATE
# usage : target::newPuts [-nonewline] [-beginofline] str
#
#   Puts the date, several spaces (depending on target level) and a message.
#
# Parameters :
#   str - The string for the message.
#
# Options :
#   -nonewline : No new line at the end of the line.
#   -beginofline : Force the string to be put at the beginning of the line even
#      if target::newPuts was previous called with -nonewline.
#
# Return value :
#   None.

proc target::newPuts args {
    variable targetLevel
    variable isNoNewLine
    variable nCol
    variable currCol

    set willBeNoNewLine no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -nonewline {
		set willBeNoNewLine yes
		set args [lreplace $args 0 0]
	    }
	    -beginofline {
		if {$isNoNewLine == "yes"} {
		    set isNoNewLine no
		    puts ""
		}
		set args [lreplace $args 0 0]
	    }
	    default {
		break
	    }
	}
    }

    if {[llength $args] != 1} {
	return -code error "wrong number of arguments"	
    }
    set msgStr [lindex $args 0]

    if {$isNoNewLine == "no"} {
	catch {exec date +%H.%M.%S} dateStr
	puts -nonewline $dateStr
	puts -nonewline " "
	set currCol 9

	for {set i 0} {$i < $targetLevel} {incr i} {
	    puts -nonewline "  "
	    incr currCol 2
	}
    } else {
	if {($currCol + [string length $msgStr]) > $nCol} {
	    puts ""
	    catch {exec date +%H.%M.%S} dateStr
	    puts -nonewline $dateStr
	    puts -nonewline " "
	    set currCol 9

	    for {set i 0} {$i < $targetLevel} {incr i} {
		puts -nonewline "  "
		incr currCol 2
	    }
	}
    }
    if {$willBeNoNewLine == "no"} {
	puts $msgStr
    } else {
	puts -nonewline $msgStr
	incr currCol [string length $msgStr]
    }

    set isNoNewLine $willBeNoNewLine

    flush stdout

    return
}

