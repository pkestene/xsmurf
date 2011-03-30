#----------------------------------------------------------------------
# class Tabs - A.k.a notebook widgets
#----------------------------------------------------------------------

DOC {class Tab (Tk) (Notebook widget)} {
 NAME
	Tab -	A class for tab-objects (a.k.a Noteboook-widgets)
		A tab-object has one or more 3-D looking tabs sticking up
		from a frame.  Clicking on a tab may invoke an action,
		which may in turn change the appearance of the tab row
		(i.e. bringing a new tab to the front).

 DESCRIPTION
	Tab <name>
	Tab new <name>
		Create a tab-object of name <name>.

	<name> destroy
		Destroy a tab-object and free all resources associated
		with it.

 METHODS
	Available methods are:

	    High level:

		manage
		unmanage

		topMargin
		borderwidth
		background
		tab_background

	    Low level:

		createTab		
		createFrameTab
		listTabs
		setAction
		raise
		getRaised
		destroyTab
		manageBody

	A description for each of the methods follows -

	High-level
	----------

	<name> manage {<text> <win> ?pack options?} ..

		Manage window <win> in a tab labeled <text>.  The tab is
		automatically created.

	<name> unmanage <win>

		Remove <win> from being managed in <name>.  The tab
		is automatically removed.

	<name> topMargin <N>
		Specifies a vertical distance from the tabs to the
		contents of the tab-object's body.  Default is 10 pixels.

	<name> borderwidth <N>
		Set the borderwidth of the tabs and the body to <N>

	<name> background <color>
		Set the color of the tabs themself and the body frame
		(not including any window managed in the body).

	<name> tab_background <color>
		Set the color for the area behind the tabs to <color>

	Low-level
	---------

	<name> createTab <text> ?action?
		Add a tab to the row of tabs.  Each tab holds a flat button.
		The return value of the call is the name of the newly
		created button (<tab>).

		If specified, `action' is invoked when the mouse is clicked
		on the tab.  Action may contain %W which will be expanded
		to the name of the tab frame.
		See ACTIONS below.

	<name> createFrameTab <text> ?action?
		Add a tab of the type frame.  Anything can be packed
		in the frame (for example a label holding an image).
		The return value of the call is the name of the newly
		created frame (<tab>).

		If specified, `action' is invoked when the mouse is clicked
		on the tab.  Action may contain %W which will be expanded
		to the name of the tab frame.
		See ACTIONS below.

	<name> listTabs
		Returns a list all existing tabs ordered from the left to
		the right.

	<name> setAction <tab> <action>
		Arrange for script <action> to be evaluated when the mouse is
		clicked on the tab, or when focus is on tab and user presses
		the spacebar. `tab' must be the value returned
		from one of `createTab' or `createFrameTab'.
		See ACTIONS below.

	<name> raise <tab>
		Draw tabs so the specified tab appears to be on top.

	<name> getRaised
		Returns name of currently raised tab.

	<name> destroyTab <tab>
		Destroys the <tab> and any children of <tab>.
		Note: a window managed in a tab does not have to
		be a child of the tab -in which case it will not be
		destroyed, only unmapped.

	<name> manageBody <win> ?pack options?
		Make <win> the currently managed widget. <win> is packed
		into the tab-objects body.  Any other previously managed
		widget is automatically unpacked.  This method is normally
		used by tab-actions (see ACTIONS below) to alter what is
		displayed under the tabs.

 ACTIONS
	A typical action to be invoked when the user clicks
	on a tab is to check that the currently displayed page is
	valid (input values are OK, etc), use the `raise' method to
	bring the wanted tab to the top, and alter the contents
	of the body of the tab-widget, using the `manageBody' method.

	The symbol %W can be used in the action script.  It will be expanded
	to the pathname of the invoking tab (useful for example to raise
	the tab).  No other %-symbols are supported.
}

class Tab
Tab inherit Widget

#---------------------------------------------------------------------\
# Tab drawing utilities
#

# Mimic highlight/shadow calculations in tk3d.c
# Mimics calcs in tk4.0/tk3d.c (sorry i don't handle monochrome etc.)
#
# (It would be great if the shadow-colours in Tk could be set as well
# as inspected!)
#
proc doSunny c {
	set tmp [expr 14 * $c / 10]
	set tmp [expr $tmp > 65535 ? 65535 : $tmp]
	set tmp2 [expr (65535 + $c)/2]
	return [expr $tmp > $tmp2 ? $tmp : $tmp2]
}
proc GetBorderColors { r g b lR lG lB dR dG dB } {
	upvar $lR LR $lG LG $lB LB $dR DR $dG DG $dB DB
	set LR [doSunny $r]
	set LG [doSunny $g]
	set LB [doSunny $b]
	set DR [expr (60 * $r) / 100]
	set DG [expr (60 * $g) / 100]
	set DB [expr (60 * $b) / 100]
}

#----------------------------------------------------------------------
# Tabs: Measures in polygons:
#
# h1	Total height of tab (including border)
# w1	Total tab width (including border)
# th	Thickness of border
# th2	2x border thicknes (top corner width and heights)

# A one pixel gap is left between part1 and part2 to let backg
# shine through (gives smoother corner).

# Left side and top of tab
set part1 {\
	0 $h1 \
	0 $th2 \
	$th2 0 \
	[expr $w1-$th2-3] 0 \
	[expr $w1-$th2-3] $th \
	[expr $th+$th2-$th/2] $th \
	$th [expr $th2+$th-$th/2] \
	$th $h1 }

# Right side of tab
set part2 {\
	[expr $w1-$th2-2] 0 \
	[expr $w1-$th2] 0 \
	[expr $w1] $th2 \
	$w1 $h1 \
	[expr $w1-$th] $h1 \
	[expr $w1-$th] [expr $th2+$th-$th/2] \
	[expr $w1-$th2-$th/2] $th \
	[expr $w1-$th2-2] $th}

# Background
set backg {\
	0 $h1 \
	0 $th2 \
	$th2 0 \
	[expr $w1-$th2] 0 \
	[expr $w1] $th2 \
	$w1 $h1 }
	
set monochrome 0
if { [winfo depth .] == 1 } {
	set monochrome 1
}

# c	  Canvas to draw in
# x y	  Pos of lower left corner
# bwidth  Border width
# height  Tab height
#
proc DrawTab { c b x bwidth width heigth bgcol lcol dcol } {
	global part1 part2 backg monochrome
	set th $bwidth
	if { $th > 2 } {
		set th2 [expr 2.5*sqrt($th)]
	} else {
		# set th2 [expr 1.5*$th]
		set th2 3
	}
	set w1 $width
	set h1 $heigth

	if $monochrome {
		set ID1 [eval $c create polygon $part1 -fill black -stipple gray50 -tag $b]
		$c move $ID1 $x 0

		set ID2 [eval $c create polygon $part2 -fill black -tag $b]
		$c move $ID2 $x 0

		set ID3 [eval $c create polygon $backg -fill white -tag $b]
		$c move $ID3 $x 0
	} else {
		set ID1 [eval $c create polygon $part1 -fill $lcol -tag $b]
		$c move $ID1 $x 0

		set ID2 [eval $c create polygon $part2 -fill $dcol -tag $b]
		$c move $ID2 $x 0

		set ID3 [eval $c create polygon $backg -fill $bgcol -tag $b]
		$c move $ID3 $x 0
	}

	$c lower $ID2
	$c lower $ID1
	$c lower $ID3
}

#----------------------------------------------------------------------
# Top-lines connected to either sides of tab
# The measures used in these polygons are:
#
# x1	Left side of tab to connect to
# x2	Right side of tab to connect to
# w1	Tab width (including borders)
# th2	2x border thickness
# maxx	Canvas max X-coord (so we can draw exactlty to the right hand side)

set line1 {\
	0 0 \
	$x1 0 \
	[expr $x1+$th] $th \
	0 $th }
set line2 {\
	$x2 0 \
	$maxx 0 \
	[expr $maxx-$th] $th \
	[expr $x2-$th] $th }
set line3 {\
	$maxx 0 \
	$maxx $th \
	[expr $maxx-$th] $th }

Tab method drawTop {} {
	global line1 line2 line3 monochrome
	instvar currB bwidth hpad lightC darkC

	set c $self.bc
	set b $currB

	if ![string compare "" $b] { return }

	set th $bwidth

	set x [winfo x $b]
	set y [winfo y $b]
	set width [winfo width $b]
	set height [winfo reqheight $b]

	set x1 [expr $x-$hpad-$th]
	set x2 [expr $x+$width+$hpad+$th]

	set maxx [winfo width $c]
	set w1 [expr $width+2*$th+2*$hpad]

	foreach i [$c find withtag TOP] {
		$c delete $i
	}

	if $monochrome {
		set ID1 [eval $c create polygon $line1 -fill black -stipple gray50 -tags TOP ]
		set ID2 [eval $c create polygon $line2 -fill black -stipple gray50 -tags TOP ]
		set ID3 [eval $c create polygon $line3 -fill black -tags TOP ]
	} else {
		set ID1 [eval $c create polygon $line1 -fill $lightC -tags TOP ]
		set ID2 [eval $c create polygon $line2 -fill $lightC -tags TOP ]
		set ID3 [eval $c create polygon $line3 -fill $darkC -tags TOP ]
	}
	set y [expr [winfo reqheight $c]-$bwidth]
	$c move $ID1 0 $y
	$c move $ID2 0 $y
	$c move $ID3 0 $y

	$c raise $ID1
	$c raise $ID2
}

Tab method setTabAreaHeight {} {
	instvar bwidth maxh bwidth vpad
	set maxh 0
	foreach b [pack slaves $self.bc] {
		set max [winfo reqheight $b]
		if { $max > $maxh } {
			set maxh $max
		}
	}
	$self.offs configure -height [expr $maxh+$bwidth+2*$vpad]
	$self.bc configure -height [expr $maxh+2*$bwidth+2*$vpad]
}
Tab method placeCanvas {} {
	instvar bwidth
	place $self.bc \
		-anchor sw \
		-relx 0 -rely 0 \
		-bordermode inside \
		-x -$bwidth \
		-y 0 \
		-in $self.body
}
Tab method drawTab b {
	instvar maxh bwidth hpad vpad sep lightC darkC background
	set x [winfo x $b]; set y [winfo y $b]
	set width [winfo width $b]
	set height [winfo reqheight $b]
	DrawTab $self.bc $b \
		[expr $x-$hpad-$bwidth-$sep] \
		$bwidth \
		[expr $width+2*$hpad+2*$bwidth+$sep] \
		[expr $maxh+2*$bwidth+2*$vpad] \
		$background $lightC $darkC
	foreach i [$self.bc find withtag $b] {
		$self.bc bind $i <Button-1> \
			"$self triggerAction $b"
	}
}
Tab method drawAllTabs {} {
	instvar maxh bwidth hpad vpad sep

	eval $self.bc delete [$self.bc find all]
	$self setTabAreaHeight
	update
	foreach b [pack slaves $self.bc] {
		$self drawTab $b
	}
}

# End of tab drawing utilities
#---------------------------------------------------------------------/


Tab method mkColors c {
	instvar lightC darkC

	eval GetBorderColors [winfo rgb $self $c] lR lG lB dR dG dB
	set lightC [format "#%03x%03x%03x" $lR $lG $lB]
	set darkC [format "#%03x%03x%03x" $dR $dG $dB]
}

Tab option {-background} [option get . background Frame] configure {
	$self background $background
} init {
	$self background $background
}

Tab option {-tab_background} [option get . background Frame] configure {
	$self-cmd configure -background $tab_background
	$self.bc configure -background $tab_background
} init {
	$self-cmd configure -background $tab_background
	$self.bc configure -background $tab_background
}

Tab method init args {
	instvar maxh currB bwidth hpad vpad sep nextID bodyW \
		lightC darkC background tab_background

	next

	set bodyW {}
	set nextID 0
	set maxh 0
	set bwidth 2
	set currB {}
	set hpad 2
	set vpad 1
	set sep 1

	eval $self conf_verify $args
	$self mkColors $background

	frame $self.offs -width 1 -background $background
	frame $self.body -height 200 -background $background \
		-borderwidth $bwidth -relief raised
	frame $self.body.topmarg -height 10 -width 1 -borderwidth 0 \
		-highlightthickness 0 -background $background

	canvas $self.bc -highlightthickness 0 \
		-borderwidth 0 -background $tab_background

	$self setTabAreaHeight

	bind $self.body <Configure> "
		set w \[winfo width %W\]
		$self.bc configure -width \$w
		update
		$self drawTop"

	# We mustn't let the canvas shrink-wrap, since we use its width
	# to draw the top border of the body.
	pack propagate $self.bc false

	$self placeCanvas

	pack $self.offs -side top -in $self
	pack $self.body -side top -expand yes -fill both -in $self \
		-padx 0
	pack $self.body.topmarg -side top

	eval $self conf_init $args

	return $self
}
Tab method adjGeom b {
	instvar maxh
	set height [winfo reqheight $b]
	if { $height > $maxh } {
		set maxh $height
	}
}

#----------------------------------------------------------------------
# Methods for altering appearance:

Tab method topMargin y {
	$self.body.topmarg configure -height $y
}
Tab method borderwidth w {
	instvar bwidth hpad vpad sep maxh
	set bwidth $w
	set hpad $w
	foreach b [pack slaves $self.bc] {
		pack configure $b -padx [expr 2*$hpad+$sep] -pady $bwidth
	}
	$self placeCanvas

	$self.body configure -borderwidth $w
	$self setTabAreaHeight
	$self drawAllTabs
	$self drawTop
}
Tab method tab_background c {
	instvar bwidth hpad vpad sep maxh
	$self-cmd configure -background $c
	$self.bc configure -background $c
}
Tab method background c {
	instvar bwidth hpad vpad sep maxh \
		lightC darkC
	$self.body configure -background $c
	$self.body.topmarg configure -background $c
	foreach b [pack slaves $self.bc] {
		$b configure -background $c -highlightbackground $c
	}
	set c [$self.body cget -background]
	eval GetBorderColors [winfo rgb $self.body $c] lR lG lB dR dG dB
	set lightC [format "#%03x%03x%03x" $lR $lG $lB]
	set darkC [format "#%03x%03x%03x" $dR $dG $dB]
	$self drawAllTabs
	$self drawTop
}
Tab method raise b {
	instvar currB
	set currB $b
	$self drawTop
}
Tab method getRaised {} {
	instvar currB
	return $currB
}
#----------------------------------------------------------------------

Tab method manageBody { w args } {
	instvar bodyW
	if [string compare "" $bodyW] {
		pack forget $bodyW
	}
	eval pack $w -in $self.body -expand yes -fill both $args
	set bodyW $w
}
Tab method unmanageBody b {
	pack forget $b
}
Tab method createFrameTab { {act {}} } {
	instvar nextID action
	set tmp $nextID
	incr nextID
	frame $self.bc.f$tmp -borderwidth 0 -highlightthickness 0

	if [string compare "" $act] {
		regsub "%W" $act $self.bc.f$tmp aa
		set action($self.bc.f$tmp) $aa
	}
	$self manageTab $self.bc.f$tmp
	return $self.bc.f$tmp
}
Tab method createTab { txt { act {} } } {
	instvar nextID action background
	set tmp $nextID
	incr nextID
	button $self.bc.b$tmp -text $txt -wraplength 200 \
		-relief flat -borderwidth 0 \
		-padx 5 -pady 1 \
		-command "$self triggerAction $self.bc.b$tmp" \
		-background $background \
		-highlightbackground $background

	if [string compare "" $act] {
		regsub -all "%W" $act $self.bc.b$tmp aa
		set action($self.bc.b$tmp) $aa
	}

	$self manageTab $self.bc.b$tmp
	return $self.bc.b$tmp
}
Tab method destroyTab b {
	instvar currB
	catch {destroy $b}
	if ![string compare $currB $b] {
		set w [lindex [winfo children $self.bc] 0]
		if [string compare "" $w] { $w invoke }
		set currB $w
	}
	$self drawAllTabs
	$self drawTop
}
Tab method listTabs {} {
	return [pack slaves $self.bc]
}

Tab method triggerAction b {
	instvar action
	if [info exists action($b)] {
		uplevel #0 "eval [set action($b)]"
	}
}
Tab method OLDsetAction { b act } {
	instvar action
	set action($b) $act
}
Tab method setAction { b act } {
	instvar action
	if [string compare "" $act] {
		regsub -all "%W" $act $b aa
	}
	set action($b) $aa
}
Tab method getAction b {
	instvar action
	set action($b)
}
Tab method manageTab b {
	instvar maxh currB bwidth hpad vpad sep

	pack $b -side left -anchor c \
		-ipadx 0 -ipady 0 -padx [expr 2*$hpad+$sep] \
		-pady $bwidth -in $self.bc \
		-expand no -fill y	

	update
	set height [winfo reqheight $b]

	if { $height > $maxh } {
		set maxh $height
		$self drawAllTabs
	} else {
		$self drawTab $b
	}
	bind $b <Configure> "\
		$self adjGeom $b
		$self drawAllTabs
		$self drawTop
	"
	bind $b <Button-1> "\
		$self triggerAction $b
		break
	"
	if ![string compare "" $currB] {
# Done some other place...
#		set currB $b
#		$self drawTop
	}
}
Tab method unmanageTab b {
	pack forget $b
}

#----------------------------------------------------------------------
# Convenience methods:
# manage - Add a number of tabs to a tab object.
#
#	manage {Title .win} ?{Title .win}?..
#
# Args contains pairs of {text widget} where `text' will
# be displayed in the tab, and `widget' is a window which will
# be packed into the tab object's body whenever the tab is pressed.
# Also ensure that the first tab gets its widget packed.
#
Tab method manage args {
	instvar win2tab currB

	# For each element in args, create a tab and arrange for packing
	# of the associated window when the tab is pressed.
	#
	foreach i $args {
		set txt [lindex $i 0]
		set w [lindex $i 1]
		set opts [lindex $i 2]
		if [info exists win2tab($w)] {
			$self unmanage $w
		}
		set b [$self createTab $txt \
			"eval {condManage $self %W} $w $opts"]
		set win2tab($w) $b
		raise $w $self.body
	}

	# Make sure something is displayed
	#
	if ![string compare "" $currB] {
		set tmp [lindex $args 0]
		if [string compare "" $tmp] {
			$self triggerAction $win2tab([lindex $tmp 1])
		}
	}
	return $b
}
Tab method manageAtButton { b w { opts ""} } {
	instvar win2tab currB

	if ![winfo exists $b] {
		error "manageAtButton: Tab $b does not exist!"
	}
	if [info exists win2tab($w)] {
		$self unmanage $w
	}
	set win2tab($w) $b

	raise $w $self.body
	#$self raise $b

	$self setAction $b "eval {condManage $self %W} $w $opts"

	# Make sure something is displayed
	#
	#if ![string compare "" $currB] {
	#	$self triggerAction $b
	#}
	return $b
}
Tab method unmanage args {
	instvar win2tab currB
	foreach i $args {
		$self destroyTab $win2tab($i)
		pack forget $i
		unset win2tab($i)
	}
}
Tab method button args {
    instvar win2tab
    return $win2tab($args)
}

# condManage -	Raise tab and change contents of the tab-body if not
#		already displayed.
#
proc condManage { obj tab body args } {
	if { [string compare $tab [$obj getRaised]] || \
			[lsearch [pack slaves $obj.body] $body] == -1 } {
		eval {$obj manageBody $body} $args
		$obj raise $tab
	}
}

#----------------------------------------------------------------------
# End of the Tab-class definitions
#----------------------------------------------------------------------

