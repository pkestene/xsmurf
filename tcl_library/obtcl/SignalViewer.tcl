
class SignalViewer
SignalViewer inherit SmurfViewer

SignalViewer method init args {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal
    instvar signal_dx signal_x0

    next

    set zoom 1
    set val ""
    set current_signal [lindex $args 0]
    set signal_first [sfirst $current_signal]
    set signal_last [slast $current_signal]
    set signal_size [ssize $current_signal]
    set signal_dx [sgetdx $current_signal]
    set signal_x0 [sgetx0 $current_signal]
    set signal_xmin $signal_first
    set signal_xmax $signal_last

    set viewer_xsize 400
    set viewer_ysize 300

    #pack   $self.viewer -padx 1m -pady 1m -side bottom

    #viewer $self.viewer.signal
    #pack   $self.viewer.signal -padx 1m -pady 1m -side bottom

    focus $self

    if {[lindex $args 1] == "-axes"} {
	init_viewer $self.viewer.content -1.876 2.978437 -9.77 12.9744
    } else {
	sconvert $current_signal $self.viewer.content \
		-rangeX $signal_xmin $signal_xmax \
		-width  $viewer_xsize \
		-height $viewer_ysize
    }

    $self.viewer content $self.viewer.content

    label $self.display.name -fg blue3 -text $current_signal
    pack    $self.display.name  -side right -padx 1m -pady 1m

    # actions associees aux boutons
    #	bind $self.viewer <Down>  "$self down"
    #	bind $self.viewer <Up>    "$self up"
    #bind $self.viewer <Right> "$self right"
    #bind $self.viewer <Left>  "$self left"
    bind $self.viewer <1>     "$self left_cut %x"
    bind $self.viewer <2>     "$self init_disp"
    bind $self.viewer <3>     "$self right_cut %x"
    bind $self.viewer <Enter> "focus $self.viewer"

    focus $self.viewer
}

SignalViewer method left_cut {pos_x} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set tmp [expr $signal_xmin+1+(($pos_x-20)*($signal_xmax-$signal_xmin)/($viewer_xsize))]
    if {($tmp > $signal_xmin) && ($tmp < $signal_xmax)} {
	set signal_xmin $tmp
	sconvert $current_signal $self.viewer.content \
		-rangeX $signal_xmin $signal_xmax \
		-width  $viewer_xsize \
		-height $viewer_ysize
    }
}

SignalViewer method right_cut {pos_x} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set tmp [expr $signal_xmin+(($pos_x-20)*($signal_xmax-$signal_xmin)/($viewer_xsize))]
    if {($tmp > $signal_xmin) && ($tmp < $signal_xmax)} {
	set signal_xmax $tmp
	sconvert $current_signal $self.viewer.content \
		-rangeX $signal_xmin $signal_xmax \
		-width  $viewer_xsize \
		-height $viewer_ysize
    }
}

SignalViewer method init_disp {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set signal_xmin $signal_first
    set signal_xmax $signal_last
    #	set signal_xmax [expr $signal_size+$signal_first-1]
    sconvert $current_signal $self.viewer.content \
	    -rangeX $signal_xmin $signal_xmax \
	    -width  $viewer_xsize \
	    -height $viewer_ysize
    #	$self.viewer create rectangle 1c 1c 1c 1c -outline yellow -fill red
}

SignalViewer method vcut {pos_x} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    icut $self sigcut -vertical [expr round($pos_x/$zoom)]
    saff sigcut "$self : vertical [expr round($pos_x/$zoom)]"
}

SignalViewer method hcut {pos_y} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    icut $self sigcut -horizontal [expr round($pos_y/$zoom)]
    saff sigcut "$self : horizontal [expr round($pos_y/$zoom)]"
}

SignalViewer method down {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { $y < [expr $size-1]} {incr y}
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method up {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { $y > 0 } {incr y -1}
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method right {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { $x < [expr $size-1]} {incr x}
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method left {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { $x > 0} {incr x -1}
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method mouse_goto {pos_x pos_y} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set x [expr round($pos_x/$zoom)]
    set y [expr round($pos_y/$zoom)] 
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method goto {pos_x pos_y} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set x $pos_x
    set y $pos_y 
    set val [value $self $x $y]
    $self display_val
}

SignalViewer method display_val {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    $self.display.x_val   configure -text "[set x]" 
    $self.display.y_val   configure -text "[set y]" 
    $self.display.val_val configure -text "[set val]" 
}

SignalViewer method zoom_incr {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { [set zoom] < 6 } {
	incr zoom 
	$self.tools.zoommsg configure -text "[set zoom]" 
	#	    sconvert $current_signal $self.viewer.content -zoom $zoom $options
    }
}
SignalViewer method zoom_decr {} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    if { [set zoom] > 1 } {
	incr zoom -1
	$self.tools.zoommsg configure -text "[set zoom]"
	#	    sconvert $current_signal $self.viewer.content -zoom $zoom $options
    }
}

SignalViewer method zoom {new_zoom} {
    instvar zoom size signal_xmin signal_xmax signal_first signal_last
    instvar signal_size viewer_xsize viewer_ysize current_signal

    set zoom $new_zoom
    if {$zoom > 6} {set zoom 6}
    if {$zoom < 1} {set zoom 1}
    $self.tools.zoommsg configure -text "[set zoom]"
    #	sconvert $current_signal $self.viewer.content -zoom $zoom $options
}

proc saff  {signal {args ""}} {
    toplevel .v$signal
    eval "SignalViewer .v$signal.sv $signal $args"
    pack .v$signal.sv
    bind .v$signal <c> "destroy .v$signal"
}
