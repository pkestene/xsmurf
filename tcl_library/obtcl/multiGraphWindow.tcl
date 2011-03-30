# multiGraphWindow.tcl
#
#       This file implements the Tcl code for creating windows that display
#       signals. The signals can be grouped by graphs. 
#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster.
#
#  RCS : $Id: multiGraphWindow.tcl,v 1.16 1999/06/25 19:52:21 decoster Exp $
#

# last modified by Pierre Kestener (2000/06/26).

class multiGraphWindow
multiGraphWindow inherit Widget

# grW 750 ou 600 ou 400
# grH 500 ou 450 ou 300
multiGraphWindow method init {gw gh graph_descr_lst {grW 750} {grH 500}} {
    # Size of the graph arrea (i.e. size of the canvas).
    instvar cvWidth
    instvar cvHeight
    # Size of the grid
    instvar gridWidth
    instvar gridHeight
    # List of graph.
    instvar grList
    # List of the position of the graphs on the grid.
    instvar grIPosList
    instvar grJPosList
    # Size of each graph.
    instvar grWidth
    instvar grHeight
    # Current graph.
    instvar curGr

    # Multi-graph label.
    instvar mgLabel
    instvar mgLabelHeight
    instvar mgLabelFont

    # Current command to use iwth the entry value
    instvar entryCommand
    # Last command result string
    instvar resStr

    # Flag that indicates that the next execution of the gr method will be
    # applied on all the graphs from grList.
    instvar allGraphFlag

    next

    set mgLabelFont my_font

    #set grWidth 400 ou 750
    set grWidth $grW
    #set grHeight 300 ou 500
    set grHeight $grH

    if {($gw <= 0)} {
	error "invalid grid width"
    }
    if {($gh <= 0)} {
	error "invalid grid height"
    }

    set mgLabelHeight 30
    set mgLabel "no label"

    set gridWidth $gw
    set gridHeight $gh
    set cvWidth [expr $gridWidth*$grWidth]
    set cvHeight [expr $mgLabelHeight+$gridHeight*$grHeight]
    set allGraphFlag no

    canvas $self.cv -width $cvWidth -height $cvHeight
    pack $self.cv -fill both -expand 1

    # Initialize the graphs.
    set grList ""
    set grIPosList ""
    set grJPosList ""
    for {set i 0} {$i < $gridWidth} {incr i} {
	for {set j 0} {$j < $gridHeight} {incr j} {
	    set sig_lst [lindex $graph_descr_lst 0]
	    set graph_descr_lst [lrange $graph_descr_lst 1 end]
	    if {$sig_lst != ""} {
		set gr_name "${self}gr[format "%.2d%.2d" $i $j]"
		set grList [concat $grList $gr_name]
		set grIPosList [concat $grIPosList $i]
		set grJPosList [concat $grJPosList $j]
		myGraph $gr_name $self.cv\
			$grWidth $grHeight \
			[expr $i*$grWidth] [expr $mgLabelHeight+$j*$grHeight] \
			$sig_lst
		set curGr $gr_name
	    }
	}
    }

    $self.cv create text \
	    [expr $cvWidth/2] [expr $mgLabelHeight/2] \
	    -text $mgLabel \
	    -font $mgLabelFont \
	    -tags mgLabel

    set entryCommand ""

    # Define the in/out frame.
    frame $self.io -relief raised -bd 1
    pack  $self.io -fill x -side bottom
    pack  $self.cv -after $self.io

    label $self.io.out -text label -anchor w
    pack  $self.io.out -fill x -side top -expand 1
    entry $self.io.in -bd 1 -exportselection 1
    pack  $self.io.in -fill x -side top -expand 1
    pack  $self.io.in -after $self.io.out

    bind $self.cv <Motion>       "$self set_current_graph %x %y"
    bind $self.cv <Motion>       "+ $self gr the_motion %x %y"
    bind $self.cv <Configure>    "$self disp_configure"

    bind $self.cv <Escape><x>       "$self def_cmd"
    bind $self.io.in <Return>       "$self exec_cmd"
    bind $self.io.in <Control-g>    "$self quit_entry"
    bind $self.io.in <Control-e><c> "$self choose_color"

    #eval "set keyScript {$self disp_key_press %K}"
    #bind $self.cv <Escape>     +$keyScript

    bind $self.cv <1>            "$self gr left_cut %x"
    bind $self.cv <2>            "$self gr init_disp"
    bind $self.cv <3>            "$self gr right_cut %x"

    bind $self.cv <Up>           "$self gr next_signal"
    bind $self.cv <Down>         "$self gr prev_signal"
    bind $self.cv <Shift-Up>     "$self gr next_signal -init"
    bind $self.cv <Shift-Down>   "$self gr prev_signal -init"

    bind $self.cv <Control-x><Control-Key-1> "$self gr set_disp_mode one_and_list"
    bind $self.cv <Control-x><a>            "$self gr set_disp_mode all"
    bind $self.cv <Control-x><Key-1>        "$self gr set_disp_mode one"
    bind $self.cv <Control-x><A>            "$self gr add_cur_sig_to_display_list"
    bind $self.cv <Control-x><R>            "$self gr remove_cur_sig_to_display_list"

    bind $self.cv <Control-x><X>            "$self gr set_log_mode xlog"
    bind $self.cv <Control-x><Y>            "$self gr set_log_mode ylog"
    bind $self.cv <Control-x><L>            "$self gr set_log_mode normal"

    bind $self.cv <Control-x><plus>         "$self gr set_type plus"
    bind $self.cv <Control-x><o>            "$self gr set_type circle"
    bind $self.cv <Control-x><O>            "$self gr set_type bigcircle"
    bind $self.cv <Control-x><x>            "$self gr set_type cross"
    bind $self.cv <Control-x><minus>        "$self gr set_type line"
    bind $self.cv <Control-x><t>            "$self def_cmd set_type"

    bind $self.cv <Control-x><r>            "$self gr set_color red"
    bind $self.cv <Control-x><w>            "$self gr set_color white"
    bind $self.cv <Control-x><g>            "$self gr set_color green"
    bind $self.cv <Control-x><b>            "$self gr set_color blue"
    bind $self.cv <Control-x><B>            "$self gr set_color black"
    bind $self.cv <Control-x><y>            "$self gr set_color yellow"
    bind $self.cv <Control-x><c> "$self def_cmd set_color"

    bind $self.cv <Control-x><m> "$self def_cmd {$self setColorsByList}"

    bind $self.cv <Control-x><p> "$self def_cmd print"

    bind $self.cv <Control-x><l> "$self def_cmd set_gr_label"

    bind $self.cv <Control-x><f> "$self gr sig_fit"
    bind $self.cv <Control-2>    "$self gr sig_fit %x %y"
    bind $self.cv <Control-x><z> "$self gr sig_fit %x %y yes"
    bind $self.cv <Control-1>    "$self gr nearest_signal %x %y"

    bind $self.cv <Control-3>    "$self sig_menu %x %y"

    bind $self.cv <Control-a>    "$self switch_allgraph_flag"

    bind $self.cv <Control-s><b>    "$self gr set_box_coord 0 5 -.8 .8"


    focus $self.cv
}

#
multiGraphWindow method destroy {args} {
    instvar grList

    foreach gr $grList {
	$gr destroy
    }
    next
}

#
multiGraphWindow method switch_allgraph_flag {} {
    instvar allGraphFlag

    if {$allGraphFlag == "no"} {
	set allGraphFlag yes
    } else {
	set allGraphFlag no
    }
}

#
multiGraphWindow method choose_color {} {
    set color [tk_chooseColor]
    $self.io.in insert insert $color
}

#
multiGraphWindow method setColorsByList {colorList} {
    instvar grList

    foreach graph $grList {
	$graph setColorsByList $colorList
    }
}

#
multiGraphWindow method setLabelsItemsByList args {
    instvar grList

    foreach graph $grList {
	eval $graph setLabelsItemsByList $args
    }
}

#
multiGraphWindow method set_gr_label args {
    instvar curGr

    eval $curGr setLabelsItemsByList $args
}

#
multiGraphWindow method setLabel {newLabel {nbOfLines 1}} {
    instvar mgLabel
    instvar mgLabelHeight
    instvar mgLabelFont
    instvar cvWidth
    instvar cvHeight

    set newMgLabelHeight [expr { 24*$nbOfLines+6 }]

    if {$mgLabelHeight == $newMgLabelHeight} {
	set mgLabel $newLabel
	$self.cv delete -withtag mgLabel
	$self.cv create text \
		[expr $cvWidth/2] [expr $mgLabelHeight/2] \
		-text $mgLabel \
		-font $mgLabelFont \
		-tags mgLabel
    } else {
	# Without the following line there is a bug in disp_configure (?!?).
	puts ""
	set mgLabel $newLabel
	set mgLabelHeight $newMgLabelHeight
	$self disp_configure -force
    }
}

#
multiGraphWindow method sig_menu {{x 100} {y 100}} {
    instvar curGr

    set sigList [$curGr get_sig_list]
    destroy .grMenu
    menu .grMenu -border 1 -tearoff 0
    .grMenu add command -label "close menu"
    foreach sigElt $sigList {
	set sig   [lindex $sigElt 0]
	set color [lindex $sigElt 1]
	set type  [lindex $sigElt 2]
	.grMenu add command \
		-label $sig \
		-foreground $color \
		-command "$curGr current $sig;destroy .grMenu"
    }
    set rootx [expr $x+[winfo rootx $self]]
    set rooty [expr $y+[winfo rooty $self]]
    .grMenu post $rootx $rooty
}

#
multiGraphWindow method disp_key_press {key} {
    $self.io.out configure -text "$key"
}

#
multiGraphWindow method print {file_name args} {
    instvar cvWidth
    instvar cvHeight

    # default values
    if {$cvWidth > $cvHeight} {
	set rotate 1
    } else {
	set rotate 0
    }
    set ratio 1

    # arguments analysis
    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    -portrait {
		set rotate 0
		set args [lreplace $args 0 0]
	    }
	    -landscape {
		set rotate 1
		set args [lreplace $args 0 0]
	    }
	    -ratio {
		set ratio [lindex $args 1]
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

    $self.cv postscript \
	    -file $file_name \
	    -pagewidth [expr $cvWidth*0.8*$ratio] \
	    -rotate $rotate

    return [format "%.4gcm x %.4gcm" [expr $cvWidth*$ratio*0.03528] [expr $cvHeight*$ratio*0.03528]]
}

#
multiGraphWindow method def_cmd {{cmd ""}} {
    $self.io.in insert 0 $cmd
    focus $self.io.in
}

#
multiGraphWindow method quit_entry {} {
    $self.io.in delete 0 end
    focus $self.cv
    $self.io.out configure -text "quit entry"
}

#
multiGraphWindow method exec_cmd {} {
    instvar curGr
    instvar resStr

    set cmd [$self.io.in get]
    if {[string compare $cmd "destroy"] == 0} {
	$self.io.out configure -text "use <C-x C-d> to kill this window."
    } elseif {$cmd != ""} {
	if {[lsearch -exact [$curGr info methods] [lindex $cmd 0]] != -1} {
	    set err_code [catch {eval $curGr $cmd} res_str]
	    $self.io.out configure -text "$res_str"
	    set resStr $res_str
	} else {
	    if {[lsearch -exact [$self info methods] [lindex $cmd 0]] != -1} {
		set err_code [catch {eval $self $cmd} res_str]
		$self.io.out configure -text "$res_str"
		set resStr $res_str
	    } else {
		set err_code [catch {uplevel #0 $cmd} res_str]
		$self.io.out configure -text "$res_str"
		set resStr $res_str
	    }
	}
    }
    $self.io.in delete 0 end
    focus $self.cv
}

# Tres crade... a changer au plus vite...
multiGraphWindow method puts_r {} {
    instvar resStr
    puts $resStr
}

#
multiGraphWindow method disp_configure args {
    instvar grList
    instvar cvWidth
    instvar cvHeight
    instvar grWidth
    instvar grHeight
    instvar gridWidth
    instvar gridHeight
    instvar grIPosList
    instvar grJPosList
    instvar mgLabel
    instvar mgLabelHeight
    instvar mgLabelFont

    set w [winfo width $self.cv]
    set h [winfo height $self.cv]
    if {($w != $cvWidth) || ($h != $cvHeight) || [string compare $args "-force"] == 0} {
	set cvWidth  [winfo width $self.cv]
	set cvHeight [winfo height $self.cv]
	set grWidth [expr int($cvWidth/$gridWidth)]
	set grHeight [expr int(($cvHeight-$mgLabelHeight)/$gridHeight)]
	foreach i $grIPosList j $grJPosList {
	    set graph "${self}gr[format "%.2d%.2d" $i $j]"
	    if {[lsearch $grList $graph] != -1} {
		$graph change_geometry \
			$grWidth $grHeight \
			[expr $i*$grWidth] [expr $mgLabelHeight+$j*$grHeight]
	    }
	}
    }
    $self.cv delete -withtag mgLabel
    $self.cv create text \
	    [expr $cvWidth/2] [expr $mgLabelHeight/2] \
	    -text $mgLabel \
	    -font $mgLabelFont \
	    -tags mgLabel
}

#
multiGraphWindow method refresh_disp {} {
    instvar grList
    
    foreach graph $grList {
	$graph refresh_disp
    }
}

#
multiGraphWindow method init_disp {} {
    instvar grList
    
    foreach graph $grList {
	$graph init_disp
    }
}

#
multiGraphWindow method gr {cmd args} {
    instvar grList
    instvar curGr
    instvar allGraphFlag

    switch $allGraphFlag {
	no {
	    set err_code [catch {eval $curGr $cmd $args} res_str]
	    $self.io.out configure -text "$res_str"
	    set resStr $res_str
	}
	yes {
	    foreach graph $grList {
		set err_code [catch {eval $graph $cmd $args} res_str]
		$self.io.out configure -text "$res_str"
		set resStr $res_str
	    }
	}
    }
}

#
multiGraphWindow method set_current_graph {x y} {
    instvar cvWidth
    instvar cvHeight
    instvar gridWidth
    instvar gridHeight
    instvar curGr

    set i [expr $x*$gridWidth/$cvWidth]
    set j [expr $y*$gridHeight/$cvHeight]
    set curGr "${self}gr[format "%.2d%.2d" $i $j]"
}

#
multiGraphWindow method get_current_graph {} {
    instvar curGr
    return $curGr
}

#
multiGraphWindow method getGrList {} {
    instvar grList

    return $grList
}

#
multiGraphWindow method getgrWidth {} {
    instvar grWidth

    return $grWidth
}

#
multiGraphWindow method getgrHeight {} {
    instvar grHeight

    return $grHeight
}

#
multiGraphWindow method getcvWidth {} {
    instvar cvWidth

    return $cvWidth
}

#
multiGraphWindow method getcvHeight {} {
    instvar cvHeight

    return $cvHeight
}

# mdisp --
# usage : mdisp int int str
#
#  Display lists of signals in a multi-graph manner.
#
# Parameters :
#   2 integers - define a grid of graph by the number of rows and the number
#                of lines.
#   string     - list of signals lists. The first signal list is for the for
#                The first graph, the 2nd list for the 2nd graph, etc.
#
# Return value :
#   The name of the object that manage this multi-graph window.
#
# Example :
#     mdisp 2 1 {{s1 s2} {sinus}}
#   This command line open a window that displays 2 graphs. "s1" and "s2" are
# displayed in the first graph, and "sinus" is displayed in the 2nd one.

proc mdisp  {args} {
    global viewNb
    global errorInfo
    toplevel .md$viewNb
    set code [catch {eval "multiGraphWindow .md$viewNb.m $args"} result]
    if {$code != 0} {
	destroy .md$viewNb
	error $result $result
    } else {
	pack .md$viewNb.m -fill both -expand 1
	bind .md$viewNb <Control-x><Control-d> "destroy .md$viewNb"
	wm geometry .md$viewNb +1+1
	set mgName .md$viewNb.m
	incr viewNb
	return $mgName
    }
}

# sdisp --
# usage : sdisp str
#
#  Display a list of signals in a single graph.
#
# Parameters :
#   string - list of signals to display.
#
# Return value :
#   The name of the object that manage this graph window.

proc sdisp {sig} {
    set code [catch "mdisp 1 1 {{$sig}}" result]
    if {$code != 0} {
	error $result $result
    } else {
	return $result
    }
}
