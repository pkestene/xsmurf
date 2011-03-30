#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#

# Command to find where a command is defined.
#
proc whereiscmd name {
    global auto_path
    global dev_dir
    set c_dir "interpreter interface"
    set is_found no

    foreach dir $auto_path {
	catch {exec grep ($name) $dir/tclIndex} result
	if {$result != "child process exited abnormally"} {
	    puts $dir
	    puts $result
	    set is_found yes
	}
    }

    foreach dir $c_dir {
	set search_dir $dev_dir/$dir
	catch {exec grep \"$name\" [glob $search_dir/*nit.c]} result
	if {$result != "child process exited abnormally"} {
	    puts $search_dir
	    puts $result
	    set is_found yes
	}
    }

    if {$is_found == "no"} {
	puts "$name not found."
    }
}