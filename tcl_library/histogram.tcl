# histogram.tcl --
#
#       This file implements the Tcl code for histograms handling in general.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: histogram.tcl,v 1.5 1999/05/22 16:42:18 decoster Exp $
#

# last modified by Pierre Kestener (2000/04/12).


package provide histogram 0.0

# snorm --
# usage : snorm signal
#
#  Normalize a signal so that its integral is 1.
#
# Parameters :
#   signal - signale to treat.
#
# Return value :
#   None

proc snorm {sig} {
    set norm 0.0
    sigloop $sig {
	set norm [expr { $norm + $y }]
    }
    set dx [sgetdx $sig]
    if {$norm != 0.0} {
	sscamult $sig [expr 1.0/($dx*$norm)] $sig
    }
    return
}


# snormall --
# usage : snormall str
#
#  Normalize a serie of signals.
#
# Parameters :
#   string - base name of the signals.
#
# Return value :
#   None

proc snormall {baseName} {
    set lst [ginfo ${baseName}* -list]
    foreach sig $lst {
	snorm $sig
    }
    return
}


# hrescale --
# usage : hrescale real str list
#
#  Rescale a serie of histograms at different scales by scale^alpha. And
# compute the log of the rescale. See source code.
#
# Parameters :
#   real   - alpha : rescaling value.
#   string - base name of the histograms.
#   list   - list of scaleId. The ratio between each successive scale must be 2.
#
# Return value :
#   None.

proc hrescale {alpha baseName args} {
    set scaleRatio 1
	
    foreach scaleId $args {
	set fmtScaleId [format "%.3d" $scaleId]
	sload ${baseName}${fmtScaleId} histo${fmtScaleId}
	set s histo${fmtScaleId}

	#set s ${baseName}${fmtScaleId}
	#set ss h_log_max_line_mod${fmtScaleId}
	#echo $ss
	set scaleR [expr pow($scaleRatio, $alpha)]
	set scaleR2 [expr $alpha*log($scaleRatio)]

	scopy $s r$s
	snorm r$s
	sputdx r$s [expr [sgetdx r$s]/$scaleR]
	sputx0 r$s [expr [sgetx0 r$s]/$scaleR]

	snorm r$s
	h2logh r$s lr$s

	#scopy $ss r$ss
	#sputx0 r$ss [expr [sgetx0 r$ss]-$scaleR2]
	#snorm r$ss
	#h2logh r$ss lr$ss
	#sthresh r$ss r$ss -1 4
	
	set scaleRatio [expr $scaleRatio*2.0]
    }
}


# hrescale2 --
# usage : hrescale2 str str real real
#
#  Rescale an histogram by scale^alpha. Execute a h2logh command.
#
# Parameters:
#   string - Name of the histogram.
#   string - Name of the result.
#   real   - Scale of the histogram.
#   real   - Alpha: rescaling value.
#
# Return value:
#   Name of the result.

proc hrescale2 {src dest scale alpha} {
    set scaleR [expr pow($scale,$alpha)]

    scopy $src $dest
    snorm $dest

    sputdx $dest [expr [sgetdx $dest]/$scaleR]
    sputx0 $dest [expr [sgetx0 $dest]/$scaleR]

    snorm $dest
    h2logh $dest $dest

    return $dest
}


# hrescale3 --
# usage : hrescale3 str str real real
#
#  Rescale an histogram by scale^alpha.
#
# Parameters:
#   string - Name of the histogram.
#   string - Name of the result.
#   real   - Scale of the histogram.
#   real   - Alpha: rescaling value.
#
# Return value:
#   Name of the result.

proc hrescale3 {src dest scale alpha} {
    set scaleR [expr pow($scale,$alpha)]
    scopy $src $dest
    snorm $dest

    sputdx $dest [expr [sgetdx $dest]/$scaleR]
    sputx0 $dest [expr [sgetx0 $dest]/$scaleR]

    snorm $dest

    return $dest
}


proc hrescalearg {alpha baseName {args}} {
    set scaleRatio 1

    foreach scaleId $args {
	set fmtScaleId [format "%.3d" $scaleId]
	set s ${baseName}${fmtScaleId}
	set ss h_log_max_line_mod${fmtScaleId}
	set scaleR [expr pow($scaleRatio, $alpha)]
	set scaleR2 [expr $alpha*log($scaleRatio)]

	scopy $s r$s

	sputdx r$s [expr [sgetdx r$s]]
	sputx0 r$s [expr [sgetx0 r$s]]
	s2fs r$s r$s x y/pow($scaleRatio,$alpha)
#	snorm r$s
	set uu [expr pow($scaleRatio, $alpha)]
	puts $uu
	set scaleRatio [expr $scaleRatio*2.0]
    }
    return
}


# h2hlog --
# usage : h2hlog signal [str]
#
#  Convert an histogram into the histogram of the log. In fact this proc creates
# 2 histograms, one for the positive values and one for the negative values.
#
# Parameters :
#   signal - histogram to treat.
#   string - the basename of the results.
#
# Return value :
#   The list of the name of the 2 created histograms.

proc h2hlog {name {resName ""}} {
    if {$resName == ""} {
	set resName1 hlogpos_${name}
	set resName2 hlogneg${name}
    }

    set xPosLst {}
    set yPosLst {}
    set xNegLst {}
    set yNegLst {}

    set name2 h_${name}
    foreachs ${name2} {
	if {$x > 0} {
	    lappend xPosLst [expr log($x)]
	    lappend yPosLst [expr $y*$x]
	}
	if {$x < 0} {
	    lappend xNegLst [expr log(-$x)]
	    lappend yNegLst [expr $y*(-$x)]
	}
    }
    screate ${resName}p 0 1 $yPosLst -xy $xPosLst
    screate ${resName}m 0 1 $yNegLst -xy $xNegLst

    return [list ${resName}p ${resName}m]
}


# h2hlog2 --
# usage : h2hlog2 signal [str]
#
#  Convert an histogram into the histogram of the log. In fact this proc creates
# 2 histograms, one for the positive values and one for the negative values.
#
# Parameters :
#   signal - histogram to treat.
#   string - the basename of the results.
#
# Return value :
#   The list of the name of the 2 created histograms.

proc h2hlog2 {name {resName ""}} {
    if {$resName == ""} {
	set resName1 hlogpos_${name}
	set resName2 hlogneg${name}
    }

    set xPosLst {}
    set yPosLst {}
    set xNegLst {}
    set yNegLst {}

    foreachs ${name} {
	if {$x > 0} {
	    lappend xPosLst [expr log($x)]
	    lappend yPosLst [expr $y*$x]
	}
	if {$x < 0} {
	    lappend xNegLst [expr log(-$x)]
	    lappend yNegLst [expr $y*(-$x)]
	}
    }
    screate ${resName}p 0 1 $yPosLst -xy $xPosLst
    screate ${resName}m 0 1 $yNegLst -xy $xNegLst

    return [list ${resName}p ${resName}m]
}


# h2logh --
# usage : h2logh signal [str]
#
#  Convert an histogram into the log of the histogram
#
# Parameters :
#   signal - histogram to treat.
#   string - basename of the results.
#
# Return value :
#   The name of the created histogram.

proc h2logh {name {resName ""}} {
    if {$resName == ""} {
	set resName log${name}
    }

    sigloop $name {
	if {$y > 0} {
	    # Don't remove [expr $x] !!
	    lappend xPosLst [expr $x]
	    lappend yPosLst [expr log($y)]
	}
    }

    screate ${resName} 0 1 $yPosLst -xy $xPosLst

    return ${resName}
}


# h2logh10 --
# usage : h2logh signal [str]
#
#  Convert an histogram into the log (base 10) of the histogram
#
# Parameters :
#   signal - histogram to treat.
#   string - basename of the results.
#
# Return value :
#   The name of the created histogram.

proc h2logh10 {name {resName ""}} {
    if {$resName == ""} {
	set resName log10${name}
    }

    sigloop $name {
	if {$y > 0} {
	    # Don't remove [expr $x] !!
	    lappend xPosLst [expr $x]
	    lappend yPosLst [expr log($y)/log(10)]
	}
    }

    screate ${resName} 0 1 $yPosLst -xy $xPosLst

    return ${resName}
}


# h2powh --
# usage : h2powh signal list [str]
#
#  Compute the q-power of an histogram according to :
#    ph(x) = h(x)*pow(abs(x),q)
#
# Parameters :
#   signal - histogram to treat.
#   list   - list of the values of q.
#   string - basename of the results.
#
# Return value :
#   List of the new histograms.

proc h2powh {name qLst {resName ""}} {
    if {$resName == ""} {
	set resName ${name}_pow
    }

    foreach q $qLst {
	set qStr [get_q_str $q]
	set dx [sgetdx $name]
	set x0 [sgetx0 $name]
	set yPosLst ""
	if {$q >= 0} {
	    sigloop $name {
		lappend yPosLst [expr pow(abs($x), $q)*$y]
	    } 
	} else {
	    sigloop $name {
		if {$x == 0} {
		    lappend yPosLst 0
		} else {
		    lappend yPosLst [expr pow(abs($x), $q)*$y]
		}
	    }
	}
	screate ${resName}${qStr} $x0 $dx $yPosLst
	lappend result ${resName}${qStr}
   }
   return $result
}


# h2powh2 --
# usage: h2powh2 signal string real
#
#  Compute the q-power of an histogram according to :
#    ph(x) = h(x)*pow(abs(x),q)
#
# Parameters:
#   signal - histogram to treat.
#   string - name of the result.
#   list   - value of q.
#
# Return value :
#   Name of the result.

proc h2powh2 {name resName q} {
    set qStr [get_q_str $q]
    set dx [sgetdx $name]
    set x0 [sgetx0 $name]
    set yPosLst ""
    if {$q >= 0} {
	sigloop $name {
	    lappend yPosLst [expr pow(abs($x),$q)*$y]
	} 
    } else {
	sigloop $name {
	    if {$x == 0} {
		lappend yPosLst 0
	    } else {
		lappend yPosLst [expr pow(abs($x),$q)*$y]
	    }
	}
    }
    screate ${resName} $x0 $dx $yPosLst

    return $resName
}


# gh2powh --
# usage : gh2powh str list list
#
#  Compute the q-power of histograms at different scales according to :
#    ph(x) = h(x)*pow(abs(x),q)
#
# Parameters :
#   string - base name of the histograms to treat.
#   list   - list of scale id.
#   list   - list of the values of q.
#
# Return value :
#   None

proc gh2powh {baseName scaleIdLst qLst} {
    foreach scaleId $scaleIdLst {
	set scaleIdF [format "%.3d" $scaleId]
	set name ${baseName}${scaleIdF}
	h2powh $name $qLst
    }
    return
}


# hdisp --
# usage : hdisp int int list [list]
#
#  Display different kind of histograms (or signals) at different scales.
#
# Parameters :
#   int    - Number of rows of graphs
#   int    - Number of lines of graphs
#   list   - list of histograms names.
#   [list] - list of scale id's.
#
# Return value :
#   Name of the window.

proc hdisp {nRows nLines nameLst {lst ""}} {
    set completeLst {}
    foreach name $nameLst {
	if {$lst == ""} {
	    set sigLst [ginfo $name* -list]
	} else {
	    set sigLst {}
	    foreach value $lst {
		set new_value [format "%.3d" $value]
		set sigLst [lappend sigLst ${name}${new_value}]
	    }
	}
	set completeLst [lappend completeLst ${sigLst}]
    }
    set code [catch {mdisp $nRows $nLines ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }
    $result setColorsByList {black red green blue yellow brown slateblue}
    set itemList {}
    foreach value $lst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $result setLabelsItemsByList $itemList
    return $result
}


# phdisp --
# usage : phdisp str list list
#
#  Display q-powered histograms at different scales.
#
# Parameters :
#   string - base name of the histograms to treat.
#   list   - list of scale id.
#   list   - list of the values of q.
#
# Return value :
#   Name of the window.

proc phdisp {baseName scaleIdLst qLst} {
    set nRows 2
    set nLines [expr [llength $qLst]/2]
    set completeLst {}
    foreach q $qLst {
	set qStr [get_q_str $q]
	set sigLst {}
	foreach scale $scaleIdLst {
	    set new_scale [format "%.3d" $scale]
	    set sigLst [lappend sigLst ${baseName}${new_scale}_pow${qStr}]
	}
	set completeLst [lappend completeLst ${sigLst}]
    }
    set code [catch {mdisp $nRows $nLines ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }
    #$result setColorsByList {darkgreen darkcyan darkblue slateblue darkviolet}
    $result setColorsByList {black red green blue}
    set itemList {}
    foreach scale $scaleIdLst {
	set itemList [lappend itemlist [list %c $scale]]
    }
    eval $result setLabelsItemsByList $itemList
    set l 0
    set r 0
    foreach q $qLst {
	set newL [format "%.2d" $l]
	set newR [format "%.2d" $r]
	${result}gr${newR}${newL} set_label [list black "h pow $q   "] allSigLabel
	incr l
	if {$l == $nLines} {
	    set l 0
	    set r 1
	}
    }
    return $result
}


# rhdisp --
# usage : rhdisp list list list
#
#  This procedure is used to display a serie of linear and logarithm rescaled
# histograms. The name have the following form : rh_${type}${scaleId} and
# lrh_${type}${scaleId}.
#
# Parameters :
#   list - list of the types.
#   list - list of the rescaling values (alpha).
#   list - list of the scaleId.
#
# Return value :
#   Name of the window.

proc rhdisp {typeLst alphaLst scaleIdLst} {
    set nRows 2
    set nLines [llength $typeLst]
    set completeLst {}
    foreach type $typeLst {
	set sigLst {}
	foreach value $scaleIdLst {
	    set new_value [format "%.3d" $value]
	    set sigLst [lappend sigLst rh_${type}${new_value}]
	}
	set completeLst [lappend completeLst ${sigLst}]
    }
    foreach type $typeLst {
	set lSigLst {}
	foreach value $scaleIdLst {
	    set new_value [format "%.3d" $value]
	    set lSigLst [lappend lSigLst lrh_${type}${new_value}]
	}
	set completeLst [lappend completeLst ${lSigLst}]
    }
    set code [catch {mdisp $nRows $nLines ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }
    #$result setColorsByList {darkgreen darkcyan darkblue slateblue darkviolet}
    $result setColorsByList {black red green blue}
    set itemList {}
    foreach value $scaleIdLst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $result setLabelsItemsByList $itemList
    set l 0
    foreach type $typeLst alpha $alphaLst {
	set newL [format "%.2d" $l]
	${result}gr00$newL set_label [list black "$type, alpha = $alpha   "] allSigLabel
	${result}gr01$newL set_label [list black "Log $type, alpha = $alpha   "] allSigLabel
	incr l
    }
    return $result
}


# smoment --
# usage: smoment Signal int
#
#   Compute a given moment of an signal.
#
# Parameters:
#   Signal   - Signal to treat.
#   integer - Order of the moment.
#
# Return value:
#   The moment.

proc smoment {signal order} {
    mylassign {type n} [sinfo $signal]

    return [expr { [sfct $signal pow(x,$order)]/$n }]
}


# sstats --
# usage: sstats Signal int
#
#   Compute successive moments of an signal.
#
# Parameters:
#   Signal   - Signal to treat.
#   integer - Order of the upper moment. Must be greater or equal to 2.
#
# Return value:
#   List of the variance and of the moments.

proc sstats {signal order} {
    for {set k 1} {$k <= $order} {incr k} {
	set res [smoment $signal $k]
        lappend resLst $res
    }

    set variance [expr { sqrt([lindex $resLst 1]-pow([lindex $resLst 0],2)) }]

    return [concat $variance $resLst]
}


# lisse --
# usage: lisse Signal
#
#   bricolage pour eviter des zeros dans un signal.
#
# Parameters:
#   Signal   - Signal to treat.
#
# Return value:
#   none.

proc lisse {sig} {

set size [ssize $sig]
set first [sfirst $sig]
set last [slast $sig]

for {set i [expr $first + 1]} { $i < $last } {incr i} {
    if { [lindex [sget $sig $i] 0] == 0} {
	set y_avant [lindex [sget $sig [expr $i - 1]] 0]
	set y_apres [lindex [sget $sig [expr $i + 1]] 0]
	set y_mean [expr ($y_apres + $y_avant)/2]
	sset $sig $i $y_avant
    }
}
}
