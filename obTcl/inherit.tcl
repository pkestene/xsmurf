#----------------------------------------------------------------------
# Method resolution and caching
#

proc otPrInherits {} {
	global _obTcl_Classes
	foreach i [array names _obTcl_Classes]\
		{puts "$i inherits from: [$i inherit]"}
}

proc otInherit { class args } {
	global _obTcl_Inherits

	if ![string compare "" $args] {
		return [set _obTcl_Inherits($class)]
	}
	if { [string compare "Base" $class] && [lsearch $args "Base"] == -1 } {
		set args [concat $args "Base"]
	}
	if [info exists _obTcl_Inherits($class)] {
		#
		# This class is not new, invalidate caches
		#
		otInvalidateCaches 0 $class [otClassInfoCached ${class}]
	} else {
		set _obTcl_Inherits($class) {}
	}
	set _obTcl_Inherits($class) $args
}

proc otInvalidateCaches { level class methods } {
	global _obTcl_CacheStop

	foreach i $methods {
		if ![string compare "unknown" $i] { set i "*" }
		set _obTcl_CacheStop($i) 1
	}
	if [array exists _obTcl_CacheStop] { otDoInvalidate }
}

# There is a catch on rename and unset since current build of tmp
# does not guarantee that each element is unique.

proc otDoInvalidate {} {
	global _obTcl_CacheStop _obTcl_Cached
	if ![array exists _obTcl_Cached] {
		unset _obTcl_CacheStop
		return
	}
	if [info exists _obTcl_CacheStop(*)] {
		set stoplist "*"
	} else {
		set stoplist [array names _obTcl_CacheStop]
	}
	foreach i $stoplist {
		set tmp [array names _obTcl_Cached *..$i]
		eval lappend tmp [array names _obTcl_Cached *..${i}_next]
		foreach k $tmp {
			catch {
				rename $k {}
				unset _obTcl_Cached($k)
			}
		}
	}
	if ![array size _obTcl_Cached] {
		unset _obTcl_Cached
	}
	unset _obTcl_CacheStop
}

if ![string compare "" [info procs otUnknown]] {
	rename unknown otUnknown
}

proc otResolve { class func } {
	return [otGetFunc 0 $class $func]
}

#----------------------------------------------------------------------
#
# `unknown' and `next' both create cache methods.
#
#----------------------------------------------------------------------
#
# unknown -
#	A missing function was found.  See if it can be resolved
#	from inheritance.
#
#	If function name does not follow the *..* pattern, call the normal
#	unknown handler.
#
#	Umethod is for use by the "unknown" method.  If the method is named
#	`unknown' it will have $method set to $Umethod (the invokers method
#	name).
#

setIfNew _obTcl_unknBarred() ""

proc unknown args {
	global _obTcl_unknBarred
	# Resolve inherited function calls
	#
	set name [lindex $args 0]
	if [string match *..* $name] {
		set tmp [split $name .]	
		set class [lindex $tmp 0]
		set func [join [lrange $tmp 2 end] .]

		set flist [otGetFunc 0 $class $func]
		if ![string compare "" $flist] {
			if [info exists _obTcl_unknBarred($name)] { return -code error }
			set flist [otGetFunc 0 $class "unknown"]
		}
		if [string compare "" $flist] {
			proc $name args "otGetSelf
set Umethod $func
eval [lindex $flist 0] \$args"
		} else {
			proc $name args "
				return -code error\
				-errorinfo \"Undefined method '$func' invoked\" \
					\"Undefined method '$func' invoked\"
			"
		}
		global _obTcl_Cached
		set _obTcl_Cached(${class}..$func) $class

		# Code below borrowed from init.tcl (tcl7.4)
		#
		global errorCode errorInfo
		set code [catch {uplevel $args} msg]
	    	if { $code == 1 } {
			#
			# Strip the last five lines off the error stack (they're
			# from the "uplevel" command).
			#
			set new [split $errorInfo \n]
			set new [join [lrange $new 0 [expr [llength $new] - 6]] \n]
			return -code error -errorcode $errorCode \
				-errorinfo $new $msg
	    	} else {
			return -code $code $msg
	    	}
	} else {
		uplevel [concat otUnknown $args]
	}
}

setIfNew _obTcl_Cnt 0

# 6/11/95 Added _obTcl_nextRet to allow propagation of return-values
#	from `next' calls.  I.e doing `return [next $args]' will
#	be meaningful.  It is only in simple cases that the return
#	value is shure to make sense.  With multiple inheritance
#	it may be impossible to rely on!
#
#	NOTE: This support is experimental and likely to be removed!!!
#
#	Improved for lower overhead with big args-lists
#	NOTE: It is understood that `args' is initialized from the `next'
#	procedure.
#
proc otChkCall { cmd } {
	global _obTcl_Trace _obTcl_Cnt _obTcl_nextRet
	if ![info exists _obTcl_Trace($cmd)] {
		set _obTcl_Trace($cmd) 1
		catch {uplevel 1 "uplevel 1 \"$cmd \$args\""} _obTcl_nextRet
	}
	return $_obTcl_nextRet
}

# otNextPrepare is really just a part of proc `next' below.
#
proc otNextPrepare {} {
	uplevel 1 {
		set all [otGetNextFunc $class $method]

		foreach i $all {
			# Note: args is the literal _name_ of var to use, hence
			#	no $-sign!
			append tmp "otChkCall $i\n"
		}

		if [info exists tmp] {
			proc $class..${method}_next args $tmp
		} else {
			proc $class..${method}_next args return
		}
		set _obTcl_Cached(${class}..${method}_next) $class
	}
}

# next -
# Invoke next shadowed method.  Protect against multiple invocation.
# Multiple invocation would occur when several inherited classes inherit
# a common superclass.
#
# Note: I use `info exists' on _obTcl_Cached, rater than `info procs' on
# the corresponding procedure, since checking for a variable seems to be
# about three times faster (Tcl7.4).
#
proc next args {
	global _obTcl_Cnt _obTcl_Cached _obTcl_nextRet
	# otGetSelf inlined and modified
	upvar 1 self self method method class class

	if { $_obTcl_Cnt == 0 } {
		set _obTcl_nextRet ""
	}
	if ![info exists _obTcl_Cached(${class}..${method}_next)] {
		otNextPrepare
	}

	incr _obTcl_Cnt 1
	set ret [catch {uplevel 1 {${class}..${method}_next} $args} val]
	incr _obTcl_Cnt -1

	if { $_obTcl_Cnt == 0 } {
		global _obTcl_Trace
		catch {unset _obTcl_Trace}
	}
	if { $ret != 0 } {
		return -code error \
		  -errorinfo "$self: $val" "$self: $val"
	} else {
		return $val
	}
}

# otGetNextFunc -
# Get a method by searching inherited classes, skipping the local
# class.
#
proc otGetNextFunc { class func } {
	global _obTcl_Inherits

	set all ""
	foreach i [set _obTcl_Inherits($class)] {
		foreach k [otGetFunc 0 $i $func] {
			lappendUniq all $k
		}
	}
	return $all
}

# otGetFunc -
# Locate a method by searching the inheritance tree.
# Cyclic inheritance is discovered and reported.  A list of all
# found methods is returned, with the closest first in the list.
# Cache-methods are skipped, and will hence not figure in the list.
#
# 16/12/95  Added support for autoloading of classes.
#
proc otGetFunc { depth class func } {
	global _obTcl_Inherits _obTcl_Cached _obTcl_NoClasses _obTcl_Classes

	if { $depth > $_obTcl_NoClasses } {
		otGetFuncErr $depth $class $func
		return ""
	}
	incr depth
	set all ""

	if ![info exists _obTcl_Classes($class)] {
		if ![auto_load $class] {
			otGetFuncMissingClass $depth $class $func
			return ""
		}
        }
	if { [string compare "" [info procs $class..$func]] &&
	     ![info exists _obTcl_Cached(${class}..$func)] } {
		return "$class..$func"
	}
	foreach i [set _obTcl_Inherits($class)] {
		set ret [otGetFunc $depth $i $func]
		if [string compare "" $ret] {
			foreach i $ret {
				lappendUniq all $i
			}
		}
	}
	return $all
}

# Note: Real error handling should be added here!
# Specifically we need to report which object triggered the error.

proc otGetFuncErr { depth class func } { 
	puts stderr "GetFunc: depth=$depth, circular dependency!?"
	puts stderr "         class=$class func=$func"
}
proc otGetFuncMissingClass { depth class func } { 
	puts stderr "GetFunc: Unable to inherit from $class"
	puts stderr "         $class not defined (and auto load failed)"
	puts stderr "         Occurred while looking for $class..$func"
}

