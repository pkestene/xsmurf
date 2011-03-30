# curvefit.tcl --
#
#       This file implements the Tcl code for fitting curves to signals
#       in general.
#
#   Copyright 2007 University of Maine
#   Written by Andre Khalil
#
#

# last modified by Andre Khalil (2007/05/09).





# sgaussfit --
# usage : sgaussfit signal
#
#  Fit a gaussian curve to a signal
#  Output is mean and sigma
#
# Parameters :
#   signal - signal to treat.
#
# Return value :
#   mean & sigma

proc sgaussfit {sig} {
    set N [lindex [sinfo $sig] 1]
    #puts $N
    sintegrate $sig ss
    set extr [sgetextr ss]
    set smax [lindex $extr 1]
    set smid [expr $smax/2.0]
    set ssigma [expr $smax*0.16]
    set slist [sgetlst ss]
    #Find mean
    for {set i 0} {$i < $N} {incr i +1} {
	if {[lindex $slist $i] > $smid} {
	    set mean $i
	    set i $N
	}
    }
    #Find sigma
    for {set i 0} {$i < $N} {incr i +1} {
	if {[lindex $slist $i] > $ssigma} {
	    set sigmatmp [expr $i-1]
	    set i $N
	}
    }
    set sigma [expr $mean-$sigmatmp]
    return [list $mean $sigma]
}