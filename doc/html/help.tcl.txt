# help.tcl --
#
#       This file implements the Tcl code for help utilities.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#   Copyright 1999-2007 CEA/DSM/DAPNIA/SEDI
#   Modified by Pierre Kestener.
#

# help --
# usage : help [str]
#
#  This procedure manage the light help utility for smurf.
#
# Parameters :
#   string - The name of a command. This can be a C-defined command or a
#            Tcl-defined procedure.
#
# Return value :
#   A help message on the command. The definition file is return if the command
# is a Tcl-defined procedure. If no command is specified, a message  with the
# list of C-defined commands is return.

proc help {{cmdName ""}} {
    global auto_index

    if {$cmdName != ""} {
	# Help an a given command.
	set code [catch "set auto_index($cmdName)" result]
	if {$code} {
	    set code [catch "set auto_index(::$cmdName)" result]
	}
	if {$code} {
	    # cmdName is not a Tcl-defined command.
	    if {[info commands $cmdName] != ""} {
		# cmdName is in the Tcl command hashtable.

		# Set the default message.
		set resStr "\"$cmdName\" is a Tcl or a Tk command. See Tcl/Tk documentation."
		set allListOfCmds [cmdlist]
		lappend allListOfCmds [cv1d cmdlist]
		lappend allListOfCmds [cv2d cmdlist]
		foreach listOfCmds ${allListOfCmds} {
		    if {[lsearch $listOfCmds $cmdName] != -1} {
			# cmdName is in a smurf command hashtable. So we change
			# the message
			set resStr [$cmdName -help]
		    }
		}
	    } else {
		# cmdName is not in the Tcl command hashtable.
		set resStr "invalid command name \"$cmdName\""
	    }
	} else {
	    # cmdName is a Tcl-defined command.
	    set fileName [lindex $result 1]
	    
	    # Set the default message.

	    set resStr "Sorry, no help for `$cmdName'."

	    # Remove leading "::" corresponding to package.

	    regsub ^:: $cmdName {} cmdName
	    
	    # Scan all the file to find help comment on the command.
	    set fileId [open $fileName r]
	    while {[gets $fileId line] != -1} {
		if {[string compare $line "# $cmdName --"] == 0} {
		    # header line of help comment is found.
		    unset resStr
		    while {[gets $fileId line] != -1} {
			if {[regexp {^# *} $line]} {
			    # the line begins by "#" -> it is a help comment line.
			    # Remove the leading "#" ...
			    regsub {^# } $line "" line
			    regsub {^#} $line "" line
			    # ... and add to result
			    if {[info exist resStr] == 1} {
				set resStr [format "%s\n%s" $resStr "$line"]
			    } else {
				set resStr $line
			    }
			} else {
			    # No more leading "#" -> end of help comment.
			    break
			}
		    }
		    if {[info exist resStr] != 1} {
			# There was no line beginning with "#" after the
			# header line of the help comment.
			set resStr "Sorry, no help for `$cmdName'."
		    }		    
		    # Exit the loop on file lines.
		    break
		}
	    }
	    close $fileId
	    set resStr [format "%s\n\n(file : %s)" $resStr $fileName]
	}
    } else {
	# No command name -> need a general help message.

	# General help on C-defined commands.
	set resStr ""
	set resStr "${resStr}* Soft help for smurf. *\n\n"

	set resStr "${resStr}List of C-defined commands :\n\n"
	set allListOfCmds [cmdlist]
	lappend allListOfCmds [cv1d cmdlist]
	lappend allListOfCmds [cv2d cmdlist]
	set namesList [list \
		"general commands (see in XSMURFDIR/tcl_library/*)"\
		"signal commands"\
		"stat commands"\
		"image commands"\
		"mathematical morphology commands (see also in XSMURFDIR/tcl_library/morphology_proc.tcl)"\
		"1d wavelet transform commands"\
		"2d wavelet transform commands"\
		"3d wavelet transform commands"\
		"files manipulation commands"\
		"1d convolution commands"\
		"2d convolution commands"]

	set index 0
	foreach listOfCmds ${allListOfCmds} {
	    set resStr "${resStr}* [lindex ${namesList} $index] :\n"
	    set resStr "${resStr}${listOfCmds}\n" 
	    set resStr "${resStr}\n"
	    incr index
	}

	set resStr "${resStr}For help on a command type :\n"
	set resStr "${resStr}help <command_name>\n"
	set resStr "${resStr}This displays usage and a \"possibly complete help message\" (that depends\n"
	set resStr "${resStr}on the command).\n"
    }
    return $resStr
}
