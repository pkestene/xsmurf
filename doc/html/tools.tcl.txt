# tools.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: tools.tcl,v 1.5 1999/01/22 14:38:17 decoster Exp $
#

package provide tools 0.0

proc greload {name inf sup} {
    for {set i $inf} {$i <= $sup} {incr i} {
	set num [format "%.3d" $i]
	eload ${name}ext$num
    }
}

proc greload2 {name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.3d" $i]
	eload ${name}$num
    }
}

proc greload3 {name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.2d" $i]
	eload ${name}$num
    }
}

proc gresave {name inf sup} {
    for {set i $inf} {$i <= $sup} {incr i} {
	set num [format "%.3d" $i]
	esave ${name}ext$num
    }
}

proc gresave3 {name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.3d" $i]
	esave ${name}$num
    }
}

proc grmv {name new_name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.3d" $i]
	exec mv ${name}$num ${new_name}$num
    }
}

# lassign --
# usage : lassign list list
#
#   Assign, one by one, a list of values to a list of variables.
#
# Parameters :
#   list - Variable name list.
#   list - Valule list.
#
# Return value :
#   None.

proc lassign {variables values} {
    uplevel 1 [list \
	    foreach $variables $values { break } \
	    ]
}


# objlist --
# usage : objlist expr
#
#   Get the sorted list of smurf object that match an expression.
#
# Parameters :
#   expr - The expression.
#
# Return value :
#   The list.

proc objlist args {
    return [lsort [eval ginfo $args -list]]
}


proc randname {} {
    set c1 [format "%c" [expr 97+int(rand()*26)]]
    set c2 [format "%c" [expr 97+int(rand()*26)]]
    set c3 [format "%c" [expr 97+int(rand()*26)]]
    set c4 [format "%c" [expr 97+int(rand()*26)]]
    set c5 [format "%c" [expr 97+int(rand()*26)]]
    set c6 [format "%c" [expr 97+int(rand()*26)]]
    set name __${c1}
}


# getObjName --
# usage : getObjName
#
#   Get name of an object that is not already used. This command creates an
# "nearly empty" signal to avoid border effects.
#
# Parameters :
#   None.
#
# Return value :
#   The name.

proc getObjName {} {
    set name __obj_[format "%c" [expr 97+int(rand()*26)]]
    while {[catch {ginfo $name -list}] == 0} {
	set name __obj_[format "%c" [expr 97+int(rand()*26)]]
	while {[catch {ginfo $name -list}] == 0 && [string length $name] < 30} {
	    set name $name[format "%c" [expr 97+int(rand()*26)]]
	}
    }

    screate $name 0 1 {}
    return $name
}


# getMin --
# usage: getMin float float
#
#   Get the minimum of to values.
#
# Parameters :
#   2 floats - Values.
#
# Return value :
#   The minimum.

proc getMin {a b} {
    return [expr { ($a)<($b)?($a):($b) }]
}


# getMax --
# usage: getMax float float
#
#   Get the maximum of to values.
#
# Parameters :
#   2 floats - Values.
#
# Return value :
#   The maximum.

proc getMax {a b} {
    return [expr { ($a)>($b)?($a):($b) }]
}

