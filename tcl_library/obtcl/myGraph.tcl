# myGraph.tcl --
#
#       This file implements the Tcl code for the creation of a graph that will
#       appear in a given canvas. A graph is an entity that displays several
#       signals with the same axes.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: myGraph.tcl,v 1.20 1999/06/25 19:52:25 decoster Exp $
#

# last modified by Pierre Kestener (2000/06/26).

class disp_par
disp_par inherit Base

disp_par method init args {
    instvar signal
    instvar disp_type
    instvar disp_color
    instvar label

    next

    set signal [lindex $args 0]
    set disp_type [lindex $args 1]
    set disp_color [lindex $args 2]
    set label [lindex $args 3]
}

disp_par method setcolor {color} {
    instvar disp_color

    if {[catch {winfo rgb . $color}]} {
	error "unknown color name \"$color\""
    } else {
	set disp_color $color
    }
}

disp_par method getLabelList {} {
    instvar signal
    instvar disp_type
    instvar disp_color
    instvar label

    set itemList {}
    foreach item $label {
	set color [lindex $item 0]
	if {[string compare $color "%c"] == 0} {
	    set color $disp_color
	}
	set string [lindex $item 1]
	regsub %c $string $disp_color string
	regsub %t $string $disp_type string
	regsub %s $string $signal string
	regsub %d $string [describe $signal] string
	lappend itemList [list $color $string]
    }
    return $itemList
}

#
class myGraph

myGraph method init {canvas w h x_pos y_pos sig_lst} {
    # Pathname of the canvas where to draw
    instvar masterCanvas

    # position of the drawable zone in the canvas.
    instvar xPos
    instvar yPos

    # Size of the drawable zone (pixels).
    instvar width
    instvar height

    # Position of the graph in the drawable zone.
    instvar xGrPos
    instvar yGrPos

    # Position of the graph in the canvas.
    instvar xGrPosInCv
    instvar yGrPosInCv

    # Size of the graph.
    instvar grWidth
    instvar grHeight

    # Real coordinates of the graph (corresponding to the signals coordinates).
    instvar xMin
    instvar xMax
    instvar yMin
    instvar yMax

    # List of signal names.
    instvar sigList

    # List of signal names that must be display.
    instvar dispSigList

    # Selected (or current) signal and his index.
    instvar curSig
    instvar curSigIndex

    # Definition of the graph label.
    instvar labelText
    instvar labelColor
    instvar label
    instvar labelFont

    # define the kind of display (all signals, current signal, etc)
    instvar dispMode

    # Define the kind of unit (normal, xlog, ylog or xylog)
    instvar logMode

    next

    # Get args.
    set masterCanvas $canvas
    set width  $w
    set height $h
    set sigList {}
    set dispSigList {}
    set curSigIndex 0
    set xPos $x_pos
    set yPos $y_pos

    # Instvar initialisation.
    set labelText ""
    set label [list {black "signal : "} curSigLabel]
    set labelFont my_font
    #set label [list {black "sig list :"} allSigLabel]
    set dispMode one
    set logMode normal
    foreach sigDesc $sig_lst {
	mylassign {sig type color theLabel} $sigDesc
	if {$type == ""} {
	    set type line
	}
	if {$color == ""} {
	    set color black
	}
	if {$theLabel == ""} {
	    set theLabel [list [list %c %s]]
	}
	disp_par new dispPar${self}$sig $sig $type $color $theLabel
	lappend sigList $sig
    }
    set curSig [lindex $sigList 0]

    set xGrPos 60
    set yGrPos 20
    set xGrPosInCv [expr $x_pos+$xGrPos]
    set yGrPosInCv [expr $y_pos+$yGrPos]
    set grWidth  [expr $width-$xGrPos-10]
    set grHeight [expr $height-$yGrPos*2]

    $self init_disp
}

#
myGraph method destroy {args} {
    instvar masterCanvas
    instvar sigList

    foreach sig $sigList {
	dispPar${self}$sig destroy
    }

    next
}

#
myGraph method setColorsByList {colorList} {
    instvar sigList

    set nSig [llength $sigList]
    set nColor [llength $colorList]
    for {set i 0; set c 0} {$i < $nSig} {incr i;incr c} {
	set sig [lindex $sigList $i]
	if {$c == $nColor} {
	    set c 0
	}
	set color [lindex $colorList $c]
	dispPar${self}$sig setcolor $color
    }
    $self refresh_disp
}

#
myGraph method disp_axes {} {
    instvar masterCanvas
    instvar xGrPosInCv
    instvar yGrPosInCv
    instvar grWidth
    instvar grHeight
    instvar xMin
    instvar xMax
    instvar yMin
    instvar yMax

    # temporary set.
    set width $grWidth
    set height $grHeight
    set xref $xGrPosInCv
    set yref $yGrPosInCv
    set canvas $masterCanvas

    # set ticks positions lists
    set yTicksLists [getrule $yMin $yMax]
    set yMinorTicksList [lindex ${yTicksLists} 0]
    set yMajorTicksList [lindex ${yTicksLists} 1]
    set xTicksLists [getrule $xMin $xMax]
    set xMinorTicksList [lindex ${xTicksLists} 0]
    set xMajorTicksList [lindex ${xTicksLists} 1]

    # make sure we have the current scale factors etc
    set xdelta [expr double($xMax-$xMin)]
    set ydelta [expr double($yMax-$yMin)]
    set xfactor [expr double($width)/$xdelta]
    set yfactor [expr double($height)/$ydelta]

    # set some layout parameters
    set majorTicksLength 10
    set minorTicksLength 5
    set ticksLabelFont my_font
    set textOffset 4

    set yMin_c [expr $height+$yref]
    set yMax_c [expr $yref]
    set xMin_c [expr $xref]
    set xMax_c [expr $width+$xref]

    # clean up any existing axes
    $canvas delete -withtag ${self}Axis

    # draw the outer box
    $canvas create line \
	    $xMin_c $yMin_c \
	    $xMin_c $yMax_c \
	    $xMax_c $yMax_c \
	    $xMax_c $yMin_c \
	    $xMin_c $yMin_c \
	    -fill black \
	    -width 0.1p \
	    -tag ${self}Axis

    # draw Y axis major ticks and ticks labels
    foreach t ${yMajorTicksList} {
	set y [expr int(($yMax-$t)*$yfactor)+$yref]
	$canvas create line \
		$xMin_c $y [expr $xMin_c+${majorTicksLength}] $y \
		-tag ${self}Axis
	$canvas create line \
		$xMax_c $y [expr $xMax_c-${majorTicksLength}] $y \
		-tag ${self}Axis
	$canvas create text \
		[expr $xMin_c-$textOffset] $y \
		-text [format "%.3g" $t] \
		-anchor e \
		-tag ${self}Axis \
		-font ${ticksLabelFont}
    }

    # draw Y axis minor ticks
    foreach t ${yMinorTicksList} {
	if {[lsearch ${yMajorTicksList} $t] == -1} {
	    set y [expr int(($yMax-$t)*$yfactor)+$yref]
	    $canvas create line \
		    $xMin_c $y [expr $xMin_c+${minorTicksLength}] $y \
		    -tag ${self}Axis
	    $canvas create line \
		    $xMax_c $y [expr $xMax_c-${minorTicksLength}] $y \
		    -tag ${self}Axis
	}
    }

    # draw X axis major ticks and ticks labels
    foreach t ${xMajorTicksList} {
	set x [expr int((-$xMin+$t)*$xfactor)+$xref]
	$canvas create line \
		$x $yMin_c $x [expr $yMin_c-${majorTicksLength}] \
		-tag ${self}Axis
	$canvas create line \
		$x $yMax_c $x [expr $yMax_c+${majorTicksLength}] \
		-tag ${self}Axis
	$canvas create text \
		$x [expr $yMin_c+$textOffset] \
		-text [format "%.3g" $t] \
		-anchor n \
		-tag ${self}Axis \
		-font ${ticksLabelFont}
    }

    # draw X axis minor ticks
    foreach t ${xMinorTicksList} {
	if {[lsearch ${xMajorTicksList} $t] == -1} {
	    set x [expr int((-$xMin+$t)*$xfactor)+$xref]
	    $canvas create line \
		    $x $yMin_c $x [expr $yMin_c-${minorTicksLength}] \
		    -tag ${self}Axis
	    $canvas create line \
		    $x $yMax_c $x [expr $yMax_c+${minorTicksLength}] \
		    -tag ${self}Axis
	}
    }

    $canvas itemconfigure ${self}Axis -width 0.1m
    $canvas addtag ${self} withtag ${self}Axis
}

# Convert a real absciss into an index on the drawable zone.
myGraph method x2i {x} {
    instvar grWidth
    instvar xMin xMax
    instvar xGrPosInCv

    return [expr int(($x-$xMin)*$grWidth/($xMax-$xMin))+$xGrPosInCv]
}

# Convert a real ordinate into an index on the drawable zone.
myGraph method y2j {y} {
    instvar grHeight
    instvar yMin yMax
    instvar yGrPosInCv

    return [expr $grHeight-int(($y-$yMin)*$grHeight/($yMax-$yMin))+$yGrPosInCv]
}

# Convert an index on the drawable zone into a real absciss.
myGraph method i2x {i} {
    instvar grWidth
    instvar xMin xMax
    instvar xGrPosInCv

    return [expr $xMin+($i-$xGrPosInCv)*($xMax-$xMin)/(1.0*$grWidth)]
}

# Convert an index on the drawable zone into a real ordinate.
myGraph method j2y {j} {
    instvar grHeight
    instvar yMin yMax
    instvar yGrPosInCv

    return [expr $yMin+($grHeight-$j+$yGrPosInCv)*($yMax-$yMin)/(1.0*$grHeight)]
}

#
myGraph method convert_coordinates {x y} {
    instvar grWidth grHeight
    instvar xMin xMax yMin yMax

    set new_x [$self i2x $x]
    set new_y [$self j2y $y]
    return [list $new_x $new_y]
}

#
myGraph method getI {x} {
    if {[regexp {.*s$} $x] == 1} {
	set newX [string range $x 0 [expr [string length $x]-2]]
	set i [$self x2i $newX]
    } else {
	set i $x
    }
    return $i
}

#
myGraph method getJ {y} {
    if {[regexp {.*s$} $y] == 1} {
	set newY [string range $y 0 [expr [string length $y]-2]]
	set j [$self y2j $newY]
    } else {
	set j $y
    }
    return $j
}

#
myGraph method line {x1 y1 x2 y2 {color black}} {
    instvar masterCanvas

    set i1 [$self getI $x1]
    set j1 [$self getJ $y1]
    set i2 [$self getI $x2]
    set j2 [$self getJ $y2]

    eval "$masterCanvas create line \
	    $i1 $j1 $i2 $j2 \
	    -fill $color \
	    -tags {${self}Curves $self}"
    return "$i1 $j1 $i2 $j2"
}

# Add a signal to the signal list.
myGraph method add {sig {index end}} {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    set is_in [lsearch -exact $sigList $sig]
    if {$is_in == -1} {
	set sigList [linsert $sigList $index $sig]
	disp_par new dispPar${self}$sig $sig line black {%c {%s - %d}}
    }
    $self init_disp

    return $sig
}

# Forward the selected signal in the list.
myGraph method + {{val 1}} {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    if {$curSigIndex < [expr [llength $sigList]-$val]} {
	incr curSigIndex $val
	set curSig [lindex $sigList $curSigIndex]
    }
    return $curSig
}

# Backward the selected signal in the list.
myGraph method - {{val 1}} {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    if {$curSigIndex >= $val} {
	incr curSigIndex -$val
	set curSig [lindex $sigList $curSigIndex]
    }
    return $curSig
}

# Forward the selected signal in the list. The graph is init if args is
# "-init", otherwise the graph is refresh.
myGraph method next_signal args {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    set old_curSig $curSig
    if {$old_curSig != [$self +]} {
	if {[string compare $args "-init"]} {
	    $self refresh_disp
	} else {
	    $self init_disp
	}
    }
    return $curSig
}

# Backward the selected signal in the list. The graph is init if args is
# "-init", otherwise the graph is refresh.
myGraph method prev_signal args {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    set old_curSig $curSig
    if {$old_curSig != [$self -]} {
	if {[string compare $args "-init"]} {
	    $self refresh_disp
	} else {
	    $self init_disp
	}
    }
    return $curSig
}

# Change the selected signal by its name. Return the name of selected signal.
# Return 0 if the guven name is not in the list.
myGraph method current {{sig ""}} {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    set index 0
    if {$sig == ""} {
	return $curSig
    } else {
	foreach signal $sigList {
	    if {[string compare $signal $sig] == 0} {
		set curSigIndex $index
		set curSig $sig
		$self set_new_extrema
		$self refresh_disp

		return $curSig
	    }
	    incr index
	}
    }
    return 0
}

# Find the nearest displayed signal from [x,y] as index on the drawable zone.
myGraph method nearest_signal {x y} {
    instvar grWidth grHeight
    instvar xMin xMax yMin yMax
    instvar sigList
    instvar dispMode
    instvar curSig
    instvar curSigIndex

    set x_y [$self convert_coordinates $x $y]
    set x [lindex $x_y 0]
    set y [lindex $x_y 1]

    set signal [lindex $sigList 0]
    set diff_min [expr abs($y-[sget $signal [lindex [sgetindex $signal $x] 0]])]
    set nearest $signal
    foreach signal $sigList {
	set diff [expr abs($y-[sget $signal [lindex [sgetindex $signal $x] 0]])]
	if {$diff < $diff_min} {
	    set nearest $signal
	    set diff_min $diff
	}
    }
    return $nearest
}

# Find the line that fits the signal near [x,y]. If both of te values are -1,
# the fitted signal is the selected one.
# *** REWRITE THIS METHOD ***
myGraph method sig_fit {{x -1} {y -1} {isChi2 no} } {
    instvar masterCanvas
    instvar grWidth grHeight
    instvar xMin xMax yMin yMax
    instvar sigList
    instvar dispMode
    instvar curSig
    instvar curSigIndex

    if {$x != -1 &&$y != -1 && [string compare $dispMode "one"]} {
	set signal [$self nearest_signal $x $y]
    } else {
	set signal $curSig
    }

    set fit [sfit $signal [expr $xMin-(0.00001*($xMax-$xMin))] [expr $xMax+(0.00001*($xMax-$xMin))]]
    set a    [lindex $fit 0]
    set b    [lindex $fit 1]
    set chi2 [lindex $fit 4]

    set x1 $xMin
    set x2 $xMax
    set y1 [expr ($a*$x1+($b))]
    if {$y1 < $yMin} {
	set y1 $yMin
	set x1 [expr ($y1-$b)/$a]
    }
    set y1 [expr ($a*$x1+($b))]
    if {$y1 > $yMax} {
	set y1 $yMax
	set x1 [expr ($y1-$b)/$a]
    }
    set y2 [expr ($a*$x2+($b))]
    if {$y2 < $yMin} {
	set y2 $yMin
	set x2 [expr ($y2-$b)/$a]
    }
    set y2 [expr ($a*$x2+($b))]
    if {$y2 > $yMax} {
	set y2 $yMax
	set x2 [expr ($y2-$b)/$a]
    }

    set i1 [$self x2i $x1]
    set i2 [$self x2i $x2]
    set j1 [$self y2j $y1]
    set j2 [$self y2j $y2]

    eval "$masterCanvas create line \
	    $i1 $j1 $i2 $j2 \
	    -fill blue \
	    -tags {${self}Curves $self} \
	    -width 2"

    set line [expr ($j2-$j1)*1.0/($i2-$i1)]
    if {abs($line) > 0.2} {
	set textXPos [expr $i1+20/abs($line)]
	set textYPos  [expr $j1]
    } elseif {$line > 0} {
	set textXPos [expr $i1+10]
	set textYPos  [expr $j1+20]
    } else {
	set textXPos [expr $i1+10]
	set textYPos  [expr $j1-20]
    }
    if {$line > 0} {
	set textAnchor nw
    } else {
	set textAnchor sw
    }

    if {$isChi2 == "yes"} {
	eval "$masterCanvas create text \
		$textXPos $textYPos \
		-text [format "%.4g" $chi2] \
		-fill blue \
		-anchor $textAnchor \
		-tags {${self}Curves $self}"
    } else {
	eval "$masterCanvas create text \
		$textXPos $textYPos \
		-text [format "%.4g" $a] \
		-fill blue \
		-anchor $textAnchor \
		-tags {${self}Curves $self} \
		-font {times 14 bold}"
    }
    #set textYPos  [expr $textYPos - 15]
    #eval "$masterCanvas create text \
#	    $textXPos $textYPos \
#	    -text [format "%.4g" $b] \
#	    -fill blue \
#	    -anchor $textAnchor \
#	    -tags {${self}Curves $self}"
#    set textYPos  [expr $textYPos - 15]
#    eval "$masterCanvas create text \
#	    $textXPos $textYPos \
#	    -text [format "chi2 %.4g" $chi2] \
#    -fill blue \
#	    -anchor $textAnchor \
#	    -tags {${self}Curves $self}"

    return "$a $b"
}

# Change the value of dispMode.
myGraph method set_disp_mode {mode} {
    instvar dispMode

    set dispMode $mode
    $self refresh_disp
    return $dispMode
}

# Change the value of logMode.
myGraph method set_log_mode {mode} {
    instvar logMode

    if {[lsearch {normal xlog ylog xylog} $mode] != -1} {
	if {($logMode == "xlog") && ($mode == "ylog")} {
	    set logMode xylog
	} elseif {($logMode == "ylog") && ($mode == "xlog")} {
	    set logMode xylog
	} else {
	    set logMode $mode
	}
	$self init_disp
    }

    return $logMode
}

# Init the real coordinates of the graph box.
myGraph method init_values {} {
    instvar curSig
    instvar xMin xMax yMin yMax
    instvar dispMode
    instvar logMode
    instvar sigList
    instvar dispSigList

    switch $dispMode {
	one {
	    if {[sgettype $curSig] != 0} {
		set signal_x0 [sgetx0 $curSig]
		set signal_dx [sgetdx $curSig]
		set signal_size [ssize $curSig]
		set xMin $signal_x0
		set xMax [expr $signal_x0+(${signal_size}-1)*${signal_dx}]
		
		set tmp_list [sgetextr $curSig]
		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig]

		set xMin [lindex $tmp_list 0]
		set xMax [lindex $tmp_list 1]
		set yMin [lindex $tmp_list 2]
		set yMax [lindex $tmp_list 3]
	    }
	}
	all {
	    if {[sgettype $curSig] != 0} {
		set signal_x0 [sgetx0 $curSig]
		set signal_dx [sgetdx $curSig]
		set signal_size [ssize $curSig]
		set xMin $signal_x0
		set xMax [expr $signal_x0+(${signal_size}-1)*${signal_dx}]
		
		set tmp_list [sgetextr $curSig]
		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig]

		set xMin [lindex $tmp_list 0]
		set xMax [lindex $tmp_list 1]
		set yMin [lindex $tmp_list 2]
		set yMax [lindex $tmp_list 3]
	    }
	    foreach signal $sigList {
		if {[sgettype $signal] != 0} {
		    set signal_x0 [sgetx0 $signal]
		    set signal_dx [sgetdx $signal]
		    set signal_size [ssize $signal]
		    set tmp_xMin $signal_x0
		    if {$xMin > $tmp_xMin} {set xMin $tmp_xMin}
		    set tmp_xMax [expr $signal_x0+(${signal_size}-1)*${signal_dx}]
		    if {$xMax < $tmp_xMax} {set xMax $tmp_xMax}

		    set tmp_list [sgetextr $signal]
		    set tmp_yMin [lindex $tmp_list 0]
		    if {$yMin > $tmp_yMin} {set yMin $tmp_yMin}
		    set tmp_yMax [lindex $tmp_list 1]
		    if {$yMax < $tmp_yMax} {set yMax $tmp_yMax}
		} else {
		    set tmp_list [sgetextr $signal]
		    
		    set tmp_xMin [lindex $tmp_list 0]
		    set tmp_xMax [lindex $tmp_list 1]
		    set tmp_yMin [lindex $tmp_list 2]
		    set tmp_yMax [lindex $tmp_list 3]
		    if {$xMin > $tmp_xMin} {
			set xMin $tmp_xMin
		    }
		    if {$xMax < $tmp_xMax} {
			set xMax $tmp_xMax
		    }
		    if {$yMin > $tmp_yMin} {
			set yMin $tmp_yMin
		    }
		    if {$yMax < $tmp_yMax} {
			set yMax $tmp_yMax
		    }
		}
	    }
	}
	one_and_list {
	    if {[sgettype $curSig] != 0} {
		set signal_x0 [sgetx0 $curSig]
		set signal_dx [sgetdx $curSig]
		set signal_size [ssize $curSig]
		set xMin $signal_x0
		set xMax [expr $signal_x0+(${signal_size}-1)*${signal_dx}]
		
		set tmp_list [sgetextr $curSig]
		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig]

		set xMin [lindex $tmp_list 0]
		set xMax [lindex $tmp_list 1]
		set yMin [lindex $tmp_list 2]
		set yMax [lindex $tmp_list 3]
	    }
	    foreach signal $dispSigList {
		if {[sgettype $signal] != 0} {
		    set signal_x0 [sgetx0 $signal]
		    set signal_dx [sgetdx $signal]
		    set signal_size [ssize $signal]
		    set tmp_xMin $signal_x0
		    if {$xMin > $tmp_xMin} {set xMin $tmp_xMin}
		    set tmp_xMax [expr $signal_x0+(${signal_size}-1)*${signal_dx}]
		    if {$xMax < $tmp_xMax} {set xMax $tmp_xMax}

		    set tmp_list [sgetextr $signal]
		    set tmp_yMin [lindex $tmp_list 0]
		    if {$yMin > $tmp_yMin} {set yMin $tmp_yMin}
		    set tmp_yMax [lindex $tmp_list 1]
		    if {$yMax < $tmp_yMax} {set yMax $tmp_yMax}
		} else {
		    set tmp_list [sgetextr $signal]
		    
		    set tmp_xMin [lindex $tmp_list 0]
		    set tmp_xMax [lindex $tmp_list 1]
		    set tmp_yMin [lindex $tmp_list 2]
		    set tmp_yMax [lindex $tmp_list 3]
		    if {$xMin > $tmp_xMin} {
			set xMin $tmp_xMin
		    }
		    if {$xMax < $tmp_xMax} {
			set xMax $tmp_xMax
		    }
		    if {$yMin > $tmp_yMin} {
			set yMin $tmp_yMin
		    }
		    if {$yMax < $tmp_yMax} {
			set yMax $tmp_yMax
		    }
		}
	    }
	}
    }
    switch $logMode {
	xlog {
	    set xMin [expr log($xMin)]
	    set xMax [expr log($xMax)]
	}
	ylog {
	    set yMin [expr log($yMin)]
	    set yMax [expr log($yMax)]
	}
	xylog {
	    set xMin [expr log($xMin)]
	    set xMax [expr log($xMax)]
	    set yMin [expr log($yMin)]
	    set yMax [expr log($yMax)]
	}
    }
    if {$xMin == $xMax} {
	set xMin [expr $xMin-0.1]
	set xMax [expr $xMax+0.1]
    }
    if {$yMin == $yMax} {
	set yMin [expr $yMin-0.1]
	set yMax [expr $yMax+0.1]
    }
    #set deltaX [expr $xMax-$xMin]
    #set xMin [expr $xMin-0.05*$deltaX]
    #set xMax [expr $xMax+0.05*$deltaX]
    #set deltaY [expr $yMax-$yMin]
    #set yMin [expr $yMin-0.05*$deltaY]
    #set yMax [expr $yMax+0.05*$deltaY]

    #set yMin 0
    #set yMax 5000
    return "$xMin $xMax $yMin $yMax"
}


# Init the real coordinates of the graph box.
# tres rudimentaire pour l'instant
myGraph method init_values_user {xMin0 xMax0 yMin0 yMax0 } {
    instvar curSig
    instvar xMin xMax yMin yMax
    instvar dispMode
    instvar logMode
    instvar sigList
    instvar dispSigList

    
    set xMin $xMin0
    set xMax $xMax0
    set yMin $yMin0
    set yMax $yMax0
    return "$xMin $xMax $yMin $yMax"
}

# pour la taille de la fenetre
# ne marche pas pour le moment
myGraph method set_win_dim {myheight mywidth } {
    instvar curSig
    instvar xMin xMax yMin yMax
    instvar dispMode
    instvar logMode
    instvar sigList
    instvar dispSigList
    instvar grWidth grHeight
    instvar gridWidth gridHeight
    instvar cvWidth cvHeight

#    set grHeight $myheight
#    set grWidth $mywidth
#    set cvWidth [expr $gridWidth*$grWidth]
#    set cvHeight [expr $mgLabelHeight+$gridHeight*$grHeight]
    set cvWidth 500
    set cvHeight 800

    $self refresh_viewer
    return "$cvHeight $cvWidth"

}



#
myGraph method set_new_extrema {} {
    instvar signal_size curSig
    instvar xMin xMax yMin yMax
    instvar dispMode
    instvar logMode
    instvar sigList
    instvar dispSigList

    if {($logMode == "xlog") || ($logMode == "xylog")} {
	set newXMin [expr exp($xMin)]
	set newXMax [expr exp($xMax)]
    } else {
	set newXMin $xMin
	set newXMax $xMax
    }

    switch $dispMode {
	one {
	    if {[sgettype $curSig] != 0} {
		set indexMin [lindex [sgetindex $curSig $newXMin] 0]
		set indexMax [lindex [sgetindex $curSig $newXMax] 0]
		set tmp_list [sgetextr $curSig -index $indexMin $indexMax]

		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig -x $newXMin $newXMax]
		if {[lindex $tmp_list 2] < [lindex $tmp_list 3]} {
		    set yMin [lindex $tmp_list 2]
		    set yMax [lindex $tmp_list 3]
		}
	    }
	}
	all {
	    if {[sgettype $curSig] != 0} {
		set indexMin [lindex [sgetindex $curSig $newXMin] 0]
		set indexMax [lindex [sgetindex $curSig $newXMax] 0]
		set tmp_list [sgetextr $curSig -index $indexMin $indexMax]

		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig -x $newXMin $newXMax]
		if {[lindex $tmp_list 2] < [lindex $tmp_list 3]} {
		    set yMin [lindex $tmp_list 2]
		    set yMax [lindex $tmp_list 3]
		}
	    }
	    foreach signal $sigList {
		if {[sgettype $signal] != 0} {
		    set indexMin [lindex [sgetindex $signal $newXMin] 0]
		    set indexMax [lindex [sgetindex $signal $newXMax] 0]
		    set tmp_list [sgetextr $signal -index $indexMin $indexMax]
		    
		    set tmp_yMin [lindex $tmp_list 0]
		    set tmp_yMax [lindex $tmp_list 1]
		} else {
		    set tmp_list [sgetextr $signal -x $newXMin $newXMax]
		    set tmp_yMin [lindex $tmp_list 2]
		    set tmp_yMax [lindex $tmp_list 3]
		}
		if {$yMin > $tmp_yMin} {set yMin $tmp_yMin}
		if {$yMax < $tmp_yMax} {set yMax $tmp_yMax}
	    }
	}
	one_and_list {
	    if {[sgettype $curSig] != 0} {
		set indexMin [lindex [sgetindex $curSig $newXMin] 0]
		set indexMax [lindex [sgetindex $curSig $newXMax] 0]
		set tmp_list [sgetextr $curSig -index $indexMin $indexMax]

		set yMin [lindex $tmp_list 0]
		set yMax [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $curSig -x $newXMin $newXMax]
		if {[lindex $tmp_list 2] < [lindex $tmp_list 3]} {
		    set yMin [lindex $tmp_list 2]
		    set yMax [lindex $tmp_list 3]
		}
	    }
	    foreach signal $dispSigList {
		if {[sgettype $signal] != 0} {
		    set indexMin [lindex [sgetindex $signal $newXMin] 0]
		    set indexMax [lindex [sgetindex $signal $newXMax] 0]
		    set tmp_list [sgetextr $signal -index $indexMin $indexMax]
		    
		    set tmp_yMin [lindex $tmp_list 0]
		    set tmp_yMax [lindex $tmp_list 1]
		} else {
		    set tmp_list [sgetextr $signal -x $newXMin $newXMax]
		    set tmp_yMin [lindex $tmp_list 2]
		    set tmp_yMax [lindex $tmp_list 3]
		}
		if {$yMin > $tmp_yMin} {set yMin $tmp_yMin}
		if {$yMax < $tmp_yMax} {set yMax $tmp_yMax}
	    }
	}
    }
    switch $logMode {
	ylog {
	    set yMin [expr log($yMin)]
	    set yMax [expr log($yMax)]
	}
	xylog {
	    set yMin [expr log($yMin)]
	    set yMax [expr log($yMax)]
	}
    }
    if {$xMin == $xMax} {
	set xMin [expr $xMin-0.1]
	set xMax [expr $xMax+0.1]
    }
    if {$yMin == $yMax} {
	set yMin [expr $yMin-0.1]
	set yMax [expr $yMax+0.1]
    }

    return "$xMin $xMax $yMin $yMax"
}

# Cut part of the signal that is in the left to an index.
myGraph method left_cut {i} {
    instvar xMin xMax yMin yMax
    instvar curSig

    set tmp [$self i2x $i]
    if {($tmp > $xMin) && ($tmp < $xMax)} {
	set xMin $tmp
	$self set_new_extrema
	$self refresh_viewer
    }
    return $xMin
}

# Cut part of the signal that is in the right to an index.
myGraph method right_cut {i} {
    instvar xMin xMax yMin yMax
    instvar curSig

    set tmp [$self i2x $i]
    if {($tmp > $xMin) && ($tmp < $xMax)} {
	set xMax $tmp
	$self set_new_extrema
	$self refresh_viewer
    }
    return $xMax
}

# Initialize the box coordinates and refresh graph display.
myGraph method init_disp {} {
    $self init_values
    $self refresh_viewer
}

# Initialize the box coordinates (new values) and refresh graph display.
myGraph method set_box_coord {xMin0 xMax0 yMin0 yMax0} {
    $self init_values_user $xMin0 $xMax0 $yMin0 $yMax0
    $self refresh_viewer
}

# Refresh graph display.
myGraph method refresh_disp {} {
    $self refresh_viewer
}

# This method displays the label of the graph acording to the pattern defined by
# <label> instvar. The label consists of a text line which width doesn't exceed
# the size of the graph (i.e. <grWidth> instvar).
#
myGraph method disp_label {} {
    instvar masterCanvas
    instvar xMin xMax yMin yMax
    instvar grWidth grHeight
    instvar curSig
    instvar dispMode
    instvar sigList
    instvar labelText
    instvar labelColor
    instvar label
    instvar labelFont
    instvar xGrPosInCv yGrPosInCv

    $masterCanvas delete -withtag ${self}Label

    # <x> and <y> store the position of the current item.
    set x $xGrPosInCv
    set y [expr $yGrPosInCv-10]

    # set the item list depending on the values of <label> elements.
    set itemList {}
    foreach item $label {
	switch $item {
	    curSigLabel {
		# item(s) defined by the current signal.
		set itemList \
			[concat $itemList [dispPar${self}$curSig getLabelList]]
	    }
	    allSigLabel {
		# list of items defined by each signals from <sigList>.
		foreach sig $sigList {
		    set itemList \
			    [concat $itemList [dispPar${self}$sig getLabelList]]
		}
	    }
	    default {
		# other items.
		lappend itemList $item
	    }
	}
    }
    set tagsList [list ${self}Label $self]
    foreach item $itemList {
	set color  [lindex $item 0]
	if {[catch "winfo rgb . $color"] != 0} {
	    # The required color is not valid.
	    set color black
	}
	set string [lindex $item 1]
	set curLabel [$masterCanvas create text \
		$x $y \
		-text $string \
		-fill $color \
		-font $labelFont \
		-anchor w \
		-tags $tagsList]
	set boxValues [$masterCanvas bbox $curLabel]
	if {[lindex $boxValues 2] > [expr $xGrPosInCv+$grWidth-20]} {
	    # The new item ends over the graph limit.
	    $masterCanvas delete $curLabel
	    $masterCanvas create text \
		    $x $y \
		    -text "..." \
		    -fill black \
		    -font {times 20 bold} \
		    -anchor w \
		    -tags $tagsList
	    #-font $labelFont
	    #-font {times 20 bold} 
	    # Stop the items display
	    break
	}
	incr x [expr [lindex $boxValues 2]-[lindex $boxValues 0]+2]
    }
    return
}

#
myGraph method disp_label2 {} {
    instvar masterCanvas
    instvar xMin xMax yMin yMax
    instvar grWidth grHeight
    instvar curSig
    instvar dispMode
    instvar sigList
    instvar labelText
    instvar labelColor
    instvar xGrPosInCv yGrPosInCv

    if {$labelText == ""} {
	set lbl_text $curSig
	set lbl_color [dispPar${self}$curSig set disp_color]
    } else {
	set lbl_text $labelText
	set lbl_color $labelColor
    }
    $masterCanvas delete -withtag ${self}Label
    eval "$masterCanvas create text \
	    [expr $xGrPosInCv+$grWidth/2] [expr $yGrPosInCv-10] \
	    -text $lbl_text \
	    -fill $lbl_color \
	    -font {times 35 bold} \
	    -tags {${self}Label $self}"
    return
}

myGraph method set_label args {
    instvar label

    set label $args
    $self disp_label

    return
}
# 
myGraph method set_label2 {{color black} {str ""}} {
    instvar labelText
    instvar labelColor

    set labelText $str
    set labelColor $color
    $self disp_label2

    return
}

# syntax for lst : {item1 item2...}
# an item has the form {color string}
#   color is :
#     - a valid color.
#     - %c : the label color will be the current display color of the signal.
#     - anything else : the label color will be black.
#   string can contain :
#     - %c : the current display color.
#     - %t : the current display type.
#     - %s : the current signal name.
# 
myGraph method setsiglabel {sig lst} {
    instvar sigList

    set is_in [lsearch -exact $sigList $sig]
    if {$is_in == -1} {
	error "signal \"$sig\" is not in graph"
    }
    dispPar${self}$sig set label $lst    
    $self disp_label

    return
}

#setLabelsItemsByList {red uhgjhg} {blue jjjjjjjj}
myGraph method setLabelsItemsByList args {
    instvar sigList
    instvar label

    set nSig [llength $sigList]
    set nItem [llength $args]
    if {$nSig > $nItem} {
	set max $nItem
    } else {
	set max $nSig
    }
    for {set i 0} {$i < $max} {incr i} {
	set sig [lindex $sigList $i]
	set item [lindex $args $i]
	dispPar${self}$sig set label [list "$item"]
    }
    $self disp_label
}

# 
myGraph method refresh_viewer {} {
    instvar masterCanvas
    instvar xMin xMax yMin yMax
    instvar grWidth grHeight
    instvar curSig
    instvar dispMode
    instvar sigList
    instvar dispSigList
    instvar xGrPosInCv yGrPosInCv

    global bmp_num

    $masterCanvas delete -withtag ${self}
    $masterCanvas delete -withtag ${self}Curves
    #$self disp_axes $masterCanvas \
	    #$xMin $xMax $yMin $yMax \
	    #$grWidth $grHeight \
	    #$xGrPosInCv $yGrPosInCv
    $self disp_axes
    $self disp_label
    switch $dispMode {
	one {
	    $self plot_sig $curSig
	}
	all {
	    foreach signal $sigList {
		$self plot_sig $signal
	    }
	}
	one_and_list {
	    $self plot_sig $curSig
	    foreach signal $dispSigList {
		$self plot_sig $signal
	    }
	}
    }
    $masterCanvas lower ${self}Axis
    return
}

# 
myGraph method set_disp_type {signal disp_type} {
    instvar sigList
    
    foreach sig $sigList {
	if {![string compare $sig $signal]} {
	    dispPar${self}$signal set disp_type $disp_type
	    return $sig
	}
    }
    $self refresh_disp
    return 0
}

# 
myGraph method set_type {{disp_type ""}} {
    instvar curSig

    if {$disp_type != ""} {
	dispPar${self}$curSig set disp_type $disp_type
	$self refresh_disp
    }
    return $curSig
}

# 
myGraph method set_color {{disp_color ""}} {
    instvar curSig

    if {$disp_color != ""} {
	dispPar${self}$curSig setcolor $disp_color
	$self refresh_disp
    }
    return $curSig
}

# 
myGraph method set_width {w} {
    instvar width
    instvar grWidth
    instvar xGrPosInCv

    set width $w
    set grWidth  [expr $width-$xGrPos-10]
    $self init_disp
}

# 
myGraph method set_height {h} {
    instvar height
    instvar grHeight
    instvar yGrPos

    set height $h
    set grHeight  [expr $height-$yGrPos*2]
    $self refresh_disp
}

# 
myGraph method change_geometry {w h x_pos y_pos} {
    instvar xPos
    instvar yPos
    instvar width
    instvar height
    instvar grWidth
    instvar grHeight
    instvar xGrPos
    instvar yGrPos
    instvar xGrPosInCv
    instvar yGrPosInCv

    set width  $w
    set height $h
    set xPos $x_pos
    set yPos $y_pos

    set xGrPosInCv [expr $x_pos+$xGrPos]
    set yGrPosInCv [expr $y_pos+$yGrPos]
    set grWidth  [expr $width-$xGrPos-10]
    set grHeight [expr $height-$yGrPos*2]

    $self refresh_disp
}

myGraph method plot_sig {sig} {
    instvar masterCanvas
    instvar xPos
    instvar yPos
    instvar width
    instvar height
    instvar grWidth
    instvar grHeight
    instvar xGrPos
    instvar yGrPos
    instvar xGrPosInCv
    instvar yGrPosInCv
    instvar xMin
    instvar xMax
    instvar yMin
    instvar yMax
    instvar logMode

    #if {[info exists emu_graph($graph,image)]} {
	#$masterCanvas create image \
		#[emu_graph__x2masterCanvas $graph $emu_graph($graph,xmin)]  \
		#[emu_graph__y2masterCanvas $graph $emu_graph($graph,ymax)] \
		#-anchor nw -image $emu_graph($graph,image) \
		#-tags [list graph$graph image$graph]
    #}

    set tags [list ${self}${sig} ${self} ${self}Curves]
    
    # plot the points, first delete any old ones
    $masterCanvas delete -withtag ${self}${sig}

    set xArgs ""
    set yArgs ""
    switch $logMode {
	xlog {
	    set xArgs -xlog
	}
	ylog {
	    set yArgs -ylog
	}
	xylog {
	    set xArgs -xlog
	    set yArgs -ylog
	}
    }

    catch {unset coords}
    set coords [sig2coords $sig \
	    $xGrPosInCv $yGrPosInCv \
	    $grWidth $grHeight \
	    $xMin $xMax $yMin $yMax \
	    $xArgs $yArgs]

    set disp_type \
	    [dispPar${self}$sig set disp_type]
    set disp_color \
	    [dispPar${self}$sig set disp_color]

    if { ([string compare $disp_type line] == 0) && ([llength $coords] >= 4)} {
	## create the line as a single line item
	eval "$masterCanvas create line $coords -width 1 -fill $disp_color -tags {$tags}"
    }

    for {set i 0} {$i < [llength $coords]-1} {incr i 2} {
	set point [lrange $coords $i [expr $i+1]]
	if { [string compare $disp_type circle] == 0 } {
	    set ox [lindex $point 0]
	    set oy [lindex $point 1]
	    eval "$masterCanvas create arc \
		    [expr $ox-2] [expr $oy-2]\
		    [expr $ox+2] [expr $oy+2]\
		    -start 0 \
		    -extent 359 \
		    -style chord \
		    -outline $disp_color \
		    -width 1 \
		    -tags {$tags}"
	} elseif { [string compare $disp_type bigcircle] == 0 } {
	    set ox [lindex $point 0]
	    set oy [lindex $point 1]
	    eval "$masterCanvas create arc \
		    [expr $ox-2] [expr $oy-2]\
		    [expr $ox+2] [expr $oy+2]\
		    -start 0 \
		    -extent 359 \
		    -style chord \
		    -outline $disp_color \
		    -width 3 \
		    -tags {$tags}" 
	} elseif { [string compare $disp_type plus] == 0 } {
	    set ox [lindex $point 0]
	    set oy [lindex $point 1]
	    eval "$masterCanvas create line \
		    [expr $ox-2] [expr $oy]\
		    [expr $ox+2] [expr $oy]\
		    -fill $disp_color \
		    -width 1 \
		    -tags {$tags}"
	    eval "$masterCanvas create line \
		    [expr $ox] [expr $oy-2]\
		    [expr $ox] [expr $oy+2]\
		    -fill $disp_color \
		    -width 1 \
		    -tags {$tags}"
	} elseif { [string compare $disp_type cross] == 0 } {
	    set ox [lindex $point 0]
	    set oy [lindex $point 1]
	    eval "$masterCanvas create line \
		    [expr $ox-2] [expr $oy-2]\
		    [expr $ox+2] [expr $oy+2]\
		    -fill $disp_color \
		    -width 1 \
		    -tags {$tags}"
	    eval "$masterCanvas create line \
		    [expr $ox+2] [expr $oy-2]\
		    [expr $ox-2] [expr $oy+2]\
		    -fill $disp_color \
		    -width 1 \
		    -tags {$tags}"
	}
    }

    if {[sgettype $sig] == 3} {
	# FOUR_NR or CPLX type. 
	set coords [sig2coords $sig \
		$xGrPosInCv $yGrPosInCv \
		$grWidth $grHeight \
		$xMin $xMax $yMin $yMax \
		-imagpart \
		$xArgs $yArgs]

	set disp_type \
		[dispPar${self}$sig set disp_type]
	set disp_color \
		[dispPar${self}$sig set disp_color]

	if { ([string compare $disp_type line] == 0) && ([llength $coords] >= 4)} {
	    ## create the line as a single line item
	    eval "$masterCanvas create line $coords -width 1 -fill $disp_color -tags {$tags}"
	}

	for {set i 0} {$i < [llength $coords]-1} {incr i 2} {
	    set point [lrange $coords $i [expr $i+1]]
	    if { [string compare $disp_type circle] == 0 } {
		set ox [lindex $point 0]
		set oy [lindex $point 1]
		eval "$masterCanvas create arc \
			[expr $ox-2] [expr $oy-2]\
			[expr $ox+2] [expr $oy+2]\
			-start 0 \
			-extent 359 \
			-style chord \
			-outline $disp_color \
			-width 1 \
			-tags {$tags}"
	    } elseif { [string compare $disp_type plus] == 0 } {
		set ox [lindex $point 0]
		set oy [lindex $point 1]
		eval "$masterCanvas create line \
			[expr $ox-2] [expr $oy]\
			[expr $ox+2] [expr $oy]\
			-fill $disp_color \
			-width 1 \
			-tags {$tags}"
		eval "$masterCanvas create line \
			[expr $ox] [expr $oy-2]\
			[expr $ox] [expr $oy+2]\
			-fill $disp_color \
			-width 1 \
			-tags {$tags}"
	    } elseif { [string compare $disp_type cross] == 0 } {
		set ox [lindex $point 0]
		set oy [lindex $point 1]
		eval "$masterCanvas create line \
			[expr $ox-2] [expr $oy-2]\
			[expr $ox+2] [expr $oy+2]\
			-fill $disp_color \
			-width 1 \
			-tags {$tags}"
		eval "$masterCanvas create line \
			[expr $ox+2] [expr $oy-2]\
			[expr $ox-2] [expr $oy+2]\
			-fill $disp_color \
			-width 1 \
			-tags {$tags}"
	    }
	}
    }
}

#
myGraph method plot_sig2 {signal} {
    instvar masterCanvas
    instvar xPos
    instvar yPos
    instvar width
    instvar height
    instvar grWidth
    instvar grHeight
    instvar xGrPos
    instvar yGrPos
    instvar xGrPosInCv
    instvar yGrPosInCv
    instvar xMin
    instvar xMax
    instvar yMin
    instvar yMax
    
    set current_disp_type \
	    [dispPar${self}$signal set disp_type]
    set current_disp_color \
	    [dispPar${self}$signal set disp_color]
    sig2bmp $signal _bmp$bmp_num \
	    $grWidth $grHeight \
	    $xMin $xMax $yMin $yMax \
	    -type $current_disp_type
    $masterCanvas create bitmap \
	    $xGrPosInCv $yGrPosInCv \
	    -bitmap _bmp$bmp_num \
	    -anchor nw \
	    -foreground $current_disp_color \
	    -tag {${self}Curves ${self}}
    incr bmp_num
}

#
myGraph method get_sig_list {} {
    instvar sigList

    foreach sig $sigList {
	set color [dispPar${self}$sig set disp_color]
	set type  [dispPar${self}$sig set disp_type]
	set lst [lappend lst [list $sig $color $type]]
    }
    return $lst
}

#
myGraph method the_motion {i j} {
    instvar xGrPosInCv
    instvar yGrPosInCv
    instvar grWidth
    instvar grHeight

    if {$i < $xGrPosInCv} {
	return
    }
    if {$j < $yGrPosInCv} {
	return
    }
    if {$i > $xGrPosInCv+$grWidth} {
	return
    }
    if {$j > $yGrPosInCv+$grHeight} {
	return
    }
    return [list [$self i2x $i] [$self j2y $j] ]
}

#
myGraph method add_cur_sig_to_display_list {} {
    instvar curSig
    instvar dispSigList

    if {[lsearch $dispSigList $curSig] == -1} {
	lappend dispSigList $curSig
    }

    return
}


#
myGraph method remove_cur_sig_to_display_list {} {
    instvar curSig
    instvar dispSigList

    set index [lsearch $dispSigList $curSig]
    if {$index != -1} {
	set dispSigList [lreplace $dispSigList $curSig $index $index]
    }

    return
}

