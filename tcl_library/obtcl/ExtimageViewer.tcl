# ExtimageViewer.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: ExtimageViewer.tcl,v 1.10 1999/05/22 16:58:18 decoster Exp $
#

# modified by Pierre Kestener.

class Extimage
Extimage inherit Base

Extimage method init {} {
    instvar mod arg x y bend

    next

    set mod ""
    set arg ""
    set x ""
    set y ""
    set bend ""
}

Extimage method x {} {
    instvar x

    return $x
}

Extimage method y {} {
    instvar y

    return $y
}
    
Extimage method mod {} {
    instvar mod

    return $mod
}

Extimage method arg {} {
    instvar arg

    return $arg
}

Extimage method bend {} {
    instvar bend

    return $bend
}

class ExtimageViewer
ExtimageViewer inherit SmurfViewer

ExtimageViewer method init args {
    instvar zoom add_ext display_mode currentExt ext_list list_pos
    instvar base_name first_index last_index current_index index_step
    instvar current_line nbox
    instvar bgImage
    instvar histoImage
    instvar appQ
    instvar qValue

    next
    # Initialisation of the instance variables.
    set add_ext ""
    set display_mode "-2"
    set ext_list ""
    set zoom 1
    set list_pos 0
    set base_name ""
    set current_line ""
    set nbox 1024
    # Initialisation of the extimage list
    set current_index none

    set bgImage no
    set histoImage no
    set appQ no
    set qValue 1

    puts $self
    
    if {[lindex $args 0] == "-minmax"} {
	set base_name [lindex $args 1]
	set first_index [lindex $args 2]
	set last_index [lindex $args 3]
	set index_step [lindex $args 4]
	for {set i $first_index} {$i <= $last_index} {incr i $index_step} {
	    set ext [getnamefrom ${base_name}max$i]
	    lappend ext_list $ext
	    Extimage new $ext
	}
	set current_index $first_index
    } else {
	set ext_list $args
	foreach ext $args {
	    Extimage new $ext
	}
    }

    # 
    set currentExt [lindex $ext_list 0]

    eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom $display_mode
    $self.viewer content $self.viewer.content

    # Display
    frame $self.display.line1
    pack $self.display.line1 -side bottom

    #label $self.display.line1.mod_msg -text "m"
    label $self.display.line1.mod_val -fg green4 -relief sunken \
	    -bd 1 -width 13
    #pack  $self.display.line1.mod_msg $self.display.line1.mod_val \
	    #-side top -padx 1m -pady 1m
    pack $self.display.line1.mod_val \
	    -side top -padx 1m -pady 1m
    
    #label $self.display.line1.arg_msg -text "a"
    label $self.display.line1.arg_val -fg green4 -relief sunken \
	    -bd 1 -width 13
    #pack  $self.display.line1.arg_msg $self.display.line1.arg_val \
	    #-side top -padx 1m -pady 1m
    pack   $self.display.line1.arg_val \
	    -side top -padx 1m -pady 1m
    
    #label $self.display.line1.x_msg -text "x"
    label $self.display.line1.x_val -fg green4 -relief sunken \
	    -bd 1 -width 5 
    #pack  $self.display.line1.x_msg $self.display.line1.x_val \
	    #-side top -padx 1m -pady 1m
    pack   $self.display.line1.x_val \
	    -side top -padx 1m -pady 1m
    
    #label $self.display.line1.y_msg -text "y"
    label $self.display.line1.y_val -fg green4 -relief sunken \
	    -bd 1 -width 5
    #pack  $self.display.line1.y_msg $self.display.line1.y_val \
	    #-side top -padx 1m -pady 1m
    pack   $self.display.line1.y_val \
	    -side top -padx 1m -pady 1m

    # bend
    label $self.display.line1.bend_val -fg green4 -relief sunken \
	    -bd 1 -width 13
    pack   $self.display.line1.bend_val \
	    -side top -padx 1m -pady 1m

    label $self.display.line1.name -fg blue -text $currentExt
    pack  $self.display.line1.name -side top -padx 1m -pady 1m

    label $self.display.line_stats

    # Buttons and keys associated actions
    bind $self.viewer <Right> "$self next"
    bind $self.viewer <Left>  "$self prev"
    bind $self.viewer <1>     "$self view_module %x %y"
    bind $self.viewer <2>     "$self view_chains %x %y"
    bind $self.viewer <3>     "$self mouse_goto %x %y"
    bind $self.viewer <Up>    "$self next_ext"
    bind $self.viewer <Down>  "$self prev_ext"

    bind $self.viewer <Control-i>     "$self switch_bgImage"
    bind $self.viewer <Control-j>     "$self switch_histoImage"
    
    bind $self.viewer <Control-q>     "$self switch_apply_q"

    bind $self.viewer <Control-2>     "$self view_line %x %y"
    bind $self.viewer <Control-3>     "$self view_vert_chain %x %y"

    bind $self.viewer <Control-Right>  "$self zoom_incr"
    bind $self.viewer <Control-Left>   "$self zoom_decr"

    bind $self.viewer <Control-Up>    "$self next_minmax"
    bind $self.viewer <Control-Down>  "$self prev_minmax"
    bind $self.viewer <Control-a>     "$self current_minmax"

    #bind $self.viewer <a>     "$self view_add_ext"
    #
    bind $self.viewer <l>     "$self set_disp_mode Linear"
    bind $self.viewer <b>     "$self set_disp_mode Binary"
    bind $self.viewer <k>     "$self set_disp_mode Log"

    
    # all MMTO in YELLOW and the greatest in RED
    bind $self.viewer <g>     "$self set_disp_mode Gradient"
    
    #
    bind $self.viewer <m>     "$self set_disp_mode Mini"
    bind $self.viewer <M>     "$self set_disp_mode Mini_inv"
    
    # MMMTO     on vc : BLUE
    # MMMTO not on vc : YELLOW
    bind $self.viewer <a>     "$self set_disp_mode all_greater"
    
    # only the greatest MMMTO of each line is displayed in RED (formerly
    # in BLUE)
    bind $self.viewer <o>     "$self set_disp_mode greater"

    bind $self.viewer <h>     "$self display_histogram"
    bind $self.viewer <H>     "$self display_arg_histogram"
    bind $self.viewer <j>     "$self display_loghistogram"
    bind $self.viewer <x>     "$self display_mx_histogram No"
    bind $self.viewer <w>     "$self display_mx_histogram Log"
    bind $self.viewer <y>     "$self display_my_histogram No"
    bind $self.viewer <t>     "$self display_my_histogram Log"

    #bind $self.viewer <Control-h> "$self set_histogram_parameters"

    bind $self.viewer <Control-Delete>     "$self destroy_line"

    bind $self.viewer <Enter> "focus $self.viewer"

    focus $self
}

ExtimageViewer method destroy args {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$ext_list != ""} {
	foreach ext $ext_list {
	    $ext destroy
	}
    }
    next
}


# to switch backgroung of the window from black to image named "i"
#
# a changer, parce qu'on ne reevalue pas un econvert.
ExtimageViewer method switch_bgImage args {
    instvar bgImage

    if {$bgImage == "yes"} {
	set bgImage no
    } else {
	set bgImage yes
    }
}

ExtimageViewer method switch_histoImage args {
    instvar histoImage

    if {$histoImage == "yes"} {
	set histoImage no
    } else {
	set histoImage yes
    }
}

ExtimageViewer method set_q_value q {
    instvar qValue

    set qValue $q
}

ExtimageViewer method switch_apply_q args {
    instvar appQ

    if {$appQ == "yes"} {
	set appQ no
    } else {
	set appQ yes
    }
}

# obtention : <Control-3>
# action    : fenetre comprenant 3 graphes (mod le long la chaine, etc...))
# attention log base 2
ExtimageViewer method view_vert_chain {x y} {
    instvar currentExt
    instvar zoom
    instvar qValue

    set new_x [expr $x/$zoom]
    set new_y [expr $y/$zoom]

    set modSig vcm${currentExt}${new_x}${new_y}
    set argSig vca${currentExt}$x$y
    set txSig vcx${currentExt}$x$y
    set tySig vcy${currentExt}$x$y

    vc2s $currentExt $modSig $new_x $new_y -arg $argSig
    scomb $modSig $argSig log(fabs(x*cos(y)))/log(2) $txSig
    s2fs $txSig $txSig x/10 y
    scomb $modSig $argSig log(fabs(x*sin(y)))/log(2) $tySig
    s2fs $tySig $tySig x/10 y
    s2fs $modSig $modSig x/10 log(y)/log(2)
    #s2fs $modSig $modSig x/10 y
    s2fs $argSig $argSig x/10 y
    mdisp 1 3 [list $modSig [list $txSig $tySig] $argSig]
}

#
ExtimageViewer method view_line {x y} {
    instvar currentExt
    instvar zoom

    set new_x [expr $x/$zoom]
    set new_y [expr $y/$zoom]
    l2s $currentExt lm${currentExt}$x$y $new_x $new_y -arg la${currentExt}$x$y
    mdisp 1 2 "lm${currentExt}$x$y la${currentExt}$x$y"
}

ExtimageViewer method set_histogram_parameters {} {
    instvar nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line h$current_line $nbox
	saff h$current_line
    } else {
	ehisto $currentExt h$currentExt $nbox
	saff h$currentExt
    }
}

ExtimageViewer method display_histogram {} {
    instvar current_line currentExt nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line h$current_line $nbox
	saff h$current_line
    } else {
	ehisto $currentExt h$currentExt $nbox
	saff h$currentExt
    }
}

ExtimageViewer method display_mx_histogram {islog} {
    instvar current_line currentExt nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line hx$current_line $nbox
	saff hx$current_line
    } else {
	ehisto $currentExt hx$currentExt $nbox -mx
	if {$islog == "Log"} {
	    slogy hx$currentExt loghx$currentExt 10   
	    sdisp loghx$currentExt
	} else {
	    sdisp hx$currentExt
	}
    }
}

ExtimageViewer method display_my_histogram {islog} {
    instvar current_line currentExt nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line hy$current_line $nbox
	saff hy$current_line
    } else {
	ehisto $currentExt hy$currentExt $nbox -my
	if {$islog == "Log"} {
	    slogy hy$currentExt loghy$currentExt 10   
	    sdisp loghy$currentExt
	} else {
	    sdisp hy$currentExt
	}
    }
}

ExtimageViewer method display_loghistogram {} {
    instvar current_line currentExt nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line h$current_line $nbox
	slogy h$current_line logh$current_line 10
	sdisp logh$current_line
    } else {
	ehisto $currentExt h$currentExt $nbox
	slogy h$currentExt logh$currentExt 10
	sdisp logh$currentExt
    }
}

ExtimageViewer method display_arg_histogram {} {
    instvar current_line currentExt nbox

    if {$current_line != ""} {
	lhisto $currentExt $current_line ha$current_line $nbox -arg
	#saff ha$current_line
	sdisp ha$current_line
    } else {
	ehisto $currentExt ha$currentExt $nbox -arg
	#saff ha$currentExt
	sdisp ha$currentExt
    }
}

ExtimageViewer method next_minmax {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$base_name != ""} {
	$self next_ext
	vadd $self.viewer.content ${base_name}min$current_index -zoom $zoom
	$self.viewer content $self.viewer.content
    }
}

ExtimageViewer method prev_minmax {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$base_name != ""} {
	$self prev_ext
	vadd $self.viewer.content ${base_name}min$current_index -zoom $zoom
	$self.viewer content $self.viewer.content
    }
}

ExtimageViewer method current_minmax {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$base_name != ""} {
	vadd $self.viewer.content ${base_name}min$current_index -zoom $zoom
	$self.viewer content $self.viewer.content
    }
}

ExtimageViewer method view_add_ext {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$add_ext != ""} {
	vadd $self.viewer.content $add_ext -zoom $zoom
	$self.viewer content $self.viewer.content
    }
}

ExtimageViewer method set_add_ext {name} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set add_ext [getnamefrom $name]
}

ExtimageViewer method set_disp_mode {mode} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos current_line
    instvar bgImage
    instvar histoImage
    instvar appQ
    instvar qValue

    set current_line ""
    if {$mode == "Linear"} {
	set display_mode ""
    }
    if {$mode == "Log"} {
	set display_mode "-log"
    }
    if {$mode == "Binary"} {
	set display_mode "-2"
    }
    if {$mode == "Gradient"} {
	set display_mode "-2 -grad"
    }
    if {$mode == "Mini"} {
	set display_mode "-min 30"
    }
    if {$mode == "Mini_inv"} {
	set display_mode "-min_inv 70"
    }
    if {$mode == "greater"} {
	set display_mode "-g"
    }
    if {$mode == "all_greater"} {
	set display_mode "-ga"
    }

    # Aie aie aie....
    if {$bgImage == "yes"} {
	set display_mode [concat $display_mode {-bg i 2}]
    }
    if {$appQ == "yes"} {
	set display_mode [concat $display_mode [list -q $qValue]]
    }

    # (Aie)^12
    if {$bgImage == "yes" && $histoImage == "yes"} {
	set display_mode [concat $display_mode {-bg i 2 -histoimage 5000}]
    }
    

    eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom $display_mode
}

ExtimageViewer method goto_ext {ext} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set list_pos [lsearch -exact $ext_list $ext]
    set currentExt $ext
    eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom $display_mode
    $self display_val
}

ExtimageViewer method next_ext {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$list_pos < [expr [llength $ext_list]-1]} {
	incr list_pos
	set currentExt [lindex $ext_list $list_pos]
	eval econvert $currentExt $self.viewer.content \
		-zoom $zoom $display_mode
	$self display_val
    }
    if {$current_index != "none"} {
	if {$current_index < $last_index} {
	    incr current_index $index_step
	}
    }
}

ExtimageViewer method prev_ext {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if {$list_pos > 0} {
	incr list_pos -1
	set currentExt [lindex $ext_list $list_pos]
	eval econvert $currentExt $self.viewer.content \
		-zoom $zoom $display_mode
	$self display_val
    }
    if {$current_index != "none"} {
	if {$current_index > $first_index} {
	    incr current_index -$index_step
	}
    }
}

ExtimageViewer method view_module {pos_x pos_y} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set current_line ""
    eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom $display_mode
}

#
# Method to emphasize the current line in the the current ext_image
#
ExtimageViewer method view_chains {pos_x pos_y} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos
    instvar current_line

    set current_line [eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom  $display_mode -h $pos_x $pos_y ${currentExt}_line]
    if {$current_line != "" && $current_line != 0} {
	set string [eval linestats $current_line]

	$self.display.line_stats configure -text "$string" 
	pack $self.display.line_stats -side top
    } else {
	eval econvert $currentExt $self.viewer.content \
		-zoom $zoom  $display_mode
    }
}

#
# Method to remove the current line from the current ext_image.
#
ExtimageViewer method destroy_line {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos
    instvar current_line

    if {$current_line != ""} {
	rm_1line $current_line
	set current_line ""

	$self.display.line_stats configure -text "" 
	pack $self.display.line_stats -side top
	econvert $currentExt $self.viewer.content -zoom $zoom $display_mode
    }
}

ExtimageViewer method mouse_goto {pos_x pos_y} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set result [eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom -p $pos_x $pos_y]
    # -type $globstyle 
    if {$result != ""} {
	$currentExt set mod [lindex $result 0]
	$currentExt set arg [lindex $result 1]
	$currentExt set x [lindex $result 2]
	$currentExt set y [lindex $result 3]
	$currentExt set bend [lindex $result 4]
	$self display_val
    } else {
	eval econvert $currentExt $self.viewer.content \
	    -zoom $zoom $display_mode
    }
}

ExtimageViewer method next {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set result [econvert $currentExt $self.viewer.content \
	    -zoom $zoom -next [$currentExt x] [$currentExt y]]
    if {$result != "no chain"} {
	$currentExt set mod [lindex $result 0]
	$currentExt set arg [expr [lindex $result 1]*180/3.141592654]
	$currentExt set x [lindex $result 2]
	$currentExt set y [lindex $result 3]
	$currentExt set bend [lindex $result 4]
	$self display_val
    }
}

ExtimageViewer method prev {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    set result [econvert $currentExt $self.viewer.content \
	    -zoom $zoom -prev [$currentExt x] [$currentExt y]]
    if {$result != "no chain"} {
	$currentExt set mod [lindex $result 0]
	$currentExt set arg [expr [lindex $result 1]*180/3.141592654]
	$currentExt set x [lindex $result 2]
	$currentExt set y [lindex $result 3]
	$currentExt set bend [lindex $result 4]
	$self display_val
    }
}

ExtimageViewer method display_val {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    $self.display.line1.x_val    configure -text "x [$currentExt x]" 
    $self.display.line1.y_val    configure -text "y [$currentExt y]" 
    $self.display.line1.mod_val  configure -text "m [$currentExt mod]" 
    $self.display.line1.arg_val  configure -text "a [$currentExt arg]" 
    $self.display.line1.bend_val configure -text "bend [$currentExt bend]" 
    $self.display.line1.name     configure -text $currentExt
}

ExtimageViewer method zoom_incr {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if { [set zoom] < 6 } {
	incr zoom 
	eval econvert $currentExt $self.viewer.content \
		-zoom $zoom $display_mode
    }
}

ExtimageViewer method zoom_decr {} {
    instvar base_name first_index last_index current_index index_step
    instvar zoom add_ext display_mode currentExt ext_list list_pos

    if { [set zoom] > 1 } {
	incr zoom -1
	eval econvert $currentExt $self.viewer.content \
		-zoom $zoom $display_mode
    }
}

ExtimageViewer method add_display {name script} {
    set string [eval $script]
    label $self.display.${name} -text "$string"
    pack $self.display.${name} -side bottom
}

ExtimageViewer method remove_display {name} {
    destroy $self.display.${name}
}

ExtimageViewer method refresh_display {name script} {
    set string [eval $script]
    $self.display.${name} configure -text "$string" 
}

#
# Set nbox to a new value.
#
ExtimageViewer method set_nbox {value} {
    instvar nbox

    set nbox $value
}


# eaff --
# usage : eaff extima args
#
#  Display an ext_image.
#
# Parameters :
#   ext_image  - 
#                
#   args       - args, options...see help message of econvert define by
#                ViewConvExtImageCmd_ in widgets/ConvExt.c !!!
#
# soon there will be a real help message for this.
#
# Return value :
#   The name of the object that manage this ViewImage window.
#
# Example :
#   eaff ext_ima
#   This command line open a window that displays ext_image "ext_ima"
# 
#   If you want to have an image in the background (instead of uniform
#   black), just change the name of your image to "i" and do <Ctrl-i>
#   maybe you'll have to type "l" just after

proc eaff args {
    global viewNb

    if {[lindex $args 0] == "-minmax"} {
	if {[lindex $args 2] > [lindex $args 3]} {
	    puts "aaaaaarg"
	    return
	}
	set ext_list $args
    } else {
	set ext_list {}
	foreach ext $args {
	    set ext_list [concat $ext_list [getnamefrom $ext]]
	}
	set ext_list [lsort $ext_list]
    }
    if {$ext_list == {}} {
	puts hein??
    } else {
	toplevel .v${viewNb}
	eval "ExtimageViewer .v${viewNb}.ev  $ext_list"
	pack .v${viewNb}.ev
	bind .v${viewNb} <c> "destroy .v${viewNb}"
	incr viewNb
    }
}
