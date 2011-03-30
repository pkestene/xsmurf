# singCasc.tcl --
#
#       This file implements the Tcl code for singular cascade images
# manipulation.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id$
#

package provide singCasc

# scGetTauEps --
# usage: scGetTauEps string real real real [real]
#
#   Create a signal that contains the theoretical tau(q) specrtum of a 2D
# singular cascade (p-model). 
#
# Arguments:
#   string - Name of the result.
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - First value of q.
#   real   - Last value of q.
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result.

proc scGetTauEps {sig p qMin qMax {dx 0.1}} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    set n [expr { int(($qMax - $qMin)/$dx + 1) }]
    exprr $sig -1-log(pow(2*$p,x)+pow(2-2*$p,x))/log(2) $qMin $qMax $n

    return $sig
}


# scGetDEps --
# usage: scGetDEps string real real real [real]
#
#   Create a signal that contains the theoretical D(h) specrtum of a 2D
# singular cascade (p-model). This spectrum is computing using a legendre
# transform of tau(q).
#
# Arguments:
#   string - Name of the result.
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - First value of q.
#   real   - Last value of q.
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result.

proc scGetDEps {sig p qMin qMax {dx 0.1}} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    scGetTauEps $sig $p $qMin $qMax $dx
    legendre $sig 3 $sig
 
    return $sig
}


# scGetTau --
# usage: scGetTau string real real real real [real]
#
#   Create a signal that contains the theoretical tau(q) specrtum of a 2D
# singular cascade (p-model) with a fractionnary integration. 
#
# Arguments:
#   string - Name of the result.
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - H* of the integration.
#   real   - First value of q.
#   real   - Last value of q.
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result.

proc scGetTau {sig p Hstar qMin qMax {dx 0.1}} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    scGetTauEps $sig $p $qMin $qMax $dx
    s2fs $sig $sig x y+$Hstar*x

    return $sig
}


# scGetD --
# usage: scGetD string real real real real [real]
#
#   Create a signal that contains the theoretical D(h) specrtum of a 2D singular
# cascade (p-model) with a fractionnary integration. This spectrum is computing
# using a legendre transform of tau(q).
#
# Arguments:
#   string - Name of the result.
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - H* of the integration.
#   real   - First value of q.
#   real   - Last value of q.
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result.

proc scGetD {sig p Hstar qMin qMax {dx 0.1}} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    scGetTau $sig $p $Hstar $qMin $qMax $dx
    legendre $sig 3 $sig
 
    return $sig
}


# scGetSpectra --
# usage: scGetSpectra string real real real real [real]
#
#   Create signals that contain the theoretical spectra (tau(a) and D(h)) of a
# 2D singular cascade (p-model) before _and_ after a fractionnary integration.
#
# Arguments:
#   string - Name of the result.
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - H* of the integration.
#   real   - First value of q.
#   real   - Last value of q.
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result.

proc scGetSpectra {baseName p Hstar qMin qMax {dx 0.1}} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    scGetTauEps ${baseName}teq $p $qMin $qMax $dx
    scGetTau ${baseName}tq $p $Hstar $qMin $qMax $dx
    scGetDEps ${baseName}Deh $p $qMin $qMax $dx
    scGetD ${baseName}Dh $p $Hstar $qMin $qMax $dx

    return [list ${baseName}teq ${baseName}tq ${baseName}Deh ${baseName}Dh]
}


# scH2BetaPhy --
# usage: scH2BetaPhy real real
#
#   Compute the value of beta phi. This is needed by the fortran program that
# generates the 2D singular cascade + integration. This program was written by
# A. Davis. ** Warning **: The fortran program need the value of beta phi - 1 (I
# guess it is in memory of 1D cascades).
#
# Arguments:
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - H* of the integration.
#
# Return value:
#   Value of beta phy.

proc scH2BetaPhy {p Hstar} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    set betaEps2 [expr { 1 - log(2*$p*$p - 2*$p + 1)/log(2) }]
    return [expr { 2*$Hstar + $betaEps2 }]
}


# scBetaPhy2H --
# usage: scBetaPhy2H real real
#
#   Compute the value of H*.
#
# Arguments:
#   real   - Value of p. Must be 0 <= p < 1/2.
#   real   - Beta phy.
#
# Return value:
#   Value of H*.

proc scBetaPhy2H {p betaPhy} {
    if {$p < 0 || $p >= 0.5} {
	return -code error "p must be 0 <= p < 1/2"
    }

    set betaEps2 [expr { 1 - log(2*$p*$p - 2*$p + 1)/log(2) }]
    return [expr { ($betaPhy - $betaEps2)/2 }]
}

