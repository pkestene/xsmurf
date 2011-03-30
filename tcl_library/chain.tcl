#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#

# Horizontal chain of a set of ext image.
#
proc hchain {name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.3d" $i]
	hsearch ${name}$num
    }
}

proc hchain2 {name inf sup {pas 1}} {
    for {set i $inf} {$i <= $sup} {incr i $pas} {
	set num [format "%.2d" $i]
	hsearch ${name}$num
    }
}
