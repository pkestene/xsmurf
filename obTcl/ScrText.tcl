#----------------------------------------------------------------------
# class ScrolledText
#----------------------------------------------------------------------

DOC "class ScrolledText (Tk)" {
 NAME
	ScrolledText
		- A text widget with vertical and horizontal scrollbars.

 SYNOPSIS
	To create a new widget:

		ScrolledText new <obj> <options>
		ScrolledText .<obj> <options>

	The second line illustrates how the `new' method is used by
	default for objects with names starting with a full stop.

 DESCRIPTION
	An object of this class is created.  <options> can be set
	at creation time, or later by using the `configure' method.
	
 METHODS
	insert <args>
	  Calls normal text `insert' command with its arguments.
	  Handles write protection to always allow insert -even
	  when user cannot write in the window.

	configure <options>
	  For available options see OPTIONS.

	unknown <args>
	  Any method which is not defined in the ScrolledText class
	  is directed to the text widget.  See the Tk manual page on
	  `text' for available widget commands.

 OPTIONS
	-useTextWidget <name>
		Use existing text widget <name> instead of creating
		a new text widget.  If this option is used, -widgettype
		has no effect.

	-widgettype <type>
		Specify another widget type than the Tk `text' widget.
		This option must be specified during creation (new).
		It will have no effect if used later.

	-scrollbars <list>
		Where list is one of:
		  x	- Widget will have horizontal scrollbar
		  y	- Widget will have vertical scrollbar
		  {x y}	- Widget will have both scrollbars
		  {}	- Widget will have no scrollbars
		  none	- Widget will have no scrollbars
		Default is {x y}
			
	-writable <boolean>
		true	- User is allowed to write in window
		false	- No writing is allowed (`insert' works though)
		Deafult is "true".

	-lmarg <pixels>
	-rmarg <pixels>
		Specifies a margin between the text and the sunken border.
		Defaults are: 3 for left margin, 0 for right margin.

	-textrelief [flat|sunken]
		Affects appearance of text-area.  Other layout details are
		also adjusted for a nice appearance.

 EXAMPLE
	Create a widget which is not writable by user, and
	load "ScrText.tcl" into it:

		ScrolledText .txt -textrelief flat -writable false
		.txt insert end [exec cat "ScrText.tcl"]
		pack .txt

 BUGS
	More options for setting various things are needed,
	as well as for retreiving settings.
}

class ScrolledText
ScrolledText inherit Widget


ScrolledText method init args {
	instvar Writable Xbar Ybar lmarg rmarg \
		textbwidth textrelief cornerwidth \
		sbRmarg sbBmarg autoy width height wrap \
		widgettype useTextWidget self_t
		
	global PackCmd
	catch {unset PackCmd}

	set textbwidth 2
	set cornerwidth 20
	set sbRmarg 3
	set sbBmarg 3
	set self_t $self.t

	# The Widget class creates our top-frame
	#
	eval {$class..conf_verify} $args

	next

	frame $self.tMargs -highlightthickness 0 \
		-borderwidth $textbwidth -relief $textrelief
	frame $self.tLmarg -width $lmarg -highlightthickness 0
	frame $self.tRmarg -width $rmarg -highlightthickness 0

	if [string compare "" $useTextWidget] {
		set self_t $useTextWidget
	} else {
		$widgettype $self.t
	}
	$self_t configure -highlightthickness 1 \
			-wrap none -state normal \
			-relief flat -borderwidth 0 \
			-xscrollcommand "$self.sbh set" \
			-yscrollcommand "autoBars $self $self.sbv set" \
			-width $width -height $height \
			-highlightbackground [$self_t cget -background]

	frame $self.sbRmarg -width $sbRmarg -height 1 -highlightthickness 0
	scrollbar $self.sbv -width 12 -highlightthickness 1 \
		-command "$self_t yview"

	frame $self.hbar
	frame $self.corner -width $cornerwidth
	frame $self.sbBmarg -height $sbBmarg -width 1 -highlightthickness 0
	scrollbar $self.sbh -orient horizontal -width 10 \
		-highlightthickness 1 -command "$self_t xview"

	if ![string compare "" [pack slaves $self]] {
		$class..packAll
	}

	eval {$class..conf_init} $args
}

ScrolledText method insert args {
	instvar self_t
	set tmp [$self_t cget -state]
	$self_t configure -state normal
	eval {$self_t insert} $args
	$self_t configure -state $tmp
}
ScrolledText method delete args {
	instvar self_t
	set tmp [$self_t cget -state]
	$self_t configure -state normal
	eval {$self_t delete} $args
	$self_t configure -state $tmp
}

ScrolledText method configure_unknown args {
	instvar self_t
	eval $self_t configure $args
}

ScrolledText method init_unknown args {
	instvar self_t
	eval $self_t configure $args
}

ScrolledText method cget_unknown opt {
	instvar self_t
	$self_t cget $opt
}

ScrolledText method unknown args {
	instvar self_t
	eval {$self_t $method} $args
}

ScrolledText method unpackVsb w {
	global PackCmd
	instvar self_t autoy

	if {$autoy} {
		set PackCmd($w) \
		   "pack $self.sbRmarg -fill y -side right -before $self.tMargs
		    pack $self.sbv -side right -fill y -before $self.sbRmarg"
		pack forget $self.sbRmarg
		pack forget $self.sbv
		return 1
	}
	return 0
}

proc autoBars { self w cmd top bot } {
	global PackCmd
	if {$top == 0 && $bot == 1} {
		if ![$self unpackVsb $w] {
			eval $w $cmd $top $bot
		}
	} else {
		if [info exists PackCmd($w)] {
			eval $PackCmd($w)
			unset PackCmd($w)
		}
		eval $w $cmd $top $bot
	}
}

ScrolledText method packAll {} {
	instvar Xbar Ybar self_t

	pack forget $self_t $self.hbar $self.sbv $self.sbRmarg
	pack $self.sbBmarg -side top -in $self.hbar
	pack $self.corner -side right -in $self.hbar
	pack $self.sbh -side left -in $self.hbar -fill x -expand yes
	if { $Xbar } {
		pack $self.hbar -side bottom -fill x
	}
	if { $Ybar } {
		pack $self.sbv -side right -fill y
		pack $self.sbRmarg -side right -fill y
	}
	pack $self.tLmarg -side left -in $self.tMargs -fill y
	pack $self.tRmarg -side right -in $self.tMargs -fill y
	pack $self_t -side top -fill both -expand yes -in $self.tMargs
	pack $self.tMargs -in $self -fill both -expand yes
}

# Option handlers:

ScrolledText option {-useTextWidget} {} configure {
	instvar self_t
	if [info exists $self_t] {
		error "$self: \"-useTextWidget $useTextWidget\" can\
			only be used at creation time"
	}
}
ScrolledText option {-widgettype} {text} configure {
	instvar self_t
	if [info exists $self_t] {
		error "$self: \"-widgettype $widgettype\" can\
			only be used at creation time"
	}
}

ScrolledText option {-width} 100 configure {
    ${self}-cmd configure -width $width
}

ScrolledText option {-height} 100 configure {
    ${self}-cmd configure -height $height
}

ScrolledText option {-background} 0 configure {
	instvar self_t
	$self.tLmarg configure -background $background
	$self.tRmarg configure -background $background
	$self.tMargs configure -background $background
	$self.sbRmarg configure -background $background
	$self.sbBmarg configure -background $background
	$self.sbv configure -highlightbackground $background
	$self.sbh configure -highlightbackground $background
	$self_t configure -background $background
	$self_t configure -highlightbackground $background
} init {
	instvar self_t
	$self.tLmarg configure -background $background
	$self.tRmarg configure -background $background
	$self.tMargs configure -background $background
	$self.sbRmarg configure -background $background
	$self.sbBmarg configure -background $background
	$self.sbv configure -highlightbackground $background
	$self.sbh configure -highlightbackground $background
	$self_t configure -background $background
	$self_t configure -highlightbackground $background
}
ScrolledText option {-autoy} 0 verify {
	if {$autoy} {
		set autoy 1
	} else {
		set autoy 0
	}
} configure {
	if {$autoy} {
		set autoy 1
	} else {
		set autoy 0
	}
}

ScrolledText option {-textrelief} "sunken" verify {
	instvar textbwidth cornerwidth \
		sbRmarg sbBmarg

	if ![string compare "flat" $textrelief] {
		set textbwidth 0
		set cornerwidth 16
		set sbRmarg 1
		set sbBmarg 1
	} elseif ![string compare "sunken" $textrelief] {
		set textbwidth 2
		set cornerwidth 20
		set sbRmarg 3
		set sbBmarg 3
	} else {
		puts stderr "$self: invalid option value: -textrelief $textrelief"
		break
	}
} configure {
	instvar textbwidth cornerwidth \
		sbRmarg sbBmarg

	$self.tMargs configure -relief $textrelief \
		-borderwidth $textbwidth
	$self.corner configure -width $cornerwidth
	$self.sbRmarg configure -width $sbRmarg
	$self.sbBmarg configure -width $sbBmarg
}

ScrolledText option {-lmarg} 3 configure {
	$self.tLmarg configure -width $lmarg
}

ScrolledText option {-rmarg} 1 configure {
	$self.tRmarg configure -width $rmarg
}

ScrolledText option {-Xbar} 1 configure {}
ScrolledText option {-Ybar} 1 configure {}
ScrolledText option {-scrollbars} {x y} verify {
	instvar Xbar Ybar
	if {[lsearch $scrollbars "x"] != -1} {
		set Xbar 1
	} else {
		set Xbar 0
	}
	if {[lsearch $scrollbars "y"] != -1} {
		set Ybar 1
	} else {
		set Ybar 0
	}
} configure {
	$self packAll
}

ScrolledText option {-state} {enabled} verify {
	instvar Writable
	if [string compare "disabled" $state] {
		set Writable 1
	} else {
		set Writable 0
	}
} configure {
	instvar self_t
	$self_t configure -state $state
} init {
	instvar self_t
	$self_t configure -state $state
}

ScrolledText option {-writable} {enabled} verify {
	instvar Writable
	if { $writable } {
		set Writable 1
	} else {
		set Writable 0
	}
	if { $Writable == 1 } {
		set tmp "normal"
	} else {
		set tmp "disabled"
	}
} configure {
	instvar Writable self_t
	if { $Writable == 1 } {
		set tmp "normal"
	} else {
		set tmp "disabled"
	}
	$self_t configure -state $tmp
} init {
	instvar Writable self_t
	if { $Writable == 1 } {
		set tmp "normal"
	} else {
		set tmp "disabled"
	}
	$self_t configure -state $tmp
}
