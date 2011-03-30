
DOC "class TkManText (Tk)" {
 NAME
	TkManText
		-  A class which handles TkMan-formatted text.

 DESCRIPTION
	TkMan formatted text is obtained by the following formatting:

		groff -man foo.n -Tascii | rman -f TkMan

	Where groff is Gnu's version of nroff, and rman is
	Rosetta-man.  Methods exist for loading man pages, catman pages,
	or ready-formatted TkMan pages.

 METHODS
	tkman_insert <list>
		Insert a TkMan-formatted list of data.

	loadMan <file name>
		Format and insert a man page by invoking `groff' and `rman'.

	loadCatman <file name>
		Format and insert a man page by invoking `rman'.

	loadTkMan <file name>
		Insert a manpage by reading file <file name>.

	setFonts <symbol> <font> ?<symbol> <font>..?

		Set fonts used for various types of text.
		Valid symbols are:

			normal	- Any normal text.
			b	- bold text
			i	- Italics text
			h1	- Header level 1
			h2	- Header level 2
			h3	- Header level 3

	All other methods are directed to the Tk text widget.
}

class TkManText
TkManText inherit Widget

TkManText method init args {
	next
	eval text $self.t $args
	pack $self.t -expand 1 -fill both

	$self setFonts \
	  normal "-*-courier-medium-r-*-*-12-*-*-*-*-*-*-*" \
	  b	"-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*" \
	  i	"-*-lucida-medium-i-*-*-12-*-*-*-*-*-*-*" \
	  h1	"-*-new century schoolbook-bold-r-*-*-12-*-*-*-*-*-*-*" \
	  h2	"-*-new century schoolbook-*-r-*-*-14-*-*-*-*-*-*-*" \
	  h3	"-*-new century schoolbook-bold-r-*-*-18-*-*-*-*-*-*-*"

}

TkManText method setFonts args {
	instvar fontmap
	array set Fonts $args
	foreach i [array names Fonts] {
		set fontmap($i) $Fonts($i)
	}
	$self.t configure -font $fontmap(normal)
	$self.t tag configure b -font $fontmap(b)
	$self.t tag configure i -font $fontmap(i)
	$self.t tag configure h1 -font $fontmap(h1)
	$self.t tag configure h2 -font $fontmap(h2)
	$self.t tag configure h3 -font $fontmap(h3)
}

TkManText method tkman_insert list {
	set t $self.t
	set tmp [$t cget -stat]
	$t configure -stat normal
	eval $list
	$t configure -stat $tmp
}

TkManText method loadMan fname {
	set t $self-cmd
	eval [exec groff -man $fname -Tascii | rman -k -f TkMan 2>@stderr]
	return ""
}

TkManText method loadCatman fname {
	set t $self-cmd
	eval [exec rman -k -f TkMan $fname 2>@stderr]
	return ""
}

TkManText method loadTkMan fname {
	set t $self-cmd
	eval [exec cat $fname 2>@stderr]
	return ""
}

TkManText method textWidgetName {} {
	return $self.t
}

TkManText method unknown args {
	eval {$self.t $method} $args
}

TkManText method configure args {
	eval {$self.t configure} $args
}
TkManText method cget opt {
	$self.t cget $opt
}

TkManText method configure_unknown { opt val } {
	$self.t configure $opt $val
}

TkManText method init_unknown { opt val } {
	$self.t configure $opt $val
}

TkManText method cget_unknown opt {
	$self.t cget $opt
}

#----------------------------------------------------------------------
