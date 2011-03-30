# signal_viewer.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: signal_viewer.tcl,v 1.8 1998/07/31 17:12:23 decoster Exp $
#

###
#
# Definition of a class to store all the display parameters for
# a given signal.
#
class disp_par2
disp_par2 inherit Base

disp_par2 method init args {
    instvar signal
    instvar disp_type
    instvar disp_color

    next

    set signal [lindex $args 0]
    set disp_type [lindex $args 1]
    set disp_color [lindex $args 2]
}

###
#
# Definition of a a new widget for signals.
#
class signal_viewer
#signal_viewer inherit SmurfViewer
signal_viewer inherit Widget

signal_viewer method init args {
    instvar display_widget_packed
    instvar viewer_xsize viewer_ysize
    instvar current_signal
    instvar current_signal_index
    instvar signals_dx
    instvar signals_x0
    instvar signals_size
    instvar x_min x_max y_min y_max
    instvar signals_list
    instvar display_mode
    instvar default_disp_type
    instvar disp_width
    instvar disp_height

    next

    set default_disp_type line

    set current_signal [lindex $args 0]
    set current_signal_index 0
    set signals_list $args
    set display_mode all
    if {[sgettype $current_signal] != 0} {
	set signals_dx [sgetdx $current_signal]
	set signals_x0 [sgetx0 $current_signal]
    } else {
	set signals_dx 1
	set signals_x0 0
    }
    set signals_size [ssize $current_signal]
    foreach signal $signals_list {
	disp_par2 new disp_par2_${self}_$signal \
		$signal $default_disp_type white
    }

    set display_widget_packed false
    set disp_width 0
    set disp_height 0

    frame $self.display -relief raised -bd 1 -height 1c

    viewer $self.viewer
    pack   $self.viewer -padx 1m -pady 1m -side top -fill both

    set string ""
    foreach signal $signals_list {
	if {[string compare $signal $current_signal]} {
	    set string "$string\n$signal"
	}
    }
    label $self.display.names -text $string -justify left -anchor w
    pack  $self.display.names  -side left -padx 1m -pady 1m

    label $self.display.fit -fg blue
    pack  $self.display.fit -side left -padx 1m -pady 1m

    label $self.display.cmd_res -wraplength 200 -justify left -anchor sw
    pack  $self.display.cmd_res -side left -padx 1m -pady 1m

    #$self pack_display_widget

    set viewer_xsize 400
    set viewer_ysize 300

    focus $self

    $self init_disp
    $self.viewer content $self.viewer.content

    bind $self.viewer <d> "$self pack_display_widget"

    bind $self.viewer <1>      "$self left_cut %x"
    bind $self.viewer <2>      "$self init_disp"
    bind $self.viewer <3>      "$self right_cut %x"

    bind $self.viewer <Up>         "$self next_signal -init"
    bind $self.viewer <Down>       "$self prev_signal -init"
    bind $self.viewer <Shift-Up>   "$self next_signal"
    bind $self.viewer <Shift-Down> "$self prev_signal"

    bind $self.viewer <a>      "$self set_disp_mode all"
    bind $self.viewer <Key-1>  "$self set_disp_mode one"

    bind $self.viewer <plus>   "$self set_current_disp_type plus"
    bind $self.viewer <period> "$self set_current_disp_type pixel"
    bind $self.viewer <minus>  "$self set_current_disp_type line"

    bind $self.viewer <r> "$self set_current_disp_color red"
    bind $self.viewer <w> "$self set_current_disp_color white"
    bind $self.viewer <g> "$self set_current_disp_color green"
    bind $self.viewer <b> "$self set_current_disp_color blue"
    bind $self.viewer <y> "$self set_current_disp_color yellow"

    bind $self.viewer <Meta-Right>   "$self x_change 4"
    bind $self.viewer <Meta-Left>    "$self x_change -4"
    bind $self.viewer <Control-Right> "$self x_change 40"
    bind $self.viewer <Control-Left> "$self x_change -40"
    bind $self.viewer <Meta-Down>    "$self y_change 4"
    bind $self.viewer <Meta-Up>      "$self y_change -4"
    bind $self.viewer <Control-Down> "$self y_change 40"
    bind $self.viewer <Control-Up>   "$self y_change -40"
    bind $self.viewer <Z>            "$self x_change 4;$self y_change 4"
    bind $self.viewer <Control-Z>    "$self x_change 40;$self y_change 40"
    bind $self.viewer <z>            "$self x_change -4;$self y_change -4"
    bind $self.viewer <Control-z>    "$self x_change -40;$self y_change -40"

    bind $self.viewer <f>            "$self sig_fit"
    bind $self.viewer <Control-2>    "$self sig_fit %x %y"
    bind $self.viewer <Control-1>    "$self nearest_signal %x %y"

    bind $self.viewer <Escape>       "$self def_cmd"

    #bind $self.viewer <Configure> {puts "lh"}

    #bind $self.viewer <Enter> "focus $self.viewer"

    focus $self.viewer
}

#
signal_viewer method add {sig {index end}} {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    set is_in [lsearch -exact $signals_list $sig]
    if {$is_in == -1} {
	linsert $signals_list $index $sig
    }
    $self init_disp

    return $sig
}

#
signal_viewer method + {{val 1}} {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    if {$current_signal_index < [expr [llength $signals_list]-$val]} {
	incr current_signal_index $val
	set current_signal [lindex $signals_list $current_signal_index]
	$self init_disp
    }
}

#
signal_viewer method - {{val 1}} {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    if {$current_signal_index >= $val} {
	incr current_signal_index -$val
	set current_signal [lindex $signals_list $current_signal_index]
	$self init_disp
    }
}

#
signal_viewer method def_cmd {} {
    entry $self.display.cmd_line -exportselection 1
    pack  $self.display.cmd_line -side top -padx 1m -pady 1m
    bind  $self.display.cmd_line <Return> "$self exec_cmd"
    set display_widget_packed true
    pack $self.display -fill both -side top
    set disp_width [lindex [w_size $self.display] 0]
    set disp_height [lindex [w_size $self.display] 1]

    focus $self.display.cmd_line
}

#
signal_viewer method exec_cmd {} {
    set cmd [$self.display.cmd_line get]
    if {$cmd != ""} {
	if {[lsearch -exact [$self info methods] $cmd] == -1} {
	    set err_code [catch {uplevel #0 $cmd} res_str]
	} else {
	    set err_code [catch {eval $self $cmd} res_str]
	}
	$self.display.cmd_res configure -text $res_str
    }
    destroy $self.display.cmd_line
    focus $self.viewer
}

#
signal_viewer method current {{sig ""}} {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    set index 0
    if {$sig == ""} {
	return $current_signal
    } else {
	foreach signal $signals_list {
	    if {[string compare $signal $sig] == 0} {
		set current_signal_index $index
		set current_signal $sig
		$self refresh_disp

		return $current_signal
	    }
	    incr index
	}
    }
    return 0
}

#
signal_viewer method convert_coordinates {x y} {
    instvar viewer_xsize viewer_ysize
    instvar x_min x_max y_min y_max
    set border 20

    set new_x [expr $x_min+($x-$border)*($x_max-$x_min)/($viewer_xsize)]
    set new_y [expr $y_min+($viewer_ysize-$y+$border)*($y_max-$y_min)/($viewer_ysize)]
    return [list $new_x $new_y]
}

#
signal_viewer method nearest_signal {x y} {
    instvar signals_dx
    instvar signals_x0
    instvar signals_size
    instvar viewer_xsize viewer_ysize
    instvar x_min x_max y_min y_max
    instvar signals_list
    instvar display_mode
    instvar default_disp_type
    instvar disp_width
    instvar disp_height
    instvar current_signal
    instvar current_signal_index

    set x_y [$self convert_coordinates $x $y]
    set x [lindex $x_y 0]
    set y [lindex $x_y 1]

    set signal [lindex $signals_list 0]
    set diff_min [expr abs($y-[sget $signal [sgetindex $signal $x]])]
    set nearest $signal
    foreach signal $signals_list {
	set diff [expr abs($y-[sget $signal [sgetindex $signal $x]])]
	if {$diff < $diff_min} {
	    set nearest $signal
	    set diff_min $diff
	}
    }
    return $nearest
}

#
signal_viewer method sig_fit {{x -1} {y -1}} {
    instvar signals_dx
    instvar signals_x0
    instvar signals_size
    instvar viewer_xsize viewer_ysize
    instvar x_min x_max y_min y_max
    instvar signals_list
    instvar display_mode
    instvar default_disp_type
    instvar disp_width
    instvar disp_height
    instvar current_signal
    instvar current_signal_index

    if {$x != -1 &&$y != -1 && [string compare $display_mode "one"]} {
	set signal [$self nearest_signal $x $y]
    } else {
	set signal $current_signal
    }

    set fit [sfit $signal [expr $x_min-(0.00001*($x_max-$x_min))] [expr $x_max+(0.00001*($x_max-$x_min))]]
    set a [lindex $fit 0]
    set b [lindex $fit 1]

    #set x1 $signals_x0
    #set x2 [expr ($signals_x0+$signals_dx*$signals_size)]
    set x1 $x_min
    set x2 $x_max
    set y1 [expr ($a*$x1+($b))]
    set y2 [expr ($a*$x2+($b))]
    
    draw_line $self.viewer.content \
	    $x1 $y1 $x2 $y2 \
	    $x_min $x_max $y_min $y_max \
	    $viewer_xsize $viewer_ysize \
	    -color blue
    $self.viewer -configure
    set string [format "fit :\n%f x\n+ %f" $a $b]
    $self.display.fit configure -text $string
}

#
signal_viewer method prev_signal args {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    if {$current_signal_index < [expr [llength $signals_list]-1]} {
	incr current_signal_index
	set current_signal [lindex $signals_list $current_signal_index]
	if {[string compare $args "-init"]} {
	    $self refresh_disp
	} else {
	    $self init_disp
	}
    }
}

#
signal_viewer method next_signal args {
    instvar signals_list
    instvar current_signal
    instvar current_signal_index

    if {$current_signal_index > 0} {
	incr current_signal_index -1
	set current_signal [lindex $signals_list $current_signal_index]
	if {[string compare $args "-init"]} {
	    $self refresh_disp
	} else {
	    $self init_disp
	}
    }
}

#
signal_viewer method set_disp_mode {mode} {
    instvar display_mode

    set display_mode $mode
    $self refresh_disp
}

#
signal_viewer method init_values {} {
    instvar current_signal
    instvar x_min x_max y_min y_max
    instvar signals_dx
    instvar display_mode
    instvar signals_list

    switch $display_mode {
	one {
	    if {[sgettype $current_signal] != 0} {
		set signal_x0 [sgetx0 $current_signal]
		set signal_size [ssize $current_signal]
		set x_min $signal_x0
		set x_max [expr $signal_x0+${signal_size}*${signals_dx}]
		
		set tmp_list [sgetextr $current_signal]
		set y_min [lindex $tmp_list 0]
		set y_max [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $current_signal]

		set x_min [lindex $tmp_list 0]
		set x_max [lindex $tmp_list 1]
		set y_min [lindex $tmp_list 2]
		set y_max [lindex $tmp_list 3]
	    }
	}
	all {
	    if {[sgettype $current_signal] != 0} {
		set signal_x0 [sgetx0 $current_signal]
		set signal_size [ssize $current_signal]
		set x_min $signal_x0
		set x_max [expr $signal_x0+${signal_size}*${signals_dx}]
		
		set tmp_list [sgetextr $current_signal]
		set y_min [lindex $tmp_list 0]
		set y_max [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $current_signal]

		set x_min [lindex $tmp_list 0]
		set x_max [lindex $tmp_list 1]
		set y_min [lindex $tmp_list 2]
		set y_max [lindex $tmp_list 3]
	    }
	    foreach signal $signals_list {
		if {[string compare $signal $current_signal] != 0} {
		    set signal_x0 [sgetx0 $signal]
		    set signal_size [ssize $signal]
		    set tmp_x_min $signal_x0
		    if {$x_min > $tmp_x_min} {set x_min $tmp_x_min}
		    set tmp_x_max [expr $signal_x0+${signal_size}*${signals_dx}]
		    if {$x_max < $tmp_x_max} {set x_max $tmp_x_max}

		    set tmp_list [sgetextr $signal]
		    set tmp_y_min [lindex $tmp_list 0]
		    if {$y_min > $tmp_y_min} {set y_min $tmp_y_min}
		    set tmp_y_max [lindex $tmp_list 1]
		    if {$y_max < $tmp_y_max} {set y_max $tmp_y_max}
		}
	    }
	}
    }
}

#
signal_viewer method set_new_extrema {} {
    instvar signal_size current_signal
    instvar x_min x_max y_min y_max
    instvar display_mode
    instvar signals_list

    switch $display_mode {
	one {
	    if {[sgettype $current_signal] != 0} {
		set index_min [sgetindex $current_signal $x_min]
		set index_max [sgetindex $current_signal $x_max]
		set tmp_list [sgetextr $current_signal -index $index_min $index_max]

		set y_min [lindex $tmp_list 0]
		set y_max [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $current_signal -x $x_min $x_max]
		set x_min [lindex $tmp_list 0]
		set x_max [lindex $tmp_list 1]
		set y_min [lindex $tmp_list 2]
		set y_max [lindex $tmp_list 3]
	    }
	}
	all {
	    if {[sgettype $current_signal] != 0} {
		set index_min [sgetindex $current_signal $x_min]
		set index_max [sgetindex $current_signal $x_max]
		set tmp_list [sgetextr $current_signal -index $index_min $index_max]

		set y_min [lindex $tmp_list 0]
		set y_max [lindex $tmp_list 1]
	    } else {
		set tmp_list [sgetextr $current_signal -x $x_min $x_max]
		set x_min [lindex $tmp_list 0]
		set x_max [lindex $tmp_list 1]
		set y_min [lindex $tmp_list 2]
		set y_max [lindex $tmp_list 3]
	    }
	    foreach signal $signals_list {
		if {[sgettype $current_signal] != 0} {
		    set index_min [sgetindex $current_signal $x_min]
		    set index_max [sgetindex $current_signal $x_max]
		    set tmp_list [sgetextr $current_signal -index $index_min $index_max]
		    
		    set tmp_y_min [lindex $tmp_list 0]
		    set tmp_y_max [lindex $tmp_list 1]
		} else {
		    set tmp_list [sgetextr $current_signal -x $x_min $x_max]
		    set x_min [lindex $tmp_list 0]
		    set x_max [lindex $tmp_list 1]
		    set tmp_y_min [lindex $tmp_list 2]
		    set tmp_y_max [lindex $tmp_list 3]
		}
		if {$y_min > $tmp_y_min} {set y_min $tmp_y_min}
		if {$y_max < $tmp_y_max} {set y_max $tmp_y_max}
	    }
	}
    }
}

#
signal_viewer method left_cut {pos_x} {
    instvar viewer_xsize viewer_ysize
    instvar x_min x_max y_min y_max
    instvar signals_dx
    instvar signals_x0
    instvar current_signal

    set tmp [expr $x_min+(($pos_x-20)*($x_max-$x_min)/($viewer_xsize))]
    if {[sgettype $current_signal] != 0} {
	set tmp [expr ceil(($tmp-$signals_x0)/$signals_dx)]
	set tmp [expr $tmp*$signals_dx+$signals_x0]
    }
    if {($tmp > $x_min) && ($tmp < $x_max)} {
	set x_min $tmp
	$self set_new_extrema
	$self refresh_viewer
    }
}

#
signal_viewer method right_cut {pos_x} {
    instvar viewer_xsize viewer_ysize
    instvar x_min x_max y_min y_max
    instvar signals_dx
    instvar signals_x0
    instvar current_signal

    set tmp [expr $x_min+(($pos_x-20)*($x_max-$x_min)/($viewer_xsize))]
    if {[sgettype $current_signal] != 0} {
	set tmp [expr floor(($tmp-$signals_x0)/$signals_dx)]
	set tmp [expr $tmp*$signals_dx+$signals_x0]
    }
    if {($tmp > $x_min) && ($tmp < $x_max)} {
	set x_max $tmp
	$self set_new_extrema
	$self refresh_viewer
    }
}

signal_viewer method init_disp {} {
    $self refresh_info
    $self init_values
    $self refresh_viewer
}

signal_viewer method refresh_disp {} {
    $self refresh_info
    $self refresh_viewer
}

# 
signal_viewer method refresh_viewer {} {
    instvar x_min x_max y_min y_max
    instvar viewer_xsize viewer_ysize
    instvar current_signal
    instvar display_mode
    instvar signals_list

    init_viewer $self.viewer.content \
	    $x_min $x_max $y_min $y_max \
	    $viewer_xsize $viewer_ysize
    switch $display_mode {
	one {
	    set current_disp_type \
		    [disp_par2_${self}_$current_signal set disp_type]
	    set current_disp_color \
		    [disp_par2_${self}_$current_signal set disp_color]
	    draw_signal $self.viewer.content $current_signal\
		    $x_min $x_max $y_min $y_max \
		    $viewer_xsize $viewer_ysize \
		    -type $current_disp_type \
		    -color $current_disp_color
	}
	all {
	    foreach signal $signals_list {
		set current_disp_type \
			[disp_par2_${self}_$signal set disp_type]
		set current_disp_color \
			[disp_par2_${self}_$signal set disp_color]
		draw_signal $self.viewer.content $signal\
			$x_min $x_max $y_min $y_max \
			$viewer_xsize $viewer_ysize \
			-type $current_disp_type \
			-color $current_disp_color
	    }
	}
    }
}

# 
signal_viewer method set_disp_type {signal disp_type} {
    instvar signals_list
    
    foreach sig $signals_list {
	if {![string compare $sig $signal]} {
	    disp_par2_${self}_$signal set disp_type $disp_type
	    $self refresh_disp
	}
    }
}

# 
signal_viewer method set_current_disp_type {disp_type} {
    instvar current_signal

    disp_par2_${self}_$current_signal set disp_type $disp_type
    $self refresh_disp
}

# 
signal_viewer method set_current_disp_color {disp_color} {
    instvar current_signal

    disp_par2_${self}_$current_signal set disp_color $disp_color
    $self refresh_disp
}

# 
signal_viewer method refresh_info {} {
    instvar current_signal
    instvar signals_list

    #$self.display.curr_name configure -text $current_signal
    #pack    $self.display.curr_name  -side top -padx 1m -pady 1m
    set string ""
    foreach signal $signals_list {
	set type [disp_par2_${self}_$signal set disp_type]
	set type [get_symbol_from_type $type]
	set color [disp_par2_${self}_$signal set disp_color]
	if {[string compare $signal $current_signal]} {
	    set string "$string\n  $signal $type $color"
	} else {
	    set string "$string\n* $signal $type $color"
	}
    }
    $self.display.names configure -text $string
    pack    $self.display.names  -side top -padx 1m -pady 1m
}

# 
# 
signal_viewer method x_change {nb_pixels} {
    instvar viewer_xsize

    incr viewer_xsize $nb_pixels
    if {$viewer_xsize < 125} {
	set $viewer_xsize 125
    }
    
    $self init_disp
    $self.viewer content $self.viewer.content
}

# 
# 
signal_viewer method y_change {nb_pixels} {
    instvar viewer_ysize

    incr viewer_ysize $nb_pixels
    if {$viewer_ysize < 125} {
	set $viewer_ysize 125
    }

    $self init_disp
    $self.viewer content $self.viewer.content
}

# A revoir. Ca marche po.
# 
signal_viewer method change_size {size} {
    instvar viewer_xsize
    instvar viewer_ysize
    instvar display_widget_packed
    instvar disp_width
    instvar disp_height

    set change "no"

    set width  [lindex $size 0]
    set height [lindex $size 1]

    if {$display_widget_packed == "true"} {
	set disp_width [lindex [w_size $self.display] 0]
    } else {
    	set disp_width 0
    }

    if {$width != [expr $viewer_xsize+46+$disp_width]} {
	set viewer_xsize [expr $width-46-$disp_width]
	set change "yes"
    }
    if {$height != [expr $viewer_ysize+46]} {
	set viewer_ysize [expr $height-46]
	set change "yes"
    }

    if {$change == "yes"} {
	#$self init_disp
	#$self.viewer content $self.viewer.content
    }
}

signal_viewer method pack_display_widget {} {
    instvar display_widget_packed
    instvar viewer_xsize
    instvar viewer_ysize
    instvar disp_width
    instvar disp_height

    if {$display_widget_packed == "true"} {
	set display_widget_packed false
	pack forget $self.display
	set disp_width 0
	set disp_height 0
    } else {
	set display_widget_packed true
	pack $self.display -fill both -side top
	set disp_width [lindex [w_size $self.display] 0]
	set disp_height [lindex [w_size $self.display] 1]
    }
}

signal_viewer method refresh_all_widgets {} {
    instvar display_widget_packed

    pack   $self.viewer -padx 1m -pady 1m -side left -fill both
    if {$display_widget_packed == "true"} {
	pack $self.display -fill both -side left
    } else {
	pack forget $self.display
    }
}

###
#
#
proc nsaff  {signal {args ""}} {
    global viewNb

    if {"S" != [gettype $signal]} {
	return "$signal is not a signal."
    }
    set signals_list $args
    if {[sgettype $signal] != 0} {
	set signals_dx [sgetdx $signal]
	set signals_x0 [sgetx0 $signal]
    } else {
	set signals_dx 1
	set signals_x0 0
    }
    set signals_size [ssize $signal]
    if {[sgettype $signal] != 0} {
	foreach sig $signals_list {
	    if {"S" != [gettype $sig]} {
		return "$sig is not a signal."
	    }
	    if {$signals_dx != [sgetdx $sig]} {
		return "Signals don't have the same dx value."
	    }
	    if {$signals_x0 != [sgetx0 $sig]} {
		return "Signals don't have the same x0 value."
	    }
	    if {$signals_size != [ssize $sig]} {
		return "Signals don't have the same size."
	    }
	}
    }
    toplevel .v$viewNb
    eval "signal_viewer .v$viewNb.sv $signal $args"
    pack .v$viewNb.sv -fill both
    bind .v$viewNb <Control-x> "destroy .v$viewNb"

    wm resizable .v$viewNb 0 0
    wm geometry .v$viewNb +1+1
    incr viewNb
    return .v$viewNb.sv
}

proc aff_size {widget} {
    puts [w_size $widget]
}

###
#
#
proc get_symbol_from_type {type} {
    switch $type {
	plus  {set str "+"}
	line  {set str "-"}
	pixel {set str "."}
    }
    return $str
}