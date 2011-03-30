# extImageTools.tcl --
#
#       This file implements the Tcl code for some ext-image tools.
#
#   Copyright 2002 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Pierre Kestener.
#
#

package provide extImageTools


# eimoment --
# usage: eimoment ExtImage int [float]
#
#   Compute a given moment of an ext-image (using modulus !)
#
# Parameters:
#   ExtImage   - ExtImage to treat.
#   integer    - Order of the moment.
#   [float]    - extra argument usefull to compute centered moment.
#                default value is zero.
#
# Options:
#
#
# Return value:
#   The moment.

proc eimoment {extimage order {mean 0} args } {
    set m0 [eval "efct $extimage pow(x,0)            $args"]
    set m  [eval "efct $extimage pow(x-$mean,$order) $args"]
    if {$m0!=0} {
	set res [expr $m/$m0]
    } else {
	set res ""
    }

    return $res
}


# ei3Dsmallmoment --
# usage: ei3Dsmallmoment ExtImage int [float]
#
#   Compute a given moment of a 3Dsmall ext-image (using modulus !)
#
# Parameters:
#   ExtImage3Dsmall   - ExtImage to treat.
#   integer           - Order of the moment.
#   [float]           - extra argument usefull to compute centered moment.
#                       default value is zero.
#
# Options:
#
# Return value:
#   The moment.

proc ei3Dsmallmoment {extimage order {mean 0} args } {
    set m0 [eval "efct3Dsmall $extimage pow(x,0)            $args"]
    set m  [eval "efct3Dsmall $extimage pow(x-$mean,$order) $args"]
    if {$m0!=0} {
	set res [expr $m/$m0]
    } else {
	set res ""
    }

    return $res
}


# eimomentlog --
# usage: eimomentlog ExtImage int [float]
#
#   Compute a given moment of an ext-image (using modulus !)
#
# Parameters:
#   ExtImage   - ExtImage to treat.
#   integer    - Order of the moment.
#   [float]    - extra argument usefull to compute centered moment.
#                default value is zero.
#
# Options:
#
#
# Return value:
#   The moment.

proc eimomentlog {extimage order {mean 0} args } {
    set m0 [eval "efct $extimage pow(x,0)                 $args"]
    set m  [eval "efct $extimage pow(log(x)-$mean,$order) $args"]
    if {$m0!=0} {
	set res [expr $m/$m0]
    } else {
	set res ""
    }

    return $res
}


# ei3Dsmallmomentlog --
# usage: ei3Dsmallmomentlog ExtImage int [float]
#
#   Compute a given moment of a 3Dsmall ext-image (using modulus !)
#
# Parameters:
#   ExtImage3Dsmall   - ExtImage to treat.
#   integer           - Order of the moment.
#   [float]           - extra argument usefull to compute centered moment.
#                       default value is zero.
#
# Options:
#
#
# Return value:
#   The moment.

proc ei3Dsmallmomentlog {extimage order {mean 0} args } {
    set m0 [eval "efct3Dsmall $extimage pow(x,0)                 $args"]
    set m  [eval "efct3Dsmall $extimage pow(log(x)-$mean,$order) $args"]
    if {$m0!=0} {
	set res [expr $m/$m0]
    } else {
	set res ""
    }

    return $res
}


# eistats --
# usage: eistats ExtImage int
#
#   Compute successive moments of an image.
#
# Parameters:
#   Image   - Image to treat.
#   integer - Order of the upper moment. Must be greater or equal to 2.
#
# Return value:
#   List of the variance and of the moments.

proc eistats {image order} {
    if {$order < 2} {
	return -code error ""
    }

    for {set k 1} {$k <= $order} {incr k} {
	set res [eimoment $image $k]
        lappend resLst $res
    }

    set variance [expr { sqrt([lindex $resLst 1]-pow([lindex $resLst 0],2)) }]

    return [concat $variance $resLst]
}

