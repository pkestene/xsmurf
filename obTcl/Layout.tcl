
class Layout
Layout inherit Widget

#----------------------------------------------------------------------
# Layout -
#	An experimental extension to `pack'.  Allows unnamed structures,
#	symbolic "places" to put your widgets, and hiding/unhiding
#	branches or leafs of the widget tree.
#	Widget layout is described using a indented style to allow easy
#	maintenance.  See the file "wrap" for example usage.
#
#  Syntax
#	Layout .m -define <spec>
#
#	<spec>:
#		{ id [x|y] "options" ?spec? ?spec?.. }
#
#  `id' can be either a symbolic name, or "*" to indicate that the frame is
#  just for helping layout.  x|y is packing direction of _children_.
#  `options' are used when creating and packing the frame.
#  Some shorthand options exist (see below for list).
#
#  Example
#	See the file `wrap' for example usage.
#
#  List of short options:
#
#	Symbol		Means
#	------		-----------------------
#	_		-relief sunken
#	^		-relief raised
#	u		-relief groove
#	n		-relief ridge
#	|		-relief flat	(the default)
#
#	fill		-fill both	(default is none)
#	exp		-expand yes	(default is no)
#	w<N>		-width <N>
#	h<N>		-height <N>
#	px<N>		-padx <N>
#	py<N>		-pady <N>
#	ix<N>		-ipadx <N>
#	iy<N>		-ipady <N>
#
#	-<opt> <value>	Used as is, example "-background red"
#
#----------------------------------------------------------------------

proc getMgrOpts { opts crOpts packOpts } {
	upvar $crOpts ret $packOpts pad
	set ret "-highlightthickness 0"
	set pad ""
	set appNext ""

	foreach i $opts {
		if [string compare "" $appNext] {
			append $appNext $i
			set appNext ""
			continue
		}
		regexp "\[^0-9\]*(\[0-9\]*)" "$i" garb val
		switch -glob -- $i {
		u*	{ append ret " -relief groove -borderwidth $val" }
		n*	{ append ret " -relief ridge -borderwidth $val" }
		_*	{ append ret " -relief sunken -borderwidth $val" }
		s*	{ append ret " -relief sunken -borderwidth $val" }
		^*	{ append ret " -relief raised -borderwidth $val" }
		r*	{ append ret " -relief raised -borderwidth $val" }
		|*	{ append ret " -relief flat -borderwidth $val" }
		exp*	{ append pad " -expand yes" }
		fil*	{ append pad " -fill both" }
		-anch*	{ append pad " $i "; set appNext pad }
		-exp*	{ append pad " $i "; set appNext pad }
		-fill*	{ append pad " $i "; set appNext pad }
		-pad*	{ append pad " $i "; set appNext pad }
		-ipad*	{ append pad " $i "; set appNext pad }
		-side	{ append pad " $i "; set appNext pad }
		-*	{ append ret " $i "; set appNext ret }
		w*	{ append ret " -width $val" }
		h*	{ append ret " -height $val" }
		px*	{ append pad " -padx $val" }
		py*	{ append pad " -pady $val" }
		ix*	{ append pad " -ipadx $val" }
		iy*	{ append pad " -ipady $val" }
		}
	}
}

Layout option {-define} {} configure {
	$class..define $define
}
Layout option {-dir} y

proc dir2side { order dir } {
	if [string compare $dir "x"] {
		if [string compare "normal" $order] {
			return bottom
		} else {
			return top
		}
	} else {
		if [string compare "normal" $order] {
			return right
		} else {
			return left
		}
	}
}

Layout method init args {
	instvar N dir order
	set N 0
	set order normal

	next
	if [string compare "" $args] {
		eval $self configure $args
	}
}

Layout method define args {
	instvar dir
	eval $self crFrames $dir $self $args
}

#----------------------------------------------------------------------
# METHOD crFrames - non public method.  Builds tree of frames.
#----------------------------------------------------------------------
Layout method crFrames { dir parent args } {
	instvar N name2f leaf_name2f memPackSibl memPackParent order

	set dir [dir2side $order $dir]

	foreach l $args {
		set name [lindex $l 0]
		set chdir [lindex $l 1]
		set opts [lindex $l 2]
		set children [lrange $l 3 end]

		if ![string compare "#" $name] { continue }
		getMgrOpts $opts crOpts packOpts
		if [string compare "*" $name] {
			if [string compare "" $children] {
				set name2f($name) $parent.f$name
			} else {
				set leaf_name2f($name) $parent.f$name
				if ![info exists name2f($name)] {
					set name2f($name) $parent.f$name
				}
			}
		} else {
			incr N 1
			set name $N
		}
		eval frame $parent.f$name $crOpts
		if ![string compare "normal" $order] {
		    eval pack $parent.f$name -side $dir -in $parent $packOpts
		}

		lappend memPackSibl($parent) $parent.f$name
		set memPackParent($parent.f$name) $parent

		foreach i $children {
			$self crFrames $chdir $parent.f$name $i
		}

		if [string compare "normal" $order] {
		    eval pack $parent.f$name -side $dir -in $parent $packOpts
		}
	}
}

#----------------------------------------------------------------------
# METHOD slaves - list managed windows
#----------------------------------------------------------------------
Layout method slaves args {
	instvar leaf_name2f
	set l {}
	foreach i [array names leaf_name2f] {
		set tmp [pack slaves $leaf_name2f($i)]
		if [string compare "" $tmp] {
			lappend l $tmp
		}
	}
	return $l
}

#----------------------------------------------------------------------
# METHOD list - list pairs of names and slaves.
#----------------------------------------------------------------------
Layout method list args {
	instvar leaf_name2f
	set l ""
	foreach i [array names leaf_name2f] {
		set tmp [pack slaves $leaf_name2f($i)]
		if [string compare "" $tmp] {
			append l "$i $tmp "
		}
	}
	return $l
}

#----------------------------------------------------------------------
# METHOD manage - manage windows.
#	args 	- Pairs of name windows specifiers: <name> <win>...
#----------------------------------------------------------------------
Layout method manage args {
	instvar win2name leaf_name2f

	array set toPack $args
	foreach i [array names toPack] {
		if [info exists leaf_name2f($i)] {
			if [string compare "" [array names leaf_name2f $i]] {
				pack forget [pack slaves $leaf_name2f($i)]
			}
			pack $toPack($i) -in $leaf_name2f($i) \
				-expand yes -fill both
			raise $toPack($i) $leaf_name2f($i)
			set win2name($toPack($i)) $i
		} else {
			puts stderr "$self: Can't manage '$toPack($i)': \
				symbolic name '$i' is unknown"
		}
	}
}

#----------------------------------------------------------------------
# METHOD unmanage - stop managing a window (symb. name can also be given)
#----------------------------------------------------------------------
Layout method unmanage args {
	instvar win2name leaf_name2f
	foreach i $args {
		if [info exists win2name($i)] {
			pack forget $i
			unset win2name($i)
		} elseif [info exists leaf_name2f($i)] {
			set w [pack slaves $leaf_name2f($i)]
			pack forget $w
			unset win2name($w)
		} else {
			puts stderr "$self: Can't unmanage '$i',\
				name is unknown"
		}
	}
}

#----------------------------------------------------------------------
# METHOD hide 	- hides specified braches or leafs
#		  Symbolic names are used.
#----------------------------------------------------------------------
Layout method hide args {
	instvar win2name name2f memPack memPackSibl hidden
	foreach i $args {
		if [info exists name2f($i)] {
			set w $name2f($i)
			set memPackSibl($i) [pack slaves [winfo parent $w]]
			set memPack($i) \
				"pack $w [pack info $w]"
			pack forget $w
			set hidden($i) $w
		} else {
			puts stderr "$self: can't hide '$i',\
				name is unknown"
		}
	}
}

#----------------------------------------------------------------------
# METHOD hidden	- returns symbols for currently hidden branches/leafs.
#----------------------------------------------------------------------
Layout method hidden {} {
	instvar win2name name2f memPack memPackSibl hidden
	set l {}
	foreach i [array names hidden] {
		lappend l $i
	}
	return $l
}

#----------------------------------------------------------------------
# METHOD unhide	- make hidden branches/leafs visible again
#----------------------------------------------------------------------
Layout method unhide args {
	instvar win2name name2f memPack memPackSibl memPackParent hidden
	foreach i $args {
		if [info exists memPack($i)] {
			set w $name2f($i)
			set sibl $memPackSibl($memPackParent($w))
			set pos [expr [lsearch $sibl $w]+1]
			set nowSibl [pack slaves $memPackParent($w)]
			set before ""
			foreach j [lrange $sibl $pos end] {
				if {[lsearch $nowSibl $j] != -1} {
					set before "-before $j"
					break
				}
			}
			eval $memPack($i) $before
			unset memPack($i)
			unset hidden($i)
		} else {
			puts stderr "$self: Can't unhide '$i',\
				not hidden, or name unknown"
		}
	}
}

#----------------------------------------------------------------------
