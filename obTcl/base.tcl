crunch_skip begin
DOC "class Base" {
 NAME
	Base	- The basic class inherited by all obTcl objects

 SYNOPSIS
	Base new <obj>
		- Creates an object of the simplest possible class.

 DESCRIPTION
	All classes inherits the Base class automatically.  The Base class
	provides methods that are essential for manipulating obTcl-objects,
	such as `info' and `destroy'.

 METHODS
	Base provides the following generic methods to all objects:

	new		- EXPERIMENTAL! Arranges to create a new object of
			  the class of the invoking object.

	instance	- EXPERIMENTAL! Arranges to create a new object of
			  the class of the invoking object.  This method
			  differs from `new' by NOT automatically invoking
			  the `init' method of the new object.
			  One possible usage:  Create a replacement for the
			  normal class object -a replacement which has no
			  hard-coded methods (this will need careful design
			  though).

	init		- Does nothing.  The init method is automatically
			  invoked whenever an object is created with `new'.

	destroy		- Frees all instance variables of the object, and
			  the object itself.

	class		- Returns the class of the object.

	set name ?value?
			- Sets the instance variable `name' to value.
			  If no value is specified, the current value is
			  returned.  Mainly used for debugging purposes.

	info <cmd>	- Returns information about the object.  See INFO
			  below.

	eval <script>	- Evaluates `script' in the context of the object.
			  Useful for debugging purposes.  Not meant to be
			  used for other purposes (create a method instead).
			  One useful trick (if you use the Tcl-debugger in
			  this package) is to enter:
				obj eval bp
			  to be able to examine `obj's view of the world
			  (breakpoints must be enabled, of course).

	unknown <method> <args>
			- Automatically invoked when unknown methods are
			  invoked.  the Base class defines this method to
			  print an error message, but this can be overridden
			  by derived classes.

	option <opt> <default> ?<section1> <body1>? ?<section2> <body2>?..
			- Define an option handler.
			  See OPTION HANDLER below for a description.

	conf_verify <args>
	conf_init <args>
			- Set options. <args> are option-value pairs.
			  See OPTION HANDLER below for a description.

	configure <args>
			- Set options. <args> are option-value pairs.
			  See OPTION HANDLER below for a description.

	cget <opt>	- Get option value.
			  See OPTION HANDLER below for a description.

	verify_unknown <args>
	init_unknown <args>
	configure_unknown <args>
	cget_unknown <opt>
			- These methods are automatically invoked when a requested
			  option has not been defined.
			  See OPTION HANDLER below for a description.

 INFO
	The method `info' can be used to inspect an object.  In the list below
	(I) means the command is only applicable to object instances, whereas
	(C) means that the command can be applied either to the class object, or
	to the object instance, if that is more convenient.
	Existing commands:

	instvars	- (I) Returns the names of all existing instance variables.
	iclassvars	- (I) List instance class variables

	classvars	- (C) List class variables.
	objects		- (C) List objects of this class.
	methods		- (C) List methods defined in this class.
	sysmethods	- (C) List system methods defined in this class.
	cached		- (C) List cached methods for this class.
	body <method>	- (C) List the body of a method.
	args <method>	- (C) List formal parameters for a method.

	options		- (I) List the current option values in the format
			  "option-value pairs".
	defaults	- (C) List the current default values in the format
			  "option-value pairs".  These values are the initial
			  values each new object will be given.

 OPTION HANDLER
	The method `option' is used to define options. It should be used on
	the class-object, which serves as a repository for default values
	and for code sections to run to verify and make use of new default values.
	
	option <opt> <default> ?<section1> <body1>? ?<section2> <body2>?..
		Define an option for this class.
		Defining an option results in an instance variable
		of the same name (with the leading '-' stripped)
		being defined.  This variable will be initiated
		with the value <default>.

		The sections `verify', `init' and `configure' can be defined.

		`verify' is used to verify new parameters without affecting
		the object.  It is typically called by an object's init method
		before all parts of the object have been created.

		`init' is used for rare situations where some action should be taken
		just after the object has been fully created.  I.e when setting
		the option variable via `verify' was not sufficient.

		The `configure' section is invoked when the configure method is
		called to re-configure an object. 

		Example usage:

		    class Graph
		    Graph inherit Widget

		    Graph option {-width} 300 verify {
			if { $width >= 600 } {
				error "width must be less than 600"
			}
		    } configure {
			$self.grf configure -width $width
		    }

		Note 1: The `verify' section should never attempt
		to access structures in the object (i.e widgets), since
		it is supposed to be callable before they exist!
		Use the `configure' section to manipulate the object.

		Note 2: Using "break" or "error" in the verify section results
		in the newly specified option value being rejected.

	conf_verify <args>
		Invoke all "verify" sections for options-value pairs
		specified in <args>.
	conf_init <args>
		Invoke all "init" sections for options-value pairs
		specified in <args>.

		Example usage:

		    Graph method init { args } {
			instvar width

			# Set any option variables from $args
			#
			eval $self conf_verify $args  ;# Set params

			next -width $width	;# Get frame

			CreateRestOfObject	;# Bogus

			# Option handlers that wish to affect the
			# object during init may declare an "init"
			# section.  Run any such sections now:
			#
			eval $self conf_init $args
		    }

		    Graph .graph -width 400	;# Set width initially

	configure <args>
		Invoke all "configure" sections for options-value pairs
		specified in <args>.

		Example usage:

		    # First create object 
		    #
		    Graph .graph -width 300

		    # Use `configure' to configure the object
		    #
		    .graph configure -width 200

	cget <opt>
		Returns the current value of option <opt>.
		Example usage:

			.graph cget -width

	<sect>_unknown <args>
		These methods are called when attempting to invoke sections
		for unknown options.  In this way a class may define methods
		to catch usage of "configure", "cget", etc. for undefined
		options.
		Example:

		    Graph method configure_unknown { opt args } {
			eval {$self-cmd configure $opt} $args
		    }

		See the definitions of the Base and Widget classes for their
		usage of these methods.
}
crunch_skip end

#----------------------------------------------------------------------
# Define the Base class.  This class provides introspection etc.
#
# It also provides "set", which gives access to object
# internal variables, and 'eval' which lets you run arbitrary scripts in
# the objects context.  You may wish to remove those methods if you
# want to disallow this.

class Base

Base method init args {}

Base method destroy args {
	otFreeObj $self
}

Base method class args {
	return $iclass
}

# Note: The `set' method takes on the class of the caller, so
#	instvars will use the callers scope.
#
Base method set args {
	set class $iclass
	# instvar [lindex $args 0]
	set var [lindex $args 0]
	regexp -- {^([^(]*)\(.*\)$} $var m var
	instvar $var
	return [eval set $args]
}

Base method eval l {
	return [eval $l]
}

Base method info { cmd args } {
	switch $cmd {
	"instvars"	{return [eval {otObjInfoVars\
		_oIV_${iclass}.${self}. _oIV_${iclass}.${self}.} $args]}
	"iclassvars"	{otObjInfoVars _oICV_${iclass}. _oICV_${iclass}. $args}
	"classvars"	{otObjInfoVars _oDCV_${iclass}. _oDCV_${iclass}. $args}
	"objects"	{otObjInfoObjects $iclass}
	"methods"	{otClassInfoMethods $iclass}
	"sysmethods"	{otClassInfoSysMethods $iclass}
	"cached"	{otClassInfoCached $iclass}
	"body"		{otClassInfoBody $iclass $args}
	"args"		{otClassInfoArgs $iclass $args}

	"options"	{$iclass..collectOptions values ret
			return [array get ret] }
	"defaults"	{$iclass..collectOptions defaults ret
			return [array get ret] }

	default	{
			return -code error \
			-errorinfo "Undefined command 'info $cmd'" \
			"Undefined command 'info $cmd'"
		}
	}	
}

Base method unknown args {
	return -code error \
		-errorinfo "Undefined method '$method' invoked" \
		"Undefined method '$method' invoked"
}

#------- START EXPERIMENTAL

Base method new { obj args } {
	eval {otNew $iclass $obj} $args
}

Base method instance { obj args } {
	eval {otInstance $iclass $obj} $args
}

Base method sys_method args {
	eval {otMkMethod S $iclass} $args
}

Base method method args {
	eval {otMkMethod N $iclass} $args
}

Base method del_method args {
	eval {otRmMethod $iclass} $args
}

Base method inherit args {
	eval {otInherit $iclass} $args
}

# class AnonInst - inherit from this class to be able to generate
# anonymous objects.  Example:
#
#	class Foo
#	Foo inherit AnonInst
#	set obj [Foo new]
#
# NOTE: EXPERIMENTAL!!!

class AnonInst
AnonInst method anonPrefix p {
	iclassvar _prefix
	set _prefix $p
}
AnonInst method new {{obj {}} args} {
	iclassvar _count _prefix
	if ![info exists _count] {
		set _count 0
	}
	if ![info exists _prefix] {
		set _prefix "$iclass"
	}
	if ![string compare "" $obj] {
        	set obj $_prefix[incr _count]
	}
	eval next {$obj} $args
	return $obj
}
#------- END EXPERIMENTAL

#----------------------------------------------------------------------
# Configure stuff
#----------------------------------------------------------------------
# The configuaration stuff is, for various reasons, probably the most
# change-prone part of obTcl.
#
# After fiddling around with various methods for handling options,
# this is what I came up with.  It uses one method for each class and option,
# plus one dispatch-method for each of "conf_init", "conf_verify", "configure"
# and "cget" per class.  Any extra sections in the `option' handler
# results in another dispatch-method being created.
# Attempts at handling undefined options are redirected to
#
#	<section_name>_unknown
#
# Note:
#	Every new object is initialized by a call to `initialize'.
#	This is done in the proc "new", before `init' is called, to guarantee
#	that initial defaults are set before usage.  `initialize' calls "next", so
#	all inherited classes are given a chance to set their initial defaults.
#
# Sections and their used (by convention):
#
#	verify	- Called at beginning of object initialization to verify
#		  specified options.
#	init	- Called at end of the class' `init' method.
#		  Use for special configuration.
#	configure
#		- This section should use the new value to configure
#		  the object.
#

# MkSectMethod - Define a method which does:
#   For each option specified, call the handler for the specified section
#   and option.  If this fails, call the <section>_unknown handler.
#   If this fails too, return an error.
#   Note that the normal call of the method `unknown' is avoided by
#   telling the unknown handler to avoid this (by means of the global array
#   "_obTcl_unknBarred").
#
proc otMkSectMethod { class name sect } {
	$class sys_method $name args "
		array set Opts \$args
		foreach i \[array names Opts\] {
			global _obTcl_unknBarred
			set _obTcl_unknBarred(\$class..${sect}.\$i) 1
			if \[catch {\$class..$sect.\$i \$Opts(\$i)} err\] {
				if \[catch {\$class..${sect}_unknown\
					\$i \$Opts(\$i)}\] {
					unset _obTcl_unknBarred(\$class..${sect}.\$i)
				  	error \"Unable to do '$sect \$i \$Opts(\$i)'\n\
						\t\$err
					\"
			 	}
			}
			unset _obTcl_unknBarred(\$class..${sect}.\$i)
		}
	"
}

# Note: MkOptHandl is really a part of `option' below.
#
proc otMkOptHandl {} {
    uplevel 1 {
	$iclass sys_method "cget" opt "
		classvar classOptions
		if \[catch {$iclass..cget.\$opt} ret\] {
			if \[catch {\$class..cget_unknown \$opt} ret\] {
			  	error \"Unable to do 'cget \$opt'\"
			}
		}
		return \$ret
	"
	otMkSectMethod $iclass conf_init init
	$iclass sys_method initialize {} {
		next
		classvar optDefaults
		eval instvar [array names optDefaults]
		foreach i [array names optDefaults] {
			set $i $optDefaults($i)
		}
	}

	# arr - Out-param
	#
	$iclass sys_method collectOptions { mode arr } {
		classvar classOptions optDefaults

		upvar 1 $arr ret
		next $mode ret
		eval instvar [array names optDefaults]
		foreach i [array names optDefaults] {
			if [string compare "defaults" $mode] {
				set ret(-$i) [set $classOptions(-$i)]
			} else {
				set ret(-$i) $optDefaults($i)
			}
		}
	}
	otMkSectMethod $iclass conf_verify verify
	otMkSectMethod $iclass configure configure

	set _optPriv(section,cget) 1
	set _optPriv(section,init) 1
	set _optPriv(section,initialize) 1
	set _optPriv(section,verify) 1
	set _optPriv(section,configure) 1
    }
}

otMkSectMethod Base configure configure

# _optPriv is used for internal option handling house keeping
# Note: checking for existence of a proc is not always a good idea,
# since it may simply be a cached pointer to a inherited method.
#
Base method option { opt dflt args } {

	classvar_of_class $iclass optDefaults classOptions _optPriv

	set var [string range $opt 1 end]
	set optDefaults($var) $dflt
	set classOptions($opt) $var

	array set tmp $args
	if ![info exists _optPriv(initialize)] {
		otMkOptHandl
		set _optPriv(initialize) 1
	}
	foreach i [array names tmp] {
		if ![info exists _optPriv(section,$i)] {
			otMkSectMethod $iclass $i $i
			set _optPriv(section,$i) 1
		}
		$iclass sys_method "$i.$opt" _val "
			instvar $var
			set _old_val \$[set var]
			set $var \$_val
			set ret \[catch {$tmp($i)} res\]
			if {\$ret != 0 && \$ret != 2 } {
				set $var \$_old_val
				return -code \$ret -errorinfo \$res \$res
			}
			return \$res
		"
		set _optPriv($i.$opt) 1
	}
	if ![info exists _optPriv(cget.$opt)] {
		$iclass sys_method "cget.$opt" {} "
			instvar $var
			return \$[set var]
		"
		set _optPriv(cget.$opt) 1
	}
	if ![info exists tmp(verify)] {
		$iclass sys_method "verify.$opt" _val "
			instvar $var
			set $var \$_val
		"
		set _optPriv(verify.$opt) 1
	}
	if ![info exists tmp(configure)] {
		$iclass sys_method "configure.$opt" _val "
			instvar $var
			set $var \$_val
		"
		set _optPriv(configure.$opt) 1
	}
	if ![info exists tmp(init)] {
		$iclass sys_method "init.$opt" _val {}
		set _optPriv(init.$opt) 1
	}
}

# Default methods for non-compulsory
# standard sections in an option definition:
#
Base sys_method init_unknown { opt val } {}
Base sys_method verify_unknown { opt val } {}

# Catch initialize for classes which have no option handlers:
#
Base sys_method initialize {} {}

# Catch conf_init in case no option handlers have been defined.
#
Base sys_method conf_init {} {}

crunch_skip begin

#----------------------------------------------------------------------
#
# class Widget
#	Base class for obTcl's Tk-widgets.
#

DOC "class Widget (Tk) base class for widgets" {
 NAME
	Widget	- A base class for mega-widgets

 SYNOPSIS
	Widget new <obj> ?tk_widget_type? ?config options?
	Widget <obj> ?tk_widget_type? ?config options?

 DESCRIPTION
	The widget class provides a base class for Tk-objects.
	This class knows about widget naming conventions, so, for example,
	destroying a Widget object will destroy any descendants of this object.

	The `new' method need not be specified if the object name starts with a
	leading ".".  Thus giving syntactical compatibility with Tk for
	creating widgets.

	If `tk_widget_type' is not specified, the widget will be created as
	a `frame'.  If the type is specified it must be one of the existing
	Tk-widget types, for example: button, radiobutton, text, etc.
	See the Tk documentation for available widget types.

	The normal case is to use a frame as the base for a mega-widget.
	This is also the recommended way, since it results in the Tk class-name
	of the frame being automatically set to the objects class name  -thus
	resulting in "winfo class <obj>" returning the mega-widget's class
	name.

	In order to create mega-widgets, derive new classes from this class.

 METHODS
	The following methods are defined in Widget:

	init ?<args>?	- Creates a frame widget, and configures it if any
			  configuration options are present.  Automatically
			  invoked by the creation process, so there is no
			  need to call it (provided that you use 'next' in
			  the init-method of the derived class).

	destroy		- Destroys the object and associated tk-widget.
			  For Tk-compatibility, the function `destroy' can be
			  used instead, example:

				destroy <obj>

			  Note:  If you plan to mix Tk-widgets transparently
			  with mega-widgets, you should use the _function_
			  `destroy'.
			  Any descendant objects of <obj> will also be
			  destroyed (this goes for both Tk-widgets and
			  mega-widgets).

	set		- Overrides the `set' method of the Base class to
			  allow objects of type `scrollbar' to work correctly.

	unknown		- Overrides the `unknown' method of the Base class.
			  Directs any unknown methods to the main frame of
			  the Widget object.

	unknown_opt	- Overrides the same method from the Base class.
			  Automatically called from the option handling system.
			  Directs any unknown options to the main frame of the
			  Widget object.

	In addition, all non-shadowed methods from the Base class can be used.

	Any method that cannot be resolved is passed on to the associated
	Tk-widget.  This behaviour can be altered for any derived classes
	by defining a new `unknown' method (thus shadowing Widget's own
	`unknown' method).  The same technique can be used to override
	the `unknown_opt' method.

 EXAMPLES
	A simple example of deriving a class MegaButton which consists of
	a button widget initiated with the text "MEGA" (yes, I know, it's
	silly).

		class MegaButton
		MegaButton inherit Widget

		MegaButton method init { args } {
			#
			# Allow the Widget class to create a button for us
			# (we need to specify widget type `button')
			#
			eval next button $args
	
			$self configure -text "MEGA"
		}

		frame .f
		MegaButton .f.b -background red -foreground white
		pack .f .f.b

	This example shows how to specify a Tk-widget type (button), although
	I advice against specifying anything (thus using a frame).
	See DESCRIPTION above for the reasoning behind this.  Also note that
	`eval' is used to split $args into separate arguments for passing to
	the init method of the Widget class.

	A more realistic example:

		class ScrolledText
		ScrolledText inherit Widget

		ScrolledText method init { args } {
			next

			text $self.t -yscrollcommand "$self.sb set"
			scrollbar $self.sb -command "$self.t yview"

			pack $self.sb -side right -fill y
			pack $self.t -side left

			eval $self configure $args
		}

		ScrolledText method unknown { args } {
			eval {$self.t $method} $args
		}

		ScrolledText .st
		.st insert end [exec cat /etc/passwd]
		pack .st

	This creates a new class, ScrolledText, containing a text window
	and a vertical scrollbar.  It arranges for all unknown methods to
	be directed to the text widget; thus allowing `.st insert' to work
	normally (along with any other text methods).

 NOTES
	Widget binds the "destroy" method to the <Destroy> event of
	the holding window, so be careful not to remove this binding
	inadvertently.
}

crunch_skip end

class Widget

# init	Create a tk-widget of specified type (or frame if not specified).
#	If the corresponding Tk-widget already exists, it will be used.
#	Otherwise the Tk-widget will be created.
#	The tk-widget will be named $self if $self has a leading ".",
#	otherwise a "." is prepended to $self to form the wigdet name.
#	The instvar `win' will contain the widgets window name, and
#	the instvar `wincmd' will contain the name of the widget's associated
#	command.

Widget method init args {
	instvar win wincmd

	next

	set first "[lindex $args 0]"
	set c1 "[string index $first 0]"
	if { ![string compare "" "$c1"] || ![string compare "-" "$c1"] } {
		set type frame
		set cl "-class $iclass"
	} else {
		set type $first
		set args [lrange $args 1 end]
		set cl ""
	}
	if [string compare "" [info commands $self-cmd]] {
		set win $self
		set wincmd $self-cmd
	} else {
	  if ![string compare "." [string index $self 0]] {
		rename $self _ooTmp
		eval $type $self $cl $args
		rename $self $self-cmd
		rename _ooTmp $self
		set win $self
		set wincmd $self-cmd
	  } else {
		eval $type .$self $cl $args
		set win .$self
		#set wincmd .$self-cmd
		set wincmd .$self
	  }
	}
	bind $win <Destroy> "\
		if { !\[string compare \"%W\" \"$self\"\] && !\[catch {info args $self}\] } {
			$self destroy -obj_only }"

	return $self
}

# Just for the case when there are no option-handlers defined:
#
Widget sys_method configure args {
	instvar wincmd
	eval {$wincmd configure} $args
}
Widget sys_method cget opt {
	instvar wincmd
	eval {$wincmd cget} $opt
}

Widget sys_method configure_unknown { opt args } {
	instvar wincmd
	eval {$wincmd configure $opt} $args
}

Widget sys_method cget_unknown opt {
	instvar wincmd
	$wincmd cget $opt
}
Widget sys_method init_unknown { opt val } {
	puts "init_unknown: $opt $val (iclass=$iclass class=$class)"
}

Widget sys_method unknown args {
	instvar wincmd
	eval {$wincmd $method} $args
}

# Note: no "next" used!  Does the `Base..destroy' stuff here for performance.
#
Widget method destroy args {
	
	instvar win wincmd

	# Must copy vars since they are destroyed by `otFreeObj'
	set wp $win
	set w $wincmd

	otFreeObj $self
	catch {bind $w <Destroy> {}}
	if [string compare "-obj_only" $args] {
		if [string compare $w $wp] {
			rename $w $wp
		}
		if [string compare "-keepwin" $args] {
			destroy $wp
		}
	}
}

# The method `set' defined here shadows the `set' method from Base.
# This allows wrapper objects around Tk-scrollbars to work correctly.
#
Widget sys_method set args {
	instvar wincmd
	eval {$wincmd set} $args
}

Widget sys_method base_set args {
	eval Base..set $args
}

