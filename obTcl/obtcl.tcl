#----------------------------------------------------------------------
#				-- obTcl --
#
# `obTcl' is a Tcl-only object- and Megawidget-extension.
#
# The system supports multiple inheritance, three new storage classes,
# and fully transparent Tk-megawidgets.
#
# Efficiency is obtained through method-resolution caching.
# obTcl provides real instance variables and class variables
# (they may be arrays).  Two types of class variables are provided:
# definition-class scoped, and instance-class scoped.
#
# The mega-widget support allows creation of mega-widgets which handle
# like ordinary Tk-widgets; i.e can be "packed", "deleted", "placed" etc,
# intermixed with ordinary Tk-widgets.
# The transparency of the mega-widget extension has been tested by
# wrapping all normal Tk-widgets into objects and running the standard
# "widget" demo provided with Tk4.0.
# 
# To try out obTcl, just start `wish' (Tk4.0 or later) and do "source demo".
# Alternatively run "demo" directly (requires that wish can be located
# by demo).
#
# If you run `wish' interactively and source `obtcl', you will be able to
# type "help" to access a simple help system.
#
# Pronunciation: `obTcl' sounds like "optical".
#
# See COPYRIGHT for copyright information.
#
# Please direct comments, ideas, complaints, etc. to:
#
#	patrik@dynas.se
# 
#   Patrik Floding
#   DynaSoft AB
#
#----------------------------------------------------------------------

# For convenience you may either append the installation directory of
# obTcl to your auto_path variable (the recommended method), or source
# `obtcl.tcl' into your script.  Either way everything should work.
#

set OBTCL_LIBRARY [file dirname [info script]]
if { [lsearch -exact $auto_path $OBTCL_LIBRARY] == -1 } {
	lappend auto_path $OBTCL_LIBRARY
}

set obtcl_version "0.56"

crunch_skip begin
cmt {
    Public procs:

	- Std. features
	classvar
	iclassvar
	instvar
	class
	obtcl_mkindex
	next

	- Subj. to changes
	instvar2global
	classvar_of_class
	instvar_of_class
	import
	renamed_instvar
	is_object
	is_class

    Non public:

	Old name			New name (as of 0.54)
	--------			----------------------
	new				otNew
	instance			otInstance
	freeObj				otFreeObj
	classDestroy			otClassDestroy
	getSelf				otGetSelf
	mkMethod			otMkMethod
	rmMethod			otRmMethod
	delAllMethods			otDelAllMethods
	objinfoVars			otObjInfoVars
	objinfoObjects			otObjInfoObjects
	classInfoBody			otClassInfoBody
	classInfoArgs			otClassInfoArgs
	classInfoMethods+Cached		otClassInfoMethods+Cached
	classInfoMethods		otClassInfoMethods
	classInfoSysMethods		otClassInfoSysMethods
	classInfoCached			otClassInfoCached
	inherit				otInherit
	InvalidateCaches		otInvalidateCaches
	chkCall				otChkCall
	GetNextFunc			otGetNextFunc
	GetFunc				otGetFunc
	GetFuncErr			otGetFuncErr
	GetFuncMissingClass		otGetFuncMissingClass
}
crunch_skip end

proc instvar2global name {
	upvar 1 class class self self
	return _oIV_${class}.${self}.$name
}

# Class variables of definition class
if ![string compare [info commands classvar] ""] {
proc classvar args {
	uplevel 1 "foreach _obTcl_i [list $args] {
		upvar #0 _oDCV_\${class}.\$_obTcl_i \$_obTcl_i
	}"
}
}

# Class variables of specified class
proc classvar_of_class { class args } {
	uplevel 1 "foreach _obTcl_i [list $args] {
		upvar #0 _oDCV_${class}.\$_obTcl_i \$_obTcl_i
	}"
}

# Class variables of instance class
if ![string compare [info commands iclassvar] ""] {
proc iclassvar args {
	uplevel 1 "foreach _obTcl_i [list $args] {
		upvar #0 _oICV_\${iclass}.\$_obTcl_i \$_obTcl_i
	}"
}
}

# Instance variables. Specific to instances.
# Make instvar from `class' available
# Use with caution!  I might put these variables in a separate category
# which must be "exported" vaiables (as opposed to "instvars").
#
proc instvar_of_class { class args } {
	uplevel 1 "foreach _obTcl_i [list $args] {
		upvar #0 _oIV_${class}.\${self}.\$_obTcl_i \$_obTcl_i
	}"
}
# Instance variables. Specific to instances.

if ![string compare [info commands instvar] ""] {
proc instvar args {
	uplevel 1 "foreach _obTcl_i [list $args] {
		upvar #0 _oIV_\${class}.\${self}.\$_obTcl_i \$_obTcl_i
	}"
}
}

# Renamed Instance variable. Specific to instances.
proc renamed_instvar { normal_name new_name } {
	uplevel 1 "upvar #0 _oIV_\${class}.\${self}.$normal_name $new_name"
}

# Check if an object exists
#
proc is_object name {
	global _obTcl_Objects
	if [info exists _obTcl_Objects($name)] {
		return 1
	} else {
		return 0
	}
}
# Check if a class exists
#
proc is_class name {
	global _obTcl_Classes
	if [info exists _obTcl_Classes($name)] {
		return 1
	} else {
		return 0
	}
}

#----------------------------------------------------------------------
# new	Creates a new object.  Creation involves creating a proc with
#	the name of the object, initializing some house-keeping data,
#	call `initialize' to set init any option-variables,
#	and finally calling the `init' method for the newly created object.
#
# 951024. Added rename of any existing command to facilitate wrapping
# of existing widgets/commands. Only one-level wrapping is supported.

proc otNew { iclass obj args } {
	global _obTcl_Objclass _obTcl_Objects
	set _obTcl_Objclass($iclass,$obj) $obj

	if ![info exists _obTcl_Objects($obj)] {
		catch {rename $obj ${obj}-cmd}
	}

	set _obTcl_Objects($obj) 1
	otProc $iclass $obj

	set self $obj
	eval {$iclass..initialize}
	eval {$iclass..init} $args
}

if ![string compare [info commands otProc] ""] {
proc otProc { iclass obj } {
	proc $obj { cmd args } "
		set self $obj
		set iclass $iclass

		if \[catch {eval {$iclass..\$cmd} \$args} val\] {
			return -code error \
			  -errorinfo \"$obj: \$val\" \"$obj: \$val\"
		} else {
			return \$val
		}
	"
}
}

# otInstance
#	Exactly like new, but does not call the 'init' method.
#	Useful when creating a class-leader object.  Class-leader
#	objects are used instead of class names when it is desirable
#	to avoid some hard-coded method ins the class proc.
#
proc otInstance { iclass obj args } {
	global _obTcl_Objclass _obTcl_Objects
	set _obTcl_Objclass($iclass,$obj) $obj

	if ![info exists _obTcl_Objects($obj)] {
		catch {rename $obj ${obj}-cmd}
	}

	set _obTcl_Objects($obj) 1
	proc $obj { cmd args } "
		set self $obj
		set iclass $iclass

		if \[catch {eval {$iclass..\$cmd} \$args} val\] {
			return -code error \
			  -errorinfo \"$obj: \$val\" \"$obj: \$val\"
		} else {
			return \$val
		}
	"
	set self $obj
	eval {$iclass..initialize}
}

#----------------------------------------------------------------------
# otFreeObj
#	Unset all instance variables.
#
proc otFreeObj obj {
	global _obTcl_Objclass _obTcl_Objects
	otGetSelf
	catch {uplevel #0 "eval unset _obTcl_Objclass($iclass,$obj) \
			_obTcl_Objects($obj) \
			\[info vars _oIV_*.${self}.*\]"}
	catch {rename $obj {}}
}

#setIfNew _obTcl_Classes() ""
setIfNewArray _obTcl_Classes ""
setIfNew _obTcl_NoClasses 0

# This new class proc allows overriding of the 'new' method.
# The usage of `new' in the resulting class object is about 10% slower
# than before though..
#
proc class class {
	global _obTcl_NoClasses _obTcl_Classes _obTcl_Inherits

	if [info exists _obTcl_Classes($class)] {
		set self $class
		otClassDestroy $class
	}
	if [string match *.* $class] {
		puts stderr "class: Fatal Error:"
		puts stderr "       class name `$class'\
				contains reserved character `.'"
		return
	}
	incr _obTcl_NoClasses 1
	set _obTcl_Classes($class) 1

	set iclass $class; set obj $class;

	proc $class { cmd args } "
		set self $obj
		set iclass $iclass

		switch -glob \$cmd {
		.*		{ eval {$class..new \$cmd} \$args }
		new		{ eval {$class..new} \$args }
		method		{ eval {otMkMethod N $class} \$args}
		inherit		{ eval {otInherit $class} \$args}
		destroy		{ eval {otClassDestroy $class} \$args }
		init		{ return -code error \
		     -errorinfo \"$obj: Error: classes may not be init'ed!\" \
				\"$obj: Error: classes may not be init'ed!\"
		}
 		default		{
			if \[catch {eval {$iclass..\$cmd} \$args} val\] {
				return -code error \
				  -errorinfo \"$obj: \$val\" \"$obj: \$val\"
			} else {
				return \$val
			}
		 }
		}
	"

	if [string compare "Base" $class] {
		$class inherit "Base"
	} else {
		set _obTcl_Inherits($class) {}
	}
	return $class
}

proc otClassDestroy class {
	global _obTcl_NoClasses _obTcl_Classes ;# _obTcl_CacheStop
	otGetSelf

	if ![info exists _obTcl_Classes($class)] { return }
	otInvalidateCaches 0 $class [otClassInfoMethods $class]
	otDelAllMethods $class
	rename $class {}
	incr _obTcl_NoClasses -1
	unset _obTcl_Classes($class)

	uplevel #0 "
		foreach _iii  \[info vars _oICV_${class}.*\] {
			unset \$_iii
		}
		foreach _iii  \[info vars _oDCV_${class}.*\] {
			unset \$_iii
		}
		catch {unset _iii}
	"
	otFreeObj $class
}

# otGetSelf -
#   Bring caller's ID into scope.  For various reasons
#   an "inlined" (copied) version is used in some places.  Theses places
#   can be located by searching for the word 'otGetSelf', which should occur
#   in a comment near the "inlining".
#

if ![string compare [info commands otGetSelf] ""] {
proc otGetSelf {} {
	uplevel 1 {upvar 1 self self iclass iclass Umethod method}
}
}

proc otMkMethod { mode class name params body } {
	otInvalidateCaches 0 $class $name

	if [string compare "unknown" "$name"] {
		set method "set method $name"
	} else {
		set method ""
	}
	proc $class..$name $params \
		"otGetSelf
set class $class
$method
$body"
	if ![string compare "S" $mode] {
		global _obTcl_SysMethod
		set _obTcl_SysMethod($class..$name) 1
	}
}

proc otRmMethod { class name } {
	global _obTcl_SysMethod

	if [string compare "unknown" "$name"] {
		otInvalidateCaches 0 $class $name
	} else {
		otInvalidateCaches 0 $class *
	}
	rename $class..$name {}
	catch {unset _obTcl_SysMethod($class..$name)}
}

proc otDelAllMethods class {
	global _obTcl_Cached
	foreach i [info procs $class..*] {
		if [info exists _obTcl_SysMethod($i)] {
			continue
		}
		if [info exists _obTcl_Cached($i)] {
			unset _obTcl_Cached($i)
		}
		rename $i {}
	}
}
proc otObjInfoVars { glob base { match "" } } {
	if ![string compare "" $match] { set match * }
	set l [info globals ${glob}$match]
	set all {}
	foreach i $l {
		regsub "${base}(.*)" $i {\1} tmp
		lappend all $tmp
	}
	return $all
}
proc otObjInfoObjects class {
	global _obTcl_Objclass
	set l [array names _obTcl_Objclass $class,*]
	set all {}
	foreach i $l {
		regsub "${class},(.*)" $i {\1} tmp
		lappend all $tmp
	}
	return $all
}
proc otClassInfoBody { class method } {
	global _obTcl_Objclass _obTcl_Cached
	if [info exists _obTcl_Cached(${class}..$method)] { return }
	if [catch {set b [info body ${class}..$method]} ret] {
		return -code error \
		  -errorinfo "info body: Method '$method' not defined in class $class" \
			"info body: Method '$method' not defined in class $class"
	} else {
		return $b
	}
}
proc otClassInfoArgs { class method } {
	global _obTcl_Objclass _obTcl_Cached
	if [info exists _obTcl_Cached(${class}..$method)] { return }
	if [catch {set b [info args ${class}..$method]} ret] {
		return -code error \
		  -errorinfo "info args: Method '$method' not defined in class $class" \
			"info args: Method '$method' not defined in class $class"
	} else {
		return $b
	}
}
proc otClassInfoMethods+Cached class {
	global _obTcl_Objclass _obTcl_SysMethod
	set l [info procs ${class}..*]
	set all {}
	foreach i $l {
		regsub "${class}..(.*)" $i {\1} tmp
		if [info exists _obTcl_SysMethod($i)] { continue }
		lappend all $tmp
	}
	return $all
}
proc otClassInfoMethods class {
	global _obTcl_Objclass _obTcl_Cached _obTcl_SysMethod
	set l [info procs ${class}..*]
	set all {}
	foreach i $l {
		if [info exists _obTcl_Cached($i)] { continue }
		if [info exists _obTcl_SysMethod($i)] { continue }
		regsub "${class}..(.*)" $i {\1} tmp
		lappend all $tmp
	}
	return $all
}
proc otClassInfoSysMethods class {
	global _obTcl_Objclass _obTcl_Cached _obTcl_SysMethod
	set l [info procs ${class}..*]
	set all {}
	foreach i $l {
		if [info exists _obTcl_Cached($i)] { continue }
		if ![info exists _obTcl_SysMethod($i)] { continue }
		regsub "${class}..(.*)" $i {\1} tmp
		lappend all $tmp
	}
	return $all
}
proc otClassInfoCached class {
	global _obTcl_Objclass _obTcl_Cached _obTcl_SysMethod
	if ![array exists _obTcl_Cached] {
		return
	}
	set l [array names _obTcl_Cached $class..*]
	set all {}
	foreach i $l {
		regsub "${class}..(.*)" $i {\1} tmp
		if [info exists _obTcl_SysMethod($i)] { continue }
		lappend all $tmp
	}
	return $all
}

# obtcl_mkindex:
# Altered version of tcl7.4's auto_mkindex.
# This version also indexes class definitions.
#
# Original comment:
# Regenerate a tclIndex file from Tcl source files.  Takes as argument
# the name of the directory in which the tclIndex file is to be placed,
# floowed by any number of glob patterns to use in that directory to
# locate all of the relevant files.

proc obtcl_mkindex {dir args} {
    global errorCode errorInfo
    set oldDir [pwd]
    cd $dir
    set dir [pwd]
    append index "# Tcl autoload index file, version 2.0\n"
    append index "# This file is generated by the \"obtcl_mkindex\" command\n"
    append index "# and sourced to set up indexing information for one or\n"
    append index "# more commands/classes.  Typically each line is a command/class that\n"
    append index "# sets an element in the auto_index array, where the\n"
    append index "# element name is the name of a command/class and the value is\n"
    append index "# a script that loads the command/class.\n\n"
    foreach file [eval glob $args] {
	set f ""
	set error [catch {
	    set f [open $file]
	    while {[gets $f line] >= 0} {
		if [regexp {^(proc|class)[ 	]+([^ 	]*)} $line match dummy entityName] {
		    append index "set [list auto_index($entityName)]"
		    append index " \"source \$dir/$file\"\n"
		}
	    }
	    close $f
	} msg]
	if $error {
	    set code $errorCode
	    set info $errorInfo
	    catch {close $f}
	    cd $oldDir
	    error $msg $info $code
	}
    }
    set f [open tclIndex w]
    puts $f $index nonewline
    close $f
    cd $oldDir
}

