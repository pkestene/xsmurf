# myEiWin.tcl --
#
#       This file implements the Tcl code for the creation of a graph that will
#       appear in a given canvas. A graph is an entity that displays several
#       signals with the same axes.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: myEiWin.tcl,v 1.4 1999/01/08 15:44:19 decoster Exp $
#


#
class myEiWin

myEiWin method init {canvas w h x_pos y_pos ei_hoffset ei_voffset the_zoom sig_lst} {
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
    # Max size of graph.
    instvar maxGrWidth
    instvar maxGrHeight
    # Real coordinates of the graph (corresponding to the signals coordinates).
    instvar xMin
    instvar xMax
    instvar yMin
    instvar yMax
    # List of signal names.
    instvar sigList
    # Selected (or current) signal and his index.
    instvar curSig
    instvar curSigIndex
    # Definition of the graph label.
    instvar labelText
    instvar labelColor
    instvar label
    instvar labelFont
    # define the kind of display
    instvar dispMode

    instvar zoom

    instvar eihoffset
    instvar eivoffset

    instvar the_draw_args

    instvar settingBox

    instvar refreshFlag

    next

    # Get args.
    set masterCanvas $canvas
    set width  $w
    set height $h
    set sigList $sig_lst
    set curSigIndex 0
    set xPos $x_pos
    set yPos $y_pos
    set zoom $the_zoom
    set eihoffset $ei_hoffset
    set eivoffset $ei_voffset
    set the_draw_args {-color white}

    set settingBox 0

    # Instvar initialisation.
    set labelText ""
    set label [list curSigLabel]
    set labelFont my_font
    #set label [list {black "sig list :"} allSigLabel]
    set dispMode one
    set curSig [lindex $sigList 0]

    set xGrPos 70
    set yGrPos 30
    set xGrPosInCv [expr $x_pos+$xGrPos]
    set yGrPosInCv [expr $y_pos+$yGrPos]
    set grWidth  [expr $width-$xGrPos-10]
    set grHeight [expr $height-$yGrPos*2]
    set maxGrWidth  $grWidth
    set maxGrHeight $grHeight

    set refreshFlag yes

    $self init_disp
}

#
myEiWin method destroy {args} {
    instvar masterCanvas
    instvar sigList

    next
}

#
myEiWin method disp_axes {} {
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
    set majorTicksLength -10
    set minorTicksLength -5
    set ticksLabelFont my_font
    set textOffset 14

    set yMin_c [expr $height+$yref+2]
    set yMax_c [expr $yref-2]
    set xMin_c [expr $xref-2]
    set xMax_c [expr $width+$xref+2]

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
	set y [expr int((-$yMin+$t)*$yfactor)+$yref]
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
	    set y [expr int((-$yMin+$t)*$yfactor)+$yref]
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
myEiWin method x2i {x} {
    instvar grWidth
    instvar xMin xMax
    instvar xGrPosInCv

    return [expr int(($x-$xMin)*$grWidth/($xMax-$xMin))+$xGrPosInCv]
}

# Convert a real ordinate into an index on the drawable zone.
myEiWin method y2j {y} {
    instvar grHeight
    instvar yMin yMax
    instvar yGrPosInCv

    return [expr int(($y-$yMin)*$grHeight/($yMax-$yMin))+$yGrPosInCv]
}

# Convert an index on the drawable zone into a real absciss.
myEiWin method i2x {i} {
    instvar grWidth
    instvar xMin xMax
    instvar xGrPosInCv

    return [expr $xMin+($i-$xGrPosInCv)*($xMax-$xMin)/(1.0*$grWidth)]
}

# Convert an index on the drawable zone into a real ordinate.
myEiWin method j2y {j} {
    instvar grHeight
    instvar yMin yMax
    instvar yGrPosInCv

    return [expr $yMin+($j-$yGrPosInCv)*($yMax-$yMin)/(1.0*$grHeight)]
}

#
myEiWin method convert_coordinates {x y} {
    instvar grWidth grHeight
    instvar xMin xMax yMin yMax

    set new_x [$self i2x $x]
    set new_y [$self j2y $y]
    return [list $new_x $new_y]
}

#
myEiWin method getI {x} {
    if {[regexp {.*s$} $x] == 1} {
	set newX [string range $x 0 [expr [string length $x]-2]]
	set i [$self x2i $newX]
    } else {
	set i $x
    }
    return $i
}

#
myEiWin method getJ {y} {
    if {[regexp {.*s$} $y] == 1} {
	set newY [string range $y 0 [expr [string length $y]-2]]
	set j [$self y2j $newY]
    } else {
	set j $y
    }
    return $j
}

#
myEiWin method line {x1 y1 x2 y2 {color black}} {
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
myEiWin method add {sig {index end}} {
    instvar sigList
    instvar curSig
    instvar curSigIndex

    set is_in [lsearch -exact $sigList $sig]
    if {$is_in == -1} {
	set sigList [linsert $sigList $index $sig]
    }
    $self init_disp

    return $sig
}

# Forward the selected signal in the list.
myEiWin method + {{val 1}} {
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
myEiWin method - {{val 1}} {
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
myEiWin method next_signal args {
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
myEiWin method prev_signal args {
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
myEiWin method current {{sig ""}} {
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


# Change the value of dispMode.
myEiWin method set_disp_mode {mode} {
    instvar dispMode

    set dispMode $mode
    $self refresh_disp
    return $dispMode
}

# Init the real coordinates of the graph box.
myEiWin method init_values {} {
    instvar curSig
    instvar xMin xMax yMin yMax
    instvar dispMode
    instvar sigList

    instvar eihoffset
    instvar eivoffset

    instvar grWidth
    instvar grHeight

    instvar zoom

    set xMin $eihoffset
    set yMin $eivoffset
    set xMax [expr { $eihoffset + $grWidth/$zoom }]
    set yMax [expr { $eivoffset + $grHeight/$zoom }]

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

# Initialize the box coordinates and refresh graph display.
myEiWin method init_disp {} {
    $self init_values
    $self refresh_viewer
}

# Refresh graph display.
myEiWin method refresh_disp {} {
    $self refresh_viewer
}

# This method displays the label of the graph acording to the pattern defined by
# <label> instvar. The label consists of a text line which width doesn't exceed
# the size of the graph (i.e. <grWidth> instvar).
#
myEiWin method disp_label {} {
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
    set y [expr $yGrPosInCv-20]

    # set the item list depending on the values of <label> elements.
    set itemList {}
    foreach item $label {
	switch $item {
	    curSigLabel {
		# item(s) defined by the current signal.
		lappend itemList  $itemList [list black $curSig]
	    }
	    allSigLabel {
		# list of items defined by each signals from <sigList>.
		foreach sig $sigList {
		    lappend itemList [list black $sig]
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
		    -font $labelFont \
		    -anchor w \
		    -tags $tagsList
	    # Stop the items display
	    break
	}
	incr x [expr [lindex $boxValues 2]-[lindex $boxValues 0]+2]
    }
    return
}

#
myEiWin method disp_label2 {} {
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
	set lbl_color black
    } else {
	set lbl_text $labelText
	set lbl_color $labelColor
    }
    $masterCanvas delete -withtag ${self}Label
    eval "$masterCanvas create text \
	    [expr $xGrPosInCv+$grWidth/2] [expr $yGrPosInCv-10] \
	    -text $lbl_text \
	    -fill $lbl_color \
	    -tags {${self}Label $self}"
    return
}

myEiWin method set_label args {
    instvar label

    set label $args
    $self disp_label

    return
}
# 
myEiWin method set_label2 {{str ""} {color black}} {
    instvar labelText
    instvar labelColor

    set labelText $str
    set labelColor $color
    $self disp_label

    return
}

# 
myEiWin method refresh_viewer {} {
    instvar masterCanvas
    instvar xMin xMax yMin yMax
    instvar grWidth grHeight
    instvar curSig
    instvar dispMode
    instvar sigList
    instvar xGrPosInCv yGrPosInCv

    instvar refreshFlag

    $self disp_label

    if {$refreshFlag == "needed"} {
	return
    }
    if {$refreshFlag == "no"} {
	set refreshFlag needed
	return
    }

    global bmp_num

    $masterCanvas delete -withtag ${self}Curves
    $self disp_axes
    switch $dispMode {
	one {
	    $self plot_sig $curSig
	}
	all {
	    foreach signal $sigList {
		$self plot_sig $signal
	    }
	}
    }
    $masterCanvas lower ${self}Axis
    return
}

# 
myEiWin method set_width {w} {
    instvar width
    instvar grWidth
    instvar xGrPosInCv

    set width $w
    set grWidth  [expr $width-$xGrPos-10]
    $self init_disp
}

# 
myEiWin method set_height {h} {
    instvar height
    instvar grHeight
    instvar yGrPos

    set height $h
    set grHeight  [expr $height-$yGrPos*2]
    $self refresh_disp
}

# 
myEiWin method change_geometry {w h x_pos y_pos} {
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

myEiWin method plot_sig {sig} {
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

    instvar zoom
    instvar eihoffset
    instvar eivoffset

    instvar the_draw_args

    set tags [list ${self}${sig} ${self} ${self}Curves]
    
    # plot the points, first delete any old ones
    $masterCanvas delete -withtag ${self}${sig}

    set xArgs ""
    set yArgs ""

    $masterCanvas create rectangle \
	    $xGrPosInCv $yGrPosInCv \
	    [expr {$xGrPosInCv + $grWidth}] [expr { $yGrPosInCv + $grHeight }] \
	    -fill black \
	    -tags $tags

    eval [concat [list eidrawcv $sig $masterCanvas \
	    $grWidth $grHeight \
	    $xGrPosInCv $yGrPosInCv \
	    $eihoffset $eivoffset \
	    $zoom \
	    -tags $tags] $the_draw_args]
}

#
myEiWin method get_sig_list {} {
    instvar sigList

    foreach sig $sigList {
	set lst [lappend lst $sig]
    }
    return $lst
}

#
myEiWin method linear_coding {} {
    instvar the_draw_args

    set the_draw_args -grey
    $self refresh_viewer
}

#
myEiWin method binary_coding {} {
    instvar the_draw_args

    set the_draw_args {-color white}
    $self refresh_viewer
}

#
myEiWin method start_box {x y} {
    instvar masterCanvas
    instvar settingBox
    instvar boxFirstX
    instvar boxFirstY

    set boxFirstX $x
    set boxFirstY $y

    $masterCanvas create rectangle $x $y $x $y -outline red -tags arrrrg
    set settingBox 1
}

#
myEiWin method set_box {x y} {
    instvar masterCanvas
    instvar settingBox
    instvar boxLastX
    instvar boxLastY
    instvar boxFirstX
    instvar boxFirstY
    instvar eihoffset
    instvar eivoffset
    instvar grWidth
    instvar grHeight
    instvar zoom
    instvar xGrPosInCv
    instvar yGrPosInCv

    if {$settingBox == 0} {
	set boxFirstX $x
	set boxFirstY $y

	$masterCanvas create rectangle $x $y $x $y -outline red -tags arrrrg

	set settingBox 1
    } else {
	set boxLastX $x
	set boxLastY $y
	$masterCanvas delete arrrrg
	set settingBox 0

	set eihoffset [expr { int([$self i2x $boxFirstX]) }]
	set eivoffset [expr { int([$self j2y $boxFirstY]) }]
	#set zoom [expr { $grWidth/($boxLastX - $boxFirstX) }]
	set grWidth  [expr { $boxLastX - $boxFirstX }]
	set grHeight [expr { $boxLastY - $boxFirstY }]


	#puts "$zeihoffset $zeivoffset $zgrHeight $zgrWidth"
	$self init_disp
    }
}

#
myEiWin method end_box {x y} {
    instvar masterCanvas
    instvar settingBox
    instvar boxLastX
    instvar boxLastY
    instvar boxFirstX
    instvar boxFirstY
    instvar eihoffset
    instvar eivoffset
    instvar grWidth
    instvar grHeight
    instvar zoom
    instvar xGrPosInCv
    instvar yGrPosInCv

    set boxLastX $x
    set boxLastY $y
    $masterCanvas delete arrrrg
    set settingBox 0

    set eihoffset [expr { int([$self i2x $boxFirstX]) }]
    set eivoffset [expr { int([$self j2y $boxFirstY]) }]
    #set zoom [expr { $grWidth/($boxLastX - $boxFirstX) }]
    set grWidth  [expr { $boxLastX - $boxFirstX }]
    set grHeight [expr { $boxLastY - $boxFirstY }]


    #puts "$zeihoffset $zeivoffset $zgrHeight $zgrWidth"
    $self init_disp
}

#
myEiWin method motion_box {x y} {
    instvar masterCanvas
    instvar settingBox
    instvar boxFirstX
    instvar boxFirstY
    instvar xGrPosInCv
    instvar yGrPosInCv
    instvar grWidth
    instvar grHeight

    set xx $x
    set yy $y

    if {$settingBox == 1} {
	if {$x < $xGrPosInCv} {
	    set xx $xGrPosInCv
	}
	if {$y < $yGrPosInCv} {
	    set yy $yGrPosInCv
	}
	if {$x > $xGrPosInCv+$grWidth} {
	    set xx [expr { $xGrPosInCv+$grWidth }]
	}
	if {$y > $yGrPosInCv+$grHeight} {
	    set yy [expr { $yGrPosInCv+$grHeight }]
	}
	$masterCanvas delete arrrrg
	$masterCanvas create rectangle $boxFirstX $boxFirstY $xx $yy -outline red -tags arrrrg
    }
}

#
myEiWin method mult_zoom {val} {
    instvar zoom
    instvar grWidth
    instvar grHeight
    instvar maxGrWidth
    instvar maxGrHeight

    set new_zoom [expr { $zoom * $val }]
    set new_grWidth [expr { int($grWidth * $val) }]
    set new_grHeight [expr { int($grHeight * $val) }]
    if { $new_grWidth > $maxGrWidth || $new_grHeight > $maxGrHeight } {
	return
    } else {
	set zoom $new_zoom
	set grWidth $new_grWidth
	set grHeight $new_grHeight
    }
    $self init_disp
}

#
myEiWin method reinit {} {
    instvar curSig
    instvar eivoffset
    instvar eihoffset
    instvar width
    instvar height
    instvar grWidth
    instvar grHeight
    instvar xGrPos
    instvar yGrPos

    instvar zoom

    lassign {gah width height} [einfo $curSig]
    set eihoffset 0
    set eivoffset 0

    if {$width >= 400} {
	set zoom [expr { 400.0/$width }]
    } else {
	set zoom 1
    }
    set width [expr { int($width * $zoom) }]
    set height [expr { int($height * $zoom) }]

    incr width 80
    incr height 60

    set grWidth  [expr $width-$xGrPos-10]
    set grHeight [expr $height-$yGrPos*2]

    $self init_disp
}

#
myEiWin method the_motion {i j} {
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
    return [list [expr { int([$self i2x $i]) }] [expr { int([$self j2y $j]) }]]
}

#
myEiWin method cancel {} {
    instvar masterCanvas
    instvar settingBox

    # Cancel box delemiting.
    $masterCanvas delete arrrrg
    set settingBox 0
}

#
myEiWin method disableRefresh {} {
    instvar refreshFlag

    set refreshFlag no
}

#
myEiWin method enableRefresh {} {
    instvar refreshFlag

    if {$refreshFlag == "needed"} {
	set refreshFlag yes
	$self refresh_viewer
    }
    set refreshFlag yes
}

