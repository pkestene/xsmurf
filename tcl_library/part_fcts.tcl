# part_fcts.tcl --
#
#       This file implements the Tcl code to handle partition functions.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: part_fcts.tcl,v 1.13 1998/04/22 16:47:53 decoster Exp $
#


# sw_fqaqtq --
# usage : sw_fqaqtq str float int int list str [args]
#
#  Create partition functions files sw can read with the commands qfile, qget,
# etc. These files are create from a serie of ext image from a wavelet
# transform.
#
# Parameters :
#   string  - base name of the ext images.
#   float   - first scale of the WT.
#   integer - number of octaves.
#   integer - number of voices for each octave.
#   list    - values of q for the partition functions.
#   string  - base name of the resulting files (i.e. name to use in the sw
#             command 'qfile').
#   args    - any option to pass to command efct.
#
# Return value :
#   None.

# State : Exp.
proc sw_fqaqtq {name a_min n_oct n_vox q_lst res args} {
    # Create general file.
    set res_file [open $res w]
    puts $res_file "noct $n_oct"
    puts $res_file "nvoice $n_vox"
    puts $res_file "wavelet Gauss_dx_dy"
    puts $res_file "amin $a_min"
    foreach q $q_lst {
	puts $res_file $q
    }
    close $res_file

    # Create history file.
    set hist_file [open ${res}_history w]
    puts $hist_file "***Computation on a new signal:"
    puts $hist_file "q Values   \[nb = [llength $q_lst]\]:"
    foreach q $q_lst {
	puts -nonewline $hist_file "$q "
    }
    puts $hist_file ""
    puts $hist_file "Signal size : [expr $n_oct*$n_vox]"
    puts $hist_file "Signal name : Gauss_dx_dy"
    puts $hist_file ""
    close $hist_file


    # Create data files.
    foreach q $q_lst {
	set q_str [get_q_str $q]

	catch {exec rm ${res}_Pq.$q_str}
	set Pq_file [open ${res}_Pq.$q_str a+]
	catch {exec rm ${res}_logsP.$q_str}
	set logsP_file [open ${res}_logsP.$q_str a+]
	catch {exec rm ${res}_PqlogP.$q_str}
	set PqlogP_file [open ${res}_PqlogP.$q_str a+]
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		set sPq [eval efct ${name}${new_num} abs(x)^y $q $args]
		puts $Pq_file $sPq
		puts $logsP_file [expr ($sPq?log($sPq):0)]
		puts $PqlogP_file [eval efct ${name}${new_num} log(abs(x))*abs(x)^y $q $args]
	    }
	}
	close $Pq_file
	close $logsP_file
	close $PqlogP_file
    }
    return
}

#  Add values to files sw can read with the qfile, qget, etc commands.
# NAME is the base name of the ext images. RES is the base name of the
# resulting files (i.e. name to use in the sw command 'qfile').
#

# sw_add_fqaqtq --
# usage : sw_fqaqtq str float int int list str [args]
#
#  Add values to (or create) partition functions files sw can read with the
# commands qfile, qget, etc. These files are (were) created from a serie of ext
# image from a wavelet transform.
#
# Parameters :
#   string  - base name of the ext images.
#   float   - first scale of the WT.
#   integer - number of octaves.
#   integer - number of voices for each octave.
#   list    - values of q for the partition functions.
#   string  - base name of the resulting files (i.e. name to use in the sw
#             command 'qfile').
#   args    - any option to pass to command efct.
#
# Warning :
#   If the history file doesn't exists, new files are created.
#   If it exists, the other files MUST exist. There is no check.
#
# Return value :
#   None.

# State : Exp.
proc sw_add_fqaqtq {name a_min n_oct n_vox q_lst res args} {
    if {[file exists ${res}_history] == 0} {
	eval sw_fqaqtq $name $a_min $n_oct $n_vox {$q_lst} $res $args
	return
    }


    # Update history file.
    set hist_file [open ${res}_history a+]
    puts $hist_file "***Computation on a new signal:"
    puts $hist_file "q Values   \[nb = [llength $q_lst]\]:"
    foreach q $q_lst {
	puts -nonewline $hist_file "$q "
    }
    puts $hist_file ""
    puts $hist_file "Signal size : [expr $n_oct*$n_vox]"
    puts $hist_file "Signal name : Gauss_dx_dy"
    puts $hist_file ""
    close $hist_file


    # Update data files.
    foreach q $q_lst {
	set q_str [get_q_str $q]

	# Read existing data.
	set Pq_file [open ${res}_Pq.$q_str r]
	set logsP_file [open ${res}_logsP.$q_str r]
	set PqlogP_file [open ${res}_PqlogP.$q_str r]
	catch {unset Pq_lst}
	catch {unset logsP_lst}
	catch {unset PqlogP_lst}
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		lappend Pq_lst [gets $Pq_file]
		lappend logsP_lst [gets $logsP_file]
		lappend PqlogP_lst [gets $PqlogP_file]
	    }
	}
	close $Pq_file
	close $logsP_file
	close $PqlogP_file

	# Add the new data.
	catch {exec rm ${res}_Pq.$q_str}
	set Pq_file [open ${res}_Pq.$q_str a+]
	catch {exec rm ${res}_logsP.$q_str}
	set logsP_file [open ${res}_logsP.$q_str a+]
	catch {exec rm ${res}_PqlogP.$q_str}
	set PqlogP_file [open ${res}_PqlogP.$q_str a+]
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		set sPq [expr [eval efct ${name}${new_num} abs(x)^y $q $args]+[lindex $Pq_lst $num]]
		puts $Pq_file $sPq
		puts $logsP_file [expr ($sPq?log($sPq):0)+[lindex $logsP_lst $num]]
		puts $PqlogP_file [expr [eval efct ${name}${new_num} log(abs(x))*abs(x)^y $q $args]+[lindex $PqlogP_lst $num]]
	    }
	}
	close $Pq_file
	close $logsP_file
	close $PqlogP_file
    }
}

# Creates (or update) signals for each value of q that contains the Tq and so
# on. NAME is the base name of the ext images. RES is the base name of the
# resulting signals.
#
# State : Exp.
#
proc sm_fqaqtq {name a_min n_oct n_vox q_lst res args} {
    foreach q $q_lst {
	set q_str [get_q_str $q]

	# Compute all the lists of points for the signals.
	catch {unset STq_lst}
	catch {unset STq_a_lst}
	catch {unset logSTq_lst}
	catch {unset STqlogT_lst}
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		set STq [eval "efct ${name}${new_num} abs(x)^y $q $args"]
		lappend STq_lst $STq
		lappend logSTq_lst [expr ($STq?log($STq):0)]
		lappend STqlogT_lst [eval "efct ${name}${new_num} log(abs(x))*abs(x)^y $q $args"]
		flush stdout
	    }
	}

	# Add the lists to signals.
	if {[string compare [gettype ${res}_STq_$q_str] S]} {
	    screate ${res}_STq_$q_str 1 [expr 1.0/$n_vox] $STq_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $STq_lst
	    sadd ${res}_STq_$q_str __tmp ${res}_STq_$q_str
	}

	if {[string compare [gettype ${res}_STqlogT_$q_str] S]} {
	    screate ${res}_STqlogT_$q_str 1 [expr 1.0/$n_vox] $STqlogT_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $STqlogT_lst
	    sadd ${res}_STqlogT_$q_str __tmp ${res}_STqlogT_$q_str
	}

	if {[string compare [gettype ${res}_logSTq_$q_str] S]} {
	    screate ${res}_logSTq_$q_str 1 [expr 1.0/$n_vox] $logSTq_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $logSTq_lst
	    sadd ${res}_logSTq_$q_str __tmp ${res}_logSTq_$q_str
	}
	catch {delete __tmp}
    }
}

# State : Exp.
#
proc part_load {q_lst name {options ""}} {
    foreach q $q_lst {
	set q_str [get_q_str $q]
	sload ${name}$q_str ${name}$q_str $options
    }
}

# State : Exp.
#
proc part_save {q_lst name {options ""}} {
    foreach q $q_lst {
	set q_str [get_q_str $q]
	ssave ${name}$q_str $options
    }
}

# sw_part_load --
# usage : sw_part_load str
#
#  Read partition functions files in the sw format and create signals for each
# partition functions (Pq, logsP and PqlogsP).
#
# Parameters :
#   string - base name of the files (i.e. name used in the sw command 'qfile').
#
# Return value :
#   A list with the parameters of the wavelet transform : first scale, number
#   of octaves, number of voices and the list of the values of q.

# State : Exp.
proc sw_part_load {name} {
    # Read general file.
    set res_file [open $name r]
    set n_oct [lindex [gets $res_file] 1]
    set n_vox [lindex [gets $res_file] 1]
    gets $res_file
    set a_min [lindex [gets $res_file] 1]
    gets $res_file q
    while {$q != ""} {
	lappend q_lst $q
	gets $res_file q
    }
    close $res_file

    foreach q $q_lst {
	set q_str [get_q_str $q]
	# Read existing data.
	set Pq_file [open ${name}_Pq.$q_str r]
	set logsP_file [open ${name}_logsP.$q_str r]
	set PqlogP_file [open ${name}_PqlogP.$q_str r]
	catch {unset Pq_lst}
	catch {unset logsP_lst}
	catch {unset PqlogP_lst}
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		lappend Pq_lst [gets $Pq_file]
		set val [gets $logsP_file]
		if {[string compare $val "--.-00000e+01"] == 0} {
		    set val 0
		}
		lappend logsP_lst $val
		lappend PqlogP_lst [gets $PqlogP_file]
	    }
	}

	close $Pq_file
	close $logsP_file
	close $PqlogP_file
	screate ${name}_Pq_$q_str 1 [expr 1.0/$n_vox] $Pq_lst
	screate ${name}_logsP_$q_str 1 [expr 1.0/$n_vox] $logsP_lst
	screate ${name}_PqlogP_$q_str 1 [expr 1.0/$n_vox] $PqlogP_lst
    }
    return "$a_min $n_oct $n_vox {$q_lst}"
}

# thd --
# usage : thd str list
#
#  Creates signals that contains tau(a), h(a) and D(a) for each value of q. Pq,
# logPq and PqlogP signals must have been loaded previously.
#
# Parameters :
#   string - base name of the signals.
#   list   - list of the values of q.
#
# Return value :
#   None.

# State : Exp.
proc thd {name q_lst} {
    foreach q $q_lst {
	set q_str [get_q_str $q]

	set STq     ${name}_Pq_$q_str
	set logSTq  ${name}_logPq_$q_str
	set STqlogT ${name}_PqlogP_$q_str

	scomb $STq $STq     log(x)/log(2)              ${name}_tau$q_str -xnull
	scomb $STqlogT $STq x/(log(2)*y)               ${name}_h$q_str   -ynull
	scomb $STqlogT $STq ($q*x-y*log(y))/(y*log(2)) ${name}_D$q_str   -ynull
    }
    return
}

proc oldthd {name a_min n_oct n_vox q_lst res args} {
    eval sm_fqaqtq $name $a_min $n_oct $n_vox $q_lst $res $args
    foreach q $q_lst {
	set q_str [get_q_str $q]

	set STq     ${res}_STq_$q_str
	set logSTq  ${res}_logSTq_$q_str
	set STqlogT ${res}_STqlogT_$q_str
	set STq_a   ${res}_STq_a_$q_str

	scomb $STq $STq     log(x)/log(2)              ${res}_tau$q_str
	scomb $STqlogT $STq x/(log(2)*y)               ${res}_h$q_str
	scomb $STqlogT $STq ($q*x-y*log(y))/(y*log(2)) ${res}_D$q_str
    }
}

# tq --
# Creates a signal that contains tau(q).

proc tq {a_min a_max q_lst name} {
    catch {unset fit_lst}
    foreach q $q_lst {
	set q_str [get_q_str $q]

	set fit [sfit ${name}_tau$q_str $a_min $a_max]
	lappend fit_lst [lindex $fit 0]
    }
    screate ${name}_tq [lindex $q_lst 0] 1 $fit_lst -xy $q_lst
}

# Dq --
# Creates a signal that contains D(q).

proc Dq {a_min a_max q_lst name} {
    catch {unset D_fit_lst}
    foreach q $q_lst {
	dputs "q = $q."
	set q_str [get_q_str $q]

	set D_fit [sfit ${name}_D$q_str $a_min $a_max]
	lappend D_fit_lst [expr [lindex $D_fit 0]+1]
    }
    screate ${name}_Dq [lindex $q_lst 0] 1 $D_fit_lst
}

# hq --
# Creates a signal that contains h(q).

proc hq {a_min a_max q_lst name} {
    catch {unset h_fit_lst}
    foreach q $q_lst {
	dputs "q = $q."
	set q_str [get_q_str $q]

	set h_fit [sfit ${name}_h$q_str $a_min $a_max]
	lappend h_fit_lst [lindex $h_fit 0]
    }
    screate ${name}_hq [lindex $q_lst 0] 1 $h_fit_lst
}

# Displays the NAME signals for all the values of q (m1p00, 0p00, 1p00, etc).
#
proc paff {name} {
    eval "nsaff [lsort -decreasing [ginfo ${name}m* -list]] [lsort [ginfo ${name}?p* -list]]"
}

#
# Partition functions for images.
#

# isw_fqaqtq_one --
#  Create files sw can read with the qfile, qget, etc commands.
# NAME is the base name of the ext images. RES is the base name of the
# resulting files (i.e. name to use in the sw command 'qfile').
#
# State : Exp.
#
proc isw_fqaqtq_one {name a_min n_oct n_vox index q_lst res args} {

    # Create general file.
    set res_file [open $res w]
    puts $res_file "noct $n_oct"
    puts $res_file "nvoice $n_vox"
    puts $res_file "wavelet Gauss_dx_dy"
    puts $res_file "amin $a_min"
    foreach q $q_lst {
	puts $res_file $q
    }
    close $res_file

    # Create history file.
    set hist_file [open ${res}_history w]
    puts $hist_file "***Computation on a new signal:"
    puts $hist_file "q Values   \[nb = [llength $q_lst]\]:"
    foreach q $q_lst {
	puts -nonewline $hist_file "$q "
    }
    puts $hist_file ""
    puts $hist_file "Signal size : [expr $n_oct*$n_vox]"
    puts $hist_file "Signal name : Gauss_dx_dy"
    puts $hist_file ""
    close $hist_file


    # Create data files.
    foreach q $q_lst {
	set q_str [get_q_str $q]

	if {$index == 0} {
	    catch {exec rm ${res}_Pq.$q_str}
	    catch {exec rm ${res}_logsP.$q_str}
	    catch {exec rm ${res}_PqlogP.$q_str}
	}
	set Pq_file [open ${res}_Pq.$q_str a+]
	set logsP_file [open ${res}_logsP.$q_str a+]
	set PqlogP_file [open ${res}_PqlogP.$q_str a+]

	set new_num [format "%.3d" $index]
	set sPq [eval ifct ${name}${new_num} abs(x)^y $q $args -domain_out -1e-8 1e-8]
	puts $Pq_file $sPq
	puts $logsP_file [expr ($sPq?log($sPq):0)]
	puts $PqlogP_file [eval ifct ${name}${new_num} log(abs(x))*abs(x)^y $q $args -domain_out -1e-8 1e-8]

	close $Pq_file
	close $logsP_file
	close $PqlogP_file
    }
}

# isw_add_fqaqtq_one --
#  Add values to files sw can read with the qfile, qget, etc commands.
# NAME is the base name of the ext images. RES is the base name of the
# resulting files (i.e. name to use in the sw command 'qfile').
#
# WARNINGS :
#    If the file ${res}_history doesn't exists, new files are created.
#    If it exists, the other files MUST exist. There is no check.
#
# State : Exp.
#
proc isw_add_fqaqtq_one {name a_min n_oct n_vox index q_lst res args} {
    if {[file exists ${res}_history] == 0} {
	eval isw_fqaqtq_one $name $a_min $n_oct $n_vox $index {$q_lst} $res $args
	return
    }

    if {$index == 0} {
	# Update history file.
	set hist_file [open ${res}_history a+]
	puts $hist_file "***Computation on a new signal:"
	puts $hist_file "q Values   \[nb = [llength $q_lst]\]:"
	foreach q $q_lst {
	    puts -nonewline $hist_file "$q "
	}
	puts $hist_file ""
	puts $hist_file "Signal size : [expr $n_oct*$n_vox]"
	puts $hist_file "Signal name : Gauss_dx_dy"
	puts $hist_file ""
	close $hist_file
    }

    # Update data files.
    foreach q $q_lst {
	set q_str [get_q_str $q]

	# Read existing data.
	set Pq_file [open ${res}_Pq.$q_str r]
	set logsP_file [open ${res}_logsP.$q_str r]
	set PqlogP_file [open ${res}_PqlogP.$q_str r]
	catch {unset Pq_lst}
	catch {unset logsP_lst}
	catch {unset PqlogP_lst}
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		set Pq [gets $Pq_file]
		if {$Pq != ""} {
		    lappend Pq_lst $Pq
		} else {
		    lappend Pq_lst 0
		}

		set logsP [gets $logsP_file]
		if {$logsP != ""} {
		    lappend logsP_lst $logsP
		} else {
		    lappend logsP_lst 0
		}

		set PqlogP [gets $PqlogP_file]
		if {$PqlogP != ""} {
		    lappend PqlogP_lst $PqlogP
		} else {
		    lappend PqlogP_lst 0
		}
	    }
	}
	close $Pq_file
	close $logsP_file
	close $PqlogP_file

	# Add the new data.
	catch {exec rm ${res}_Pq.$q_str}
	set Pq_file [open ${res}_Pq.$q_str a+]
	catch {exec rm ${res}_logsP.$q_str}
	set logsP_file [open ${res}_logsP.$q_str a+]
	catch {exec rm ${res}_PqlogP.$q_str}
	set PqlogP_file [open ${res}_PqlogP.$q_str a+]
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		if {$index == $num} {
		    set sPq [expr [eval ifct ${name}${new_num} abs(x)^y $q $args -domain_out -1e-8 1e-8]+[lindex $Pq_lst $num]]
		    puts $Pq_file $sPq
		    puts $logsP_file [expr ($sPq?log($sPq):0)+[lindex $logsP_lst $num]]
		    puts $PqlogP_file [expr [eval ifct ${name}${new_num} log(abs(x))*abs(x)^y $q $args -domain_out -1e-8 1e-8]+[lindex $PqlogP_lst $num]]
		} else {
		    puts $Pq_file [lindex $Pq_lst $num]
		    puts $logsP_file [lindex $logsP_lst $num]
		    puts $PqlogP_file [lindex $PqlogP_lst $num]
		}
	    }
	}
	close $Pq_file
	close $logsP_file
	close $PqlogP_file
    }
}

# Creates (or update) signals for each value of q that contains the Tq and so
# on. NAME is the base name of the ext images. RES is the base name of the
# resulting signals.
#
# State : Exp.
#
proc ism_fqaqtq {name a_min n_oct n_vox q_lst res args} {
    foreach q $q_lst {
	dputs "q = $q"
	set q_str [get_q_str $q]

	# Compute all the lists of points for the signals.
	catch {unset STq_lst}
	catch {unset STq_a_lst}
	catch {unset logSTq_lst}
	catch {unset STqlogT_lst}
	for { set oct 0;set num 0}\
		{ $oct < $n_oct} \
		{ incr oct } {
	    for {set vox 0} \
		    { $vox < $n_vox } \
		    { incr vox ; incr num} {
		set new_num [format "%.3d" $num]
		set STq [eval ifct ${name}${new_num} abs(x)^y $q $args -domain_out -1e-8 1e-8]
		lappend STq_lst $STq
		lappend logSTq_lst [expr ($STq?log($STq):0)]
		lappend STqlogT_lst [eval ifct ${name}${new_num} log(abs(x))*abs(x)^y $q $args -domain_out -1e-8 1e-8]
		puts -nonewline "."
		flush stdout
	    }
	}

	# Add the lists to signals.
	if {[string compare [gettype ${res}_STq_$q_str] S]} {
	    screate ${res}_STq_$q_str 1 [expr 1.0/$n_vox] $STq_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $STq_lst
	    sadd ${res}_STq_$q_str __tmp ${res}_STq_$q_str
	}

	if {[string compare [gettype ${res}_STqlogT_$q_str] S]} {
	    screate ${res}_STqlogT_$q_str 1 [expr 1.0/$n_vox] $STqlogT_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $STqlogT_lst
	    sadd ${res}_STqlogT_$q_str __tmp ${res}_STqlogT_$q_str
	}

	if {[string compare [gettype ${res}_logSTq_$q_str] S]} {
	    screate ${res}_logSTq_$q_str 1 [expr 1.0/$n_vox] $logSTq_lst
	} else {
	    screate __tmp 1 [expr 1.0/$n_vox] $logSTq_lst
	    sadd ${res}_logSTq_$q_str __tmp ${res}_logSTq_$q_str
	}
	puts ""
	catch {delete __tmp}
    }
}


#  Creates signals for each value of q that contains tau(a), h(a) and D(a).
# NAME is the base name of the ext images. RES is the base name of the resulting
# signals.
#
# State : Exp.
#
proc ithd {name a_min n_oct n_vox q_lst res args} {
    eval ism_fqaqtq $name $a_min $n_oct $n_vox $q_lst $res $args
    foreach q $q_lst {
	set q_str [get_q_str $q]

	set STq     ${res}_STq_$q_str
	set logSTq  ${res}_logSTq_$q_str
	set STqlogT ${res}_STqlogT_$q_str
	set STq_a   ${res}_STq_a_$q_str

	scomb $STq $STq     log(x)/log(2)              ${res}_tau$q_str
	scomb $STqlogT $STq x/(log(2)*y)               ${res}_h$q_str
	scomb $STqlogT $STq ($q*x-y*log(y))/(y*log(2)) ${res}_D$q_str
    }
}

# Displays the NAME signals for all the values of q (0p00, 1p00, etc).
# but not negative values of q.
#
proc ipaff {name} {
    eval "nsaff [lsort -decreasing [lsort [ginfo ${name}?p* -list]]]"
}

# get_q_str --
# usage : get_q_str float
#
#  Create a q-formated string from a float.
#
# Parameters :
#   float - value to format.
#
# Return value :
#   The string.
#
# Examples :
#   %get_q_str 1.2
#   1p20
#   %get_q_str -10.92
#   m10p92

proc get_q_str {q} {
    if {[file extension $q] != ""} {
	set q_str [expr int(abs($q))]p[format "%.2d" [expr int(100*[file extension $q])]]
    } else {
	set q_str [expr int(abs($q))]p00
    }
    if {$q < 0} {
	set q_str m$q_str
    }
    return $q_str
}

proc oldpdisp {name} {
    set tau_lst "[lsort -decreasing [ginfo ${name}_taum* -list]] [lsort [ginfo ${name}_tau?p* -list]]"
    set h_lst "[lsort -decreasing [ginfo ${name}_hm* -list]] [lsort [ginfo ${name}_h?p* -list]]"
    set D_lst "[lsort -decreasing [ginfo ${name}_Dm* -list]] [lsort [ginfo ${name}_D?p* -list]]"
    set code [catch {eval "mdisp 2 2 {{$tau_lst} {$h_lst} {$D_lst}}"} result]
    if {$code != 0} {
	error $result $result
    }
    return $result
}

# pdisp --
# usage : pdisp str [list]
#
#  Display part functions (tau(a,q), h(a,q), and D(a,q)).
#
# Parameters :
#   string - base name of the part functions.
#   list   - list of the values of q to display. The default is all the values.
#
# Return value :
#   Name of the object that handle the window.

proc pdisp {name {q_lst ""}} {
    set completeLst {}
    if {$q_lst == ""} {
	return opdisp $name
    } else {
	set sigLst {}
	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${name}_tau${q_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${name}_h${q_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${name}_D${q_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
    }

    set code [catch {mdisp 2 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }
    $result setColorsByList {darkgreen green darkcyan darkblue slateblue darkviolet violet}
    set itemList {}
    foreach value $q_lst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $result setLabelsItemsByList $itemList
    ${result}gr0000 set_label {black "tau(a), q = "} allSigLabel
    ${result}gr0100 set_label {black "D(a), q = "} allSigLabel
    ${result}gr0001 set_label {black "h(a), q = "} allSigLabel
    return $result
}
