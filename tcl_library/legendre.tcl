# packageName.tcl --
#
#       This file implements the Tcl code for legendre transform.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS: $Id: legendre.tcl,v 1.5 1999/06/25 19:55:47 decoster Exp $
#

package provide legendre 0.0

package require steph 0.0
package require tools 0.0

# legendre --
# usage: legendre signal string
#
#   Compute the legendre transform of a signal. Old version, kept for backward
# compatibility. Use the new command (which name is certainly legendre2).
#
# Parameters:
#   signal - Signal to transform.
#
# Return value:
#   Names of the 3 resulting signals.

proc legendre {zeSig {order 3} {dhName ""}} {
    # h(q)	h$zeSig

    if {[catch [list ginfo $zeSig -list]] == 1} {
	return -code error "can't read \"$zeSig\": no such signal"
    }

    if {$dhName == ""} {
	set dhName dh$zeSig
    }

    set dName [getObjName]
    s2fs $zeSig $dName x 0.05*abs(x)+.01

    set orderLst {}
    for {set i 0} {$i < $order} {incr i} {
	lappend orderLst 0
    }
    set valName [getObjName]
    screate $valName 0 1 $orderLst

    set fName [getObjName]
    screate f$zeSig 0 1 [sgetlst $zeSig -x]
    
    set resName [getObjName]
    polyf $zeSig $dName $valName $resName -fit f$zeSig
    sderiv f$zeSig h$zeSig

    # We verifie that h(q) is always decreasing

    set uuu [getObjName]
    sderiv h$zeSig $uuu
    set i 0
    sigloop $uuu {
        if {$y > 0.0} {
            break
        }
        incr i
    } 

    set imin [expr { $i - 2 }]
    set imax [expr { $i + 1 }]
    set xmin [lindex [sget f$zeSig $imin] 1]  
    set xmax [lindex [sget f$zeSig $imax] 1]  

    sthresh f$zeSig $uuu $xmin $xmax

    set s [getObjName]
    s2fs $uuu $s x .00001

    set newf [getObjName]
    screate $newf 0 1 [sgetlst $zeSig -x]

    sthresh $newf $newf $imin 1000

    set v [getObjName]
    screate $v 0 1 $orderLst
    set r [getObjName]
    polyf $uuu $s $v $r -fit $newf
    set xx [expr { $xmin - 0.001 }]
    sthresh f$zeSig f$zeSig -100 $xx
    scolle f$zeSig $newf f$zeSig
    sderiv f$zeSig h$zeSig

    set temp [getObjName]
    s2fs h$zeSig $temp x y*x

    sthresh f$zeSig f$zeSig -11 10.2

    scut f$zeSig $uuu 0 1

    scomb $temp $uuu x-y $uuu
    smerge h$zeSig $uuu $dhName

    # Clean objects created by getObjName.

    delete $dName
    delete $valName
    delete $resName
    delete $uuu
    delete $s
    delete $newf
    delete $v
    delete $r
    delete $temp

    return [list f$zeSig h$zeSig $dhName]
}


# legendre2 --
# usage: legendre2 signal [-dh str] [-tqfit str] [-hq str] [-qsig str] [-order int] [-fit int]
#
#   Compute the legendre transform of a signal. Here is the formula (and
# notations): D(h) = min (q * h - tau(q)). To estimate the value of h for each
# value of q, we fit tau(q) around the value of q and we take the numerical
# derivative of this fit on q.
#
# Arguments:
#   signal - Signal to transform.
#
# Options:
#   -dh string: Gives the name of D(h) signal. Default is dh$tq, where $tq is
#      the name of tau(q).
#   -tqfit string: Gives the name of the signal that contains the fit of tau(q).
#      Which must be extremly similar to the input signal. Default is fit$tq,
#      where $tq is the name of tau(q).
#   -hq string: Gives the name of h(q) signal. Default is hq$tq, where $tq is
#      the name of tau(q).
#   -qsig string: Gives the name of the signal that contains the values of q for
#      wich we have to compute D(h). Default is q$tq, where $tq is the name of
#      tau(q).
#   -order int: Gives the order of the fit. Default is 3.
#   -fit int: Gives the number of points to take befare and after each q for the
#      fit. Default is 2.
#
# Return value:
#   Names of the 4 resulting signals: D(h), fit of tau(q), h(q) and values of q.

proc legendre2 {tq args} {
    # Default values.

    set dh dh$tq
    set tqfit fit$tq
    set hq hq$tq
    set qsig q$tq
    set order 3

    # Number of points to take before and after each q to estimate the fit.
    set m 2


    # Arguments analysis

    set oldArgs $args
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -dh {
		set dh [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -tqfit {
		set tqfit [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -hq {
		set hq [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -qsig {
		set qsig [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -order {
		set order [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -fit {
		set m [lindex $args 1]
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

    # Treatment.


    set orderSig2 [getObjName]
    set orderSig [getObjName]
    set tmp [getObjName]
    set error [getObjName]
    set qValues [getObjName]

    # polyf command needs a signal that contains "order" points.
    set orderLst {}
    for {set i 0} {$i < $order} {incr i} {
	lappend orderLst 0
    }
    screate $orderSig2 0 1 $orderLst

    set n [ssize $tq]
    set iMin $m
    set iMax [expr { $n-$m }]

    if {$iMax < $iMin} {
	return -code error "signal to transform is too small"
    }

    set qLst [list]
    set hLst [list]
    set DLst [list]
    set tauLst [list]

    for {set i $iMin} {$i < $iMax} {incr i} {
	lassign {tau q} [sget $tq $i]
	lappend qLst $q

	# Get the piece of signal to fit. Around the point q.
	scut $tq $tmp \
		[expr { $i - $m }] \
		[expr { $n - ($i + $m) - 1 }]

	lassign {gah qMin} [sget $tmp 0]
	lassign {gah qMax} [sget $tmp [expr { 2*$m }]]

	s2fs $tmp $error x 0.005*abs(x)+.00001
	scopy $orderSig2 $orderSig

	# To compute the derivative in the point q, we only needs 2 successive
	# points. We get the values of the fit for q and q+0.001
	screate $qValues 0 1 [list $q [expr { $q+0.001 }]]
 	polyf $tmp $error $orderSig gah -fit $qValues

 	sderiv $qValues h
 	lassign {h gah} [sget h 0]
 	lappend hLst $h

 	lassign {tau gah} [sget $qValues 0]
	lappend tauLst $tau

 	lappend DLst [expr { $q*$h - $tau }]
    }

    screate $dh 0 1 $DLst -xy $hLst
    screate $tqfit 0 1 $tauLst -xy $qLst
    screate $hq 0 1 $hLst -xy $qLst
    screate $qsig 0 1 $qLst -xy $qLst

    delete $orderSig
    delete $tmp
    delete $error
    delete $qValues
    delete $orderSig2

    return [list $dh $tqfit $hq $qsig]
}

