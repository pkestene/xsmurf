#----------------------------------------------------------------------
# class PanedWindow
#----------------------------------------------------------------------

DOC "class PanedWindow (Tk)" {

 NAME
	PanedWindow
		- A geometry manager with movable sashes between
		  its managed windows.
 SYNOPSIS
	Creation:
		PanedWindow <window> <options>
		PanedWindow new <window> <options>

	Configuration:
		<window> manage <win> ?win? ?win?..
		<window> slaves
		<window> fractions
		<window> configure <options>

 DESCRIPTION
	Manage one or more windows with movable sashes between.
	Configuration can be altered dynamically.  The case with no
	windows begin managed is handled gracefully as well.

 METHODS
	manage win ?win? ?win?..
		Arrange for the specified windows to be managed by the
		PanedWindow object.  The windows are managed in the specified
		order from top to bottom, or left to right (for horizontal
		layout).  Repeatedly using this method will unmanage all
		currently managed windows, before managing the specified ones.
		Specifying the null list, {}, will unmanage all windows.

	slaves	Returns a list of all currently managed windows.

	fractions
		Returns a list of fractions reflecting the current placement
		of the sashes.  This list can then be used with the
		-fraction option to re-set a specific layout.

	resize
		Returns a list of resize behaviour specifiers.  This list
		can be used with the -resize option to re-set a specific
		resize behaviour.

	configure <option>
		Configure the PanedWindow object.  See OPTIONS below
		for possible alternatives.

 OPTIONS
	-manage <list>
		Specifies a list of slaves to manage.  Equivalent to using
		the `manage' method, but can be specified at object creation-
		time.
	-orientation [x|y]
		For windows stacked on top of each other specify "y", for
		horizontal layout specify "x".
	-width	Initial width of the paned window.
	-height Initial height of the paned window.
	-min	Minimum size in pixels to which a pane can shrink
		(default 50)
	-fraction <list>
		List of initial fractions each pane is to occupy.
		The list should be cumulative (each new position is
		counted from the top/left side).  Example: ".3 .7" would
		give three panes equal space.  If less panes exist than
		specified fractions, the last pane gets the space from the
		"missing" windows.
	-resize <list>
		See NOTE below!!    
		List of resize behaviours for the panes.  The list should
		consist of the words "static" and "dynamic" (may be
		abbreviated).  "static" means that the pane will only change
		size when the user moves a sash. "dynamic" means the pane will
		also adjust its size when the main window changes size.
		Normally at least one of the panes should be dynamic, to allow
		resizing of the main window to take effect.
		Dynamic panes share the space left over by the static panes.
		This space is shared in proportion to the fraction
		specifications for the dynamic panes.
		Initially all panes are dynamic.
	-handleSide [begin|end]
		Specifies on which side of the sash the handle should be
		placed.  "begin" means top or left, "end" means bottom
		or right.  Default is "end"
	-handleOffs <pixels>
		Specify how many pixels from the near edge the sash handle
		should be placed.  Measured from the handle's centre.

 NOTE:
	This is an alpha release of a PanedWindow class which incorporates
	the notion of static and dynamic panes.  Static panes are only
	resized by the action of dragging a sash.  Dynamic panes are
	also resized by changes in their available space.

	This version leaves some things to be desired, especially when
	the available space for the paned window gets very small.

	Contributions and improvements are welcome, as are bug reports.
	I will take all feedback into consideration for the non alpha
	implementation.

 EXAMPLE

	# Create a PanedWindow object:

	PanedWindow .pw -width 300 -height 300 -min .15 \
		-manage {.foo .bar .baz}

	# Add window .new_one first:

	eval .pw manage .new_one [.priv.pw slaves]

	# Save current sash settings for later

	set save [.pw fractions]

	# Restore sash settings

	.pw configure -fraction $save

 ACKNOWLEDGEMENTS

     James Noble
	1993 - unknown
     Jay Schmidgall
	1994 - base logic posted to comp.lang.tcl
     Mark L. Ulferts <mulferts@spd.dsccc.com>
	1994 - added additional features and made it a class.
     Joe Hidebrand <hildjj@fuentez.com>
	07/25/94 - posted first multipane version to comp.lang.tcl
	07/28/94 - added support for vertical panes

 AUTHOR

     Patrik Floding <patrik@dynas.se>
	23/10/95 - Adapted from itcl to obTcl.  Changes to allow handling
	of non-child windows.  Moving towards a general geometry
	manager approach.  Changes to handle reconfiguration (managing
	new windows, etc).
}

class PanedWindow
PanedWindow inherit Widget

PanedWindow option {-orientation} y verify {
	switch -exact -- $orientation {
	y {}
	x {}
	default {error "orientation must be 'x' or 'y'"}
	}
}

PanedWindow option {-width} 100 configure {
       	$self.priv.pw config -width $width
}

PanedWindow option {-height} 100 configure {
       	$self.priv.pw config -height $height
}

PanedWindow option {-min} 50 verify {
      	if {[expr $min < 1 || $min > 10000]} {
        	error "min size must be between 1 and 10000"
      	}
}

PanedWindow option {-fraction} {} verify {
	instvar frac number
	if { [info exists number] && $number > 0 } {
		$self setNumber $number
	}
} configure {
	instvar frac number
	if { [info exists number] && $number > 0 } {
		$self setNumber $number
	}
      	$self replace
}

PanedWindow option {-resize} {} verify {
	instvar type
	set n 1
	foreach i $resize {
		switch -glob -- $i {
		s*	{ set type($n) static }
		d*	{ set type($n) dynamic }
		default { error "option -resize takes\
			list of \"static\" and \"dynamic\" specifiers" }
		}
		incr n
	}
} configure {
	instvar type
	set n 1
	foreach i $resize {
		switch -glob -- $i {
		s*	{ set type($n) static }
		d*	{ set type($n) dynamic }
		default { error "option -resize takes\
			list of \"static\" and \"dynamic\" specifiers" }
		}
		incr n
	}
}

PanedWindow option {-handleOffs} {-25} verify {
	instvar handlePos hOffs
	
	if { $handleOffs >= 0 } {
		set hOffs $handleOffs
	}
} configure {
	instvar handlePos
	$self configure -handleSide $handlePos
}

PanedWindow option {-handleSide} {end} verify {
	instvar handlePos hOffs handleOffs

      	if { ![string compare "begin" $handleSide] || $handleSide == 0 } {
	 	set handlePos 0
		set handleOffs $hOffs
      	} elseif { ![string compare "end" $handleSide] || $handleSide == 1 } {
	 	set handlePos 1
		set handleOffs -$hOffs
      	} else {
         	error "handleSide must be begin or end"
      	}
} configure {
	$self replace
}

PanedWindow option {-manage} {} init {
	eval $self manage $manage
} configure {
	eval $self manage $manage
}

PanedWindow method setNumber num {
	instvar frac type fraction number

	set number $num
	for {set i 1} {$i <= $number} {incr i} {
		if ![info exists type($i)] {
			set type($i) dynamic
		}
	}

	if {![info exists fraction] || [llength $fraction] < $number-1} {
		set frac(0) 0
         	set part [expr 1.0 / $number.0]
         	for {set i 1} {$i <= $number} {incr i} {
            		set frac($i) [expr $i * $part]
         	}
		while {[info exists frac($i)] == 1} {
			unset frac($i)
			catch {unset type($i)}
			incr i
		}
      	} else {
		$self setFracVector $fraction
	}
}

PanedWindow method setDefaults {} {
	instvar min orientation handlePos hOffs handleOffs frac

	set frac(0) 0
	set handlePos 1
	set hOffs 20
}


# ------------------------------------------------------------------
#  METHOD:  destroy
# ------------------------------------------------------------------
PanedWindow method destroy args {
	next $args
}

# ------------------------------------------------------------------
#  METHOD:  init - Called upon object creation.  Creates main window.
# ------------------------------------------------------------------
PanedWindow method init args {
	instvar handlePos number orientation inInit pendingManage \
		height width

	$self setDefaults

	set inInit 1

	eval {$self conf_verify} $args
	next -width $width -height $height

	lower $self
	frame $self.priv -borderwidth 0
      	frame $self.priv.pw
	bind $self.priv.pw <Configure> "$self reconf %w %h"

	unset inInit
	if [info exists pendingManage] {
		eval $self manage $pendingManage
	}

	eval {$self conf_init} $args
}

PanedWindow method init_unknown args {
	eval $self-cmd configure $args
}

PanedWindow method bindHandles { w n dir } {
    bind $w <Button-1> "$self start-grip %$dir $n"
    bind $w <B1-Motion> "$self handle-grip %$dir $n"
    bind $w <B1-ButtonRelease-1> "$self end-grip %$dir $n"
}

# ------------------------------------------------------------------
#  METHOD:  makePanes - internal method
# ------------------------------------------------------------------
PanedWindow method makePanes no {
	instvar number orientation handlePos hOffs handleOffs frac \
		fraction firstTime

	catch {eval destroy [info commands $self.priv.pane*]}
	catch {eval destroy [info commands $self.priv.sep*]}
	catch {eval destroy [info commands $self.priv.handle*]}
	catch {eval destroy [info commands $self.priv.*marg]}

	set firstTime 1
      	#
      	# Make the windows
      	#

      	for {set i 0} {$i < $no} {incr i} {

	    frame $self.priv.pane$i -borderwidth 0

            if {$i != 0} {

	    	frame $self.priv.pane$i.tmarg -height 5 -width 5 \
			 -cursor crosshair
		if [string compare "y" $orientation] {
		    	pack $self.priv.pane$i.tmarg -side left -fill y
		} else {
		    	pack $self.priv.pane$i.tmarg -side top -fill x
		}

		#
		# Make the separator
		#
		frame $self.priv.sep$i -height 2 -width 2 -borderwidth 1 \
        		-relief sunken -cursor crosshair

            	#
            	# Make the sash button
            	#
            	frame $self.priv.handle$i -width 10 -height 10 -borderwidth 2 \
                	-relief raised -cursor crosshair

            	if [string compare "y" $orientation] {
			$self bindHandles $self.priv.handle$i $i x
			$self bindHandles $self.priv.sep$i $i x
			$self bindHandles $self.priv.pane$i.tmarg $i x
            	} else {
               		$self bindHandles $self.priv.handle$i $i y
			$self bindHandles $self.priv.sep$i $i y
			$self bindHandles $self.priv.pane$i.tmarg $i y
	    	}

	    }
	    if { $i < $no - 1 } {
		    set n [expr $i + 1]
		    frame $self.priv.pane$i.bmarg -height 5 -width 5 \
			-cursor crosshair
		    if [string compare "y" $orientation] {
			    pack $self.priv.pane$i.bmarg -side right -fill y
			    $self bindHandles $self.priv.pane$i.bmarg $n x
		    } else {
			    pack $self.priv.pane$i.bmarg -side bottom -fill x
			    $self bindHandles $self.priv.pane$i.bmarg $n y
		    }
	    }
      	}

      	pack $self.priv.pw -fill both -expand yes -anchor w
      	pack $self.priv -fill both -expand yes -anchor w
}


# ------------------------------------------------------------------
#  METHOD slaves - Returns list of currently managed windows
# ------------------------------------------------------------------
PanedWindow method slaves {} {
	instvar slaves
	return $slaves
}

# ------------------------------------------------------------------
#  METHOD manage - Arranges for a list of windows to be managed
#  		   by PanedWindow.
# ------------------------------------------------------------------
PanedWindow method manage args {
	instvar slaves frac fraction number inInit pendingManage

	set slaves {}
	if [info exists inInit] {
		set pendingManage $args
		return
	}
	$self setNumber [llength $args]
	$self makePanes $number
	set j 0
	foreach i $args {
		if ![string compare "" $i] { continue }
		if [catch {pack $i -fill both -expand yes\
				-in $self.priv.pane$j} msg] {
			puts stderr "$msg"
		} else {
			lappend slaves $i
			raise $i $self.priv.pane$j
		}
		incr j
	}
}

# ------------------------------------------------------------------
#  METHOD setFracVector - fills in the frac-array from the fraction var.
# ------------------------------------------------------------------
PanedWindow method setFracVector fraction {
	instvar frac type number

      	set i 0
      	set frac(0) 0

      	foreach f $fraction {
		if { $i >= $number - 1 } {
			break
		}
        	incr i
         	set frac($i) $f
	 	if {$frac($i) <= $frac([expr $i-1])} {
	    		error "fractions must be in ascending order"
	 	}
      	}
      	incr i
      	set frac($i) 1
      	incr i
	while {[info exists frac($i)]} {
		unset frac($i)
		catch {unset type($i)}
		incr i
	}
}

# ------------------------------------------------------------------
#  METHOD calc-fraction - determines the fraction for the sash
# ------------------------------------------------------------------
PanedWindow method calc-fraction {where num} {
	instvar orientation frac min drag_start frac_start

      	if [string compare "y" $orientation] {
		set size [winfo width $self]
		set frac($num) \
	     		[expr (($where.0 - $drag_start.0) / $size) + $frac_start]
      	} else {
		set size [winfo height $self]
	 	set frac($num) \
	     		[expr (($where.0 - $drag_start.0) / $size) + $frac_start]
      	}

	set minfrac [expr double($min)/double($size)]
      	if {$frac($num) < [expr $frac([expr $num - 1]) + $minfrac]} {
         	set frac($num) [expr $frac([expr $num - 1]) + $minfrac]
      	}
      	if {$frac($num) > [expr $frac([expr $num + 1]) - $minfrac]} {
         	set frac($num) [expr $frac([expr $num + 1]) - $minfrac]
      	}
}

# ------------------------------------------------------------------
#  METHOD start-grip - Starts the sash drag and drop operation
# ------------------------------------------------------------------
PanedWindow method start-grip {where num} {
	instvar drag_start drag_win frac_start frac orientation

	set w [winfo toplevel $self]
	if [string compare "." $w] {
		set drag_win $w._priv_handle
	} else {
		set drag_win ._priv_handle
	}

	catch {destroy $drag_win}
	frame $drag_win -relief flat -bg grey40 -borderwidth 0 -width 1 -height 1

	if [string compare "y" $orientation] {
		place $drag_win -in $self.priv.pw -rely 0 -relheight 1 \
			-relx $frac($num) -anchor n
	} else {
		place $drag_win -in $self.priv.pw -relx 0 -relwidth 1 \
			-rely $frac($num) -anchor w
	}
      	set drag_start $where
	set frac_start $frac($num)
}

# ------------------------------------------------------------------
#  METHOD end-grip - Ends the sash drag and drop operation
# ------------------------------------------------------------------
PanedWindow method end-grip {where num} {
	instvar drag_win

	$self calc-fraction $where $num
	destroy $drag_win
      	$self replace
}

# ------------------------------------------------------------------
#  METHOD handle-grip - Motion action for sash
# ------------------------------------------------------------------
PanedWindow method handle-grip {where num} {
	instvar frac orientation handlePos drag_win

      	$self calc-fraction $where $num

      	if [string compare "y" $orientation] {
	 	place $drag_win -in $self.priv.pw -rely 0 -relheight 1 \
	     		-relx $frac($num) -anchor n
      	} else {
	 	place $drag_win -in $self.priv.pw -relx 0 -relwidth 1 \
	     		-rely $frac($num) -anchor w
      	}
}

# ------------------------------------------------------------------
#  METHOD reconf - Calculate new fractions after a window resize.
#                  Take into account "static" panes (panes that only
#		   change size when the sash is dragged).
# ------------------------------------------------------------------
PanedWindow method reconf { w h } {
	instvar orientation frac type number firstTime

	if {$firstTime} {
		set firstTime 0
		$self replace
		return
	}

	if [string compare "y" $orientation] {
		set size $w
		set dim width
	} else {
		set size $h
		set dim height
	}

	# fr() are the new non-cumulative fractions.
	#
	set total 0.0
	set totStatic 0.0
	set totDynamic 0.0
	set dynMin 0.0
    	for {set i 1} {$i <= $number} {incr i} {
		if [string compare "dynamic" $type($i)] {
			set fr($i) [expr $frac($i) - $total]
		} else {
			set newFrac [expr $frac($i) - $total]
			if { $newFrac <= 0.1 } {
				set newFrac 0.1
				set dynMin [expr $dynMin + 0.1]
			}
			set fr($i) $newFrac
			set totDynamic [expr $totDynamic + $newFrac]
		}
		set total [expr $total + $fr($i)]
	}

	set totStatic 0.0
    	for {set i 1} {$i <= $number} {incr i} {
		if ![string compare "static" $type($i)] {
			set oldSize [winfo $dim $self.priv.pane[expr $i-1]]
			set newFrac [expr double($oldSize)/double($size)]
			set fr($i) $newFrac
			set totStatic [expr $totStatic + $newFrac]
		}
	}
	if { ( $totStatic + $dynMin ) > 1.0 } {
		#puts stderr "Oops: reconf: unable to fit panes!! totStatic=$totStatic"
		set adjust 1.0
	} else {
		set adjust [expr (1.0 - $totStatic) / $totDynamic]
	}
	
	set acc 0.0
    	for {set i 1} {$i <= $number} {incr i} {
		if ![string compare "dynamic" $type($i)] {
			set fr($i) [expr $fr($i) * $adjust]
		}
		set acc [expr $acc + $fr($i)]
		set frac($i) $acc
	}

	$self replace
}

# ------------------------------------------------------------------
#  METHOD replace - Resets the panes of the window following
#                   movement of the sash or reconfiguration
# ------------------------------------------------------------------
PanedWindow method replace {} {
	instvar orientation frac handlePos handleOffs number
 
	if ![winfo exists $self.priv.pane0] { return }

	if [string compare "y" $orientation] {
		set totSize [winfo width $self.priv.pw]

		place forget $self.priv.pane0
	    	place $self.priv.pane0 -in $self.priv.pw -y 0 -relx 0 \
				-relheight 1 -width [expr $frac(1)*$totSize]

	    	for {set i 1} {$i < $number} {incr i} {
	       		place $self.priv.sep$i -in $self.priv.pw -rely 0 -relheight 1 \
		   		-relx $frac($i) -anchor n
	       		place $self.priv.handle$i -in $self.priv.pw \
				-rely $handlePos -y $handleOffs \
		   		-relx $frac($i) \
		   		-anchor center
	       		place $self.priv.pane$i -in $self.priv.pw -y 0 -relx $frac($i) \
		   		-relheight 1 \
		   		-width \
					[expr ($frac([expr $i + 1])-$frac($i))*$totSize]
	    	}
	} else {
		set totSize [winfo height $self.priv.pw]

		place forget $self.priv.pane0
	    	place $self.priv.pane0 -in $self.priv.pw -x 0 -rely 0 \
				-relwidth 1 -height [expr $frac(1)*$totSize]

	    	for {set i 1} {$i < $number} {incr i} {
	       		place $self.priv.sep$i -in $self.priv.pw -relx 0 -relwidth 1 \
		   		-rely $frac($i) -anchor w
	       		place $self.priv.handle$i -in $self.priv.pw \
				-relx $handlePos -x $handleOffs \
		   		-rely $frac($i) \
		   		-anchor center
			place forget $self.priv.pane$i
	       		place $self.priv.pane$i -in $self.priv.pw -x 0 -rely $frac($i) \
		   		-relwidth 1 \
		   		-height \
					[expr ($frac([expr $i + 1])-$frac($i))*$totSize]
	    	}
	}
}

# ------------------------------------------------------------------
#  METHOD fractions - Return the current list of fractions
# ------------------------------------------------------------------
PanedWindow method fractions {} {
	instvar number frac

      	set fracs ""
      	for {set i 1} {$i < $number} {incr i} {
         	lappend fracs $frac($i)
      	}
      	return $fracs
}

# ------------------------------------------------------------------
#  METHOD resize - Return the current list of resize behaviours
# ------------------------------------------------------------------
PanedWindow method resize {} {
	instvar number type

      	set res ""
      	for {set i 1} {$i <= $number} {incr i} {
         	lappend res $type($i)
      	}
      	return $res
}
