# cascades.tcl --
#
#       This file implements the Tcl code for cascades images commands.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id$
#

package provide cascades


# ---------- Singular cascades with/without fractionnal integration ----------


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
#   Create a signal that contains the theoretical D(h) spectrum of a 2D
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
#   Create a signal that contains the theoretical tau(q) spectrum of a 2D
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
#   Create a signal that contains the theoretical D(h) spectrum of a 2D singular
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



# ---------- Cascades using orthogonal wavelets ----------


# wcGetPhiStar --
# usage: wcGetPhiStar real [real]
#
#   Return an approximation (dichotomy) of Phi* and the value of c3. Phi* verifies
# the following formula:
#                                                
#     sin(2 Phi*)       k                           (tau(2)/2 + 2)
#    ------------- = ------- - 0.5, with k = 2 x cst              .
#        4 Phi*       1 + k
#
# c3 verifies the following formula:
#
#            -(tau(2)/4 + 1)
#    c3 = cst               .
#
# This is used to determine the Phi* angle and the largest scale coefficient
# (c3) used to cascade the XY component on a wavelet base. The value of cst
# reflect the...
#
# Arguments:
#   real   - Value of tau(2).
#   [real] - Value of cst. Default is 2.
#
# Return value:
#   Phi* and c3.

proc wcGetPhiStar {tau2 {cst 2}} {
    # Precision on Phi*.
    set eps 0.0000001

    set k [expr { 2*pow($cst,$tau2/2+2) }]
    set C [expr { $k/(1+$k)-0.5 }]

    if {$C == 0.5} {
	# 0.5 is the limit of sin(2 Phi*)/(4 Phi*) when Phi*->0.

	return 0
    }

    set p1 $eps
    set val1 [expr { sin(2*$p1)/(4*$p1) - $C }]

    set p2 [expr { acos(-1)/2.0 }]
    set val2 [expr { sin(2*$p2)/(4*$p2) - $C }]

    set diff [expr { $p2-$p1 }]

    while {$diff > $eps} {
	set p [expr { ($p2+$p1)/2.0 }]
	set val [expr { sin(2*$p)/(4*$p) - $C }]

	if {$val*$val1 > 0} {
	    set p1 $p
	    set val1 $val
	} else {
	    set p2 $p
	    set val2 $val
	}

	set diff [expr { $p2-$p1 }]
    }

    set c3 [expr { pow($cst,-($tau2/4+1)) }]

    return [list $p1 $c3]
}



# ---------- Log-normal cascade using orthogonal wavelets ----------


# lncGetTau --
# usage: lncGetTau string real real real real [real]
#
#   Create a signal that contains the theoretical tau(q) spectrum of a 2D
# log-normal cascade using orthogonal wavelets. Here is the formula:
#
#                     2
#                    s      2     m q
#     tau(q) = - --------- q  - ------- - 2, with s = sigma and m = mean.
#                 2 ln(2)        ln(2)
#
# Arguments:
#   string - Name of the result.
#   real   - Squared sigma.
#   real   - Mean.
#   real   - First value of q (qMin).
#   real   - Last value of q (qMax).
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result. If qMin equals qMax the result is tau(qMin).

proc lncGetTau {sig sigma2 mean qMin qMax {dq 0.1}} {

    if {$qMin == $qMax} {
	set q $qMin
	return [expr { -${sigma2}*$q*$q/(2*log(2))-${mean}*$q/log(2)-2 }]
    }

    set n [expr { int(($qMax - $qMin)/$dq + 1) }]

    #exprr $sig -${mean}*x-${sigma2}*x*x/2-2 $qMin $qMax $n

    exprr $sig -${sigma2}*x*x/(2*log(2))-${mean}*x/log(2)-2 $qMin $qMax $n

    return $sig
}



# lncGetH --
# usage: lncGetTau string real real real real [real]
#
#   Create a signal that contains the theoretical h(q) of a 2D
# log-normal cascade using orthogonal wavelets. Here is the formula:
#
#                     2
#                   s              m
#     tau(q) = - --------- q  - ------- , with s = sigma and m = mean.
#                  ln(2)         ln(2)
#
# Arguments:
#   string - Name of the result.
#   real   - Squared sigma.
#   real   - Mean.
#   real   - First value of q (qMin).
#   real   - Last value of q (qMax).
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result. If qMin equals qMax the result is h(qMin).

proc lncGetH {sig sigma2 mean qMin qMax {dq 0.1}} {

    if {$qMin == $qMax} {
	set q $qMin
	return [expr { -${sigma2}*$q/(log(2))-${mean}/log(2) }]
    }

    set n [expr { int(($qMax - $qMin)/$dq + 1) }]

    #exprr $sig -${mean}*x-${sigma2}*x*x/2-2 $qMin $qMax $n

    exprr $sig -${sigma2}*x/(log(2))-${mean}/log(2) $qMin $qMax $n

    return $sig
}


# lncGetD --
# usage: lncGetD string real real real real [real]
#
#   Create a signal that contains the theoretical D(h) spectrum of a 2D
# log-normal cascade using orthogonal wavelets. Here is the formula:
#
#                                2
#             - ( h + m / ln(2) )
#     D(h) = ---------------------- + 2, with s = sigma and m = mean.
#                    2
#                 2 s / ln(2)
#
# Arguments:
#   string - Name of the result.
#   real   - Squared sigma.
#   real   - Mean.
#   real   - First value of h (hMin).
#   real   - Last value of h (hMax).
#   real   - Value of the dh. Default is 0.1.
#
# Return value:
#   Name of the result. If hMin equals hMax the result is D(hMin).

proc lncGetD {sig sigma2 mean hMin hMax {dh 0.1}} {

    if {$hMin == $hMax} {
	set h $hMin
	return [expr { -pow($h+${mean}/log(2),2)/(2*${sigma2}/log(2))+2 }]
    }

    set n [expr { int(($hMax - $hMin)/$dh + 1) }]

    exprr $sig -pow(x+${mean}/log(2),2)/(2*${sigma2}/log(2))+2 $hMin $hMax $n
 
    return $sig
}


# lncGetG --
# usage: lncGetG string string real real real real real real [real]
#
#   Compute 2 signals that contains the theoretical mod(G^) and phi(G^). Here is
# the formula:
#
#                          2    2
#     mod(G^(p)) = exp( - s  * p  s(a,a') ),
#
#     phi(G^(p)) = s(a,a') * p * m,
#
#     with s(a,a') = log2 (a/a'), s = sigma  and m = mean.
#
# Arguments:
#   string - Name of signal for mod(G^).
#   string - Name of signal for phi(G^).
#   real   - Squared sigma.
#   real   - Mean.
#   real   - Value of a/a'.
#   real   - First value of p (pMin).
#   real   - Last value of p (pMax).
#   real   - Value of the dp. Default is 0.01.
#
# Return value:
#   None.

proc lncGetG {sigm siga sigma2 mean da pMin pMax {dp 0.01}} {
    set n [expr { int(($pMax - $pMin)/$dp + 1) }]

    exprr $sigm exp(-$sigma2*x*x*log($da)/(2*log(2))) $pMin $pMax $n
    exprr $siga (x*$mean)*log($da)/log(2)  $pMin $pMax $n

    return
}


# ---------- Log-Poisson cascade using orthogonal wavelets ----------


# lpcGetTau --
# usage: lpcGetTau string real real real real [real]
#
#   Create a signal that contains the theoretical tau(q) spectrum of a 2D
# log-Poisson cascade using orthogonal wavelets. Here is the formula:
#
#                 l           q       g
#     tau(q) = ------- ( 1 - b ) - ------- q - 2, with b = beta, g = gamma
#               ln(2)               ln(2)                       and l = lambda.
#
# Arguments:
#   string - Name of the result.
#   real   - Beta.
#   real   - Gamma.
#   real   - Lambda.
#   real   - First value of q (qMin).
#   real   - Last value of q (qMax).
#   real   - Value of the dq. Default is 0.1.
#
# Return value:
#   Name of the result. If qMin equals qMax the result is tau(qMin).

proc lpcGetTau {sig beta gamma lambda qMin qMax {dq 0.1}} {

    if {$qMin == $qMax} {
	set q $qMin
	return [expr { ${lambda}*(1-pow(${beta},$q))/log(2)-${gamma}*$q/log(2)-2 }]
    }

    set n [expr { int(($qMax - $qMin)/$dq + 1) }]

    exprr $sig ${lambda}*(1-pow(${beta},x))/log(2)-${gamma}*x/log(2)-2 \
	    $qMin $qMax $n

    return $sig
}


# lpcGetD --
# usage: lpcGetD string real real real real [real]
#
#   Create a signal that contains the theoretical D(h) spectrum of a 2D
# log-Poisson cascade using orthogonal wavelets. Here is the formula:
#
#                h         g                h + g/ln(2)                  l
#     D(h) = ( ----- + ---------- ) ( ln(----------------) - 1 ) + 2 - -----,
#              ln(b)   ln(2)ln(b)        -l/ln(2) * ln(b)              ln(2)
#
#     with b = beta, g = gamma and l = lambda.
#
# Arguments:
#   string - Name of the result.
#   real   - Beta.
#   real   - Gamma.
#   real   - Lambda.
#   real   - First value of h (hMin).
#   real   - Last value of h (hMax).
#   real   - Value of the dh. Default is 0.1.
#
# Return value:
#   Name of the result. If hMin equals hMax the result is D(hMin).

proc lpcGetD {sig beta gamma lambda hMin hMax {dh 0.1}} {

    if {$hMin == $hMax} {
	set h $hMin
	return [expr { -pow($h+${mean}/log(2),2)/(2*${sigma2}/log(2))+2 }]
    }

    set n [expr { int(($hMax - $hMin)/$dh + 1) }]

    exprr $sig (x/log(${beta})+(${gamma})/(log(2)*log(${beta})))*(log((x+(${gamma})/log(2))/(-${lambda}*log(${beta})/log(2)))-1)+2-${lambda}/log(2) \
	    $hMin $hMax $n

    return $sig
}


# lpcGetG --
# usage: lpcGetG string string real real real real real real [real]
#
#   Compute 2 signals that contains the theoretical mod(G^) and phi(G^). Here is
# the formula:
#
#
#     mod(G^(p)) = exp( s(a,a') l * (cos(p ln(b)) - 1) ),
#
#     phi(G^(p)) = s(a,a') (p * g + l * sin(p ln(b))),
#
#     with s(a,a') = log2 (a/a'), b = beta, g = gamma and l = lambda.
#
# Arguments:
#   string - Name of signal for mod(G^).
#   string - Name of signal for phi(G^).
#   real   - Beta.
#   real   - Gamma.
#   real   - Lambda.
#   real   - Value of a/a'.
#   real   - First value of p (pMin).
#   real   - Last value of p (pMax).
#   real   - Value of the dp. Default is 0.01.
#
# Return value:
#   None.

proc lpcGetG {sigm siga beta gamma lambda da pMin pMax {dp 0.01}} {
    set n [expr { int(($pMax - $pMin)/$dp + 1) }]

    exprr $sigm exp($lambda*(cos(x*log($beta))-1)*log($da)/log(2)) $pMin $pMax $n
    exprr $siga (x*$gamma+$lambda*sin(x*log($beta)))*log($da)/log(2) $pMin $pMax $n

    return
}

