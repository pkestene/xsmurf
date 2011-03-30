# part_comp_fcts.tcl --
#
#       This file implements the Tcl code to handle complex partition functions.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: part_comp_fcts.tcl,v 1.1 1998/08/18 14:30:04 decoster Exp $
#


# sw_comp_fqaqtq --
# usage : sw_comp_fqaqtq str float int int list str [args]
#
#  Create coomplex partition functions files sw can read with the commands gget,
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
proc sw_comp_fqaqtq {name a_min n_oct n_vox q_lst res args} {
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
    set val [expr $n_vox/2]

    # Create data files.
    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    if {$vox == $val || $vox == 0} {
		set new_num [format "%.3d" $num]
		puts "scale=$new_num"
		catch {exec rm ${res}cosG_${new_num}}
		catch {exec rm ${res}sinG_${new_num}}
		set cos_file [open ${res}cosG_${new_num} a+]
		set sin_file [open ${res}sinG_${new_num} a+]
		foreach q $q_lst {
		    set cosPq [eval efct ${name}${new_num} cos(y*log(abs(x))) $q $args]
		    set sinPq [eval efct ${name}${new_num} sin(y*log(abs(x))) $q $args]
		    puts $cos_file $cosPq
		    puts $sin_file $sinPq
		}
		
		set nb_vc 0
		set nb_max 0
		foreache ${name}${new_num} {
		    incr nb_max 
		    if {[string compare $type "vc"] == 0} {
			incr nb_vc 
		    }
		}    
		if {[string compare $args "-vc"] == 0} {
		    puts $cos_file $nb_vc
		    puts $sin_file $nb_vc
		} else {
		    puts $cos_file $nb_max
		    puts $sin_file $nb_max
		}
		close $cos_file
		close $sin_file
		puts "scale=$new_num max=$nb_max vc=$nb_vc"
	    }
	}
    }
    return
}


# sw_comp_add_fqaqtq --
# usage : sw_comp_add_fqaqtq str float int int list str [args]
#
#  Add values to (or create) complex partition functions files sw can read with the
# commands getfg etc. These files are (were) created from a serie of ext
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

proc sw_comp_add_fqaqtq {name a_min n_oct n_vox res args} {
    set q_lst {}
    set q 0.0
    for {set i 0} {$i <= 500} {incr i} {
	set newq [format %.2f $q]
	set q_lst [lappend q_lst $newq]
	set q [expr $q+0.02]
    }

    if {[file exists ${res}_history] == 0} {
	eval sw_comp_fqaqtq $name $a_min $n_oct $n_vox {$q_lst} $res $args
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
    set val [expr $n_vox/2]

    # Update data files.
    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    if {$vox == $val || $vox == 0} {
		set new_num [format "%.3d" $num]
		puts "scale=$new_num"
		# Read existing data.
		set cos_file [open ${res}cosG_${new_num} r]
		set sin_file [open ${res}sinG_${new_num} r]
		
		catch {unset cos_lst}
		catch {unset sin_lst}
		
		foreach q $q_lst {	
		    lappend cos_lst [gets $cos_file]
		    lappend sin_lst [gets $sin_file]
		}
		lappend cos_lst [gets $cos_file]
		lappend sin_lst [gets $sin_file]

		close $cos_file
		close $sin_file
		
		# Add the new data.
		catch {exec rm ${res}cosG_${new_num}}
		set cos_file [open ${res}cosG_${new_num} a+]
		catch {exec rm ${res}sinG_${new_num}}
		set sin_file [open ${res}sinG_${new_num} a+]
		
		set index 0
		foreach q $q_lst {
		    set cosPq [expr [eval efct ${name}${new_num} cos(log(abs(x))*y) $q $args]+[lindex $cos_lst $index]]
		    set sinPq [expr [eval efct ${name}${new_num} sin(log(abs(x))*y) $q $args]+[lindex $sin_lst $index]]
		    puts $cos_file $cosPq
		    puts $sin_file $sinPq
		    incr index
		}
		
		set nb_vc [lindex $cos_lst $index]
		set nb_max [lindex $cos_lst $index]
		
		set nb_vcbis [lindex $cos_lst $index]
		
		if {$nb_vc != $nb_vcbis} {
		    error "problem with the number of max"
		}
		
		foreache ${name}${new_num} {
		    set nb_max [expr $nb_max+1]
		    if {[string compare $type "vc"] == 0} {
			set nb_vc [expr $nb_vc+1]
		    }
		}    
		if {[string compare $args "-vc"] == 0} {
		    puts $cos_file $nb_vc
		    puts $sin_file $nb_vc
		} else {
		    puts $cos_file $nb_max
		    puts $sin_file $nb_max
		}	    
		close $cos_file
		close $sin_file
		puts "scale=$new_num max=$nb_max vc=$nb_vc"
	    }
	}
    }
}

proc sw_comp_part_load_add {lstname name} {

    set name1 [lindex $lstname 0]

    # Read general file for the first name of the list
    set res_file [open $name1 r]
    set n_oct [lindex [gets $res_file] 1]
    set n_vox [lindex [gets $res_file] 1]
    gets $res_file
    set a_min [lindex [gets $res_file] 1]
    gets $res_file q
    while {$q != ""} {
	lappend q_lst $q
	gets $res_file q
    }
    puts $q
    close $res_file
    set val [expr $n_vox/2]


    foreach name2 [lrange $lstname 1 end] {
	
	# Read general file for the other name of the list
	set res_file [open $name2 r]
	set n_oct2 [lindex [gets $res_file] 1]
	set n_vox2 [lindex [gets $res_file] 1]
	gets $res_file
	set a_min2 [lindex [gets $res_file] 1]
	gets $res_file q

	catch {unset q_lst2}

	while {$q != ""} {
	    lappend q_lst2 $q
	    gets $res_file q
	}
	close $res_file
	set val2 [expr $n_vox/2]
	
	# Check consistency
	
	if {$n_oct != $n_oct2 } {
	    error "n_oct has different value for $name2"
	}
	if {$n_vox != $n_vox2 } {
	    error "n_vox has different value for $name2"
	}
	if {$a_min != $a_min2 } {
	    error "a_min has different value for $name2"
	}

	set ii 0
	foreach q1 $q_lst q2 $q_lst2 {
	    if {$q1 != $q2} {
		error "q ($q1 $q2) has different value for $name2"
	    }
	}
	if {$val != $val2 } {
	    error "val has different value for $name2"
	}
    }
    
    # Read the data 

    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    if {$vox == $val || $vox == 0} {
		set new_num [format "%.3d" $num]
		puts "scale=$new_num"

		# Read data of the first name.
		set cos_file [open ${name1}cosG_${new_num} r]
		set sin_file [open ${name1}sinG_${new_num} r]

		catch {unset cos_lst}
		catch {unset sin_lst}
		
		set num_of_pt 0
		foreach q $q_lst {
		    lappend cos_lst [gets $cos_file]
		    lappend sin_lst [gets $sin_file]
		    incr num_of_pt
		}
		set num_of_max [gets $cos_file]
		set num_of_max2 [gets $sin_file]
		
		if {$num_of_max != $num_of_max2} {
		    error "problem with the number of max"		
		}
		set num_of_pt [expr $num_of_pt-1]
		
		close $cos_file
		close $sin_file	
    

		foreach name2 [lrange $lstname 1 end] {
		    
		    # Read the data of the other files
		    set cos_file2 [open ${name2}cosG_${new_num} r]
		    set sin_file2 [open ${name2}sinG_${new_num} r]
		    
		    catch {unset cos_lst2}
		    catch {unset sin_lst2}
		    
		    set num_of_pt 0
		    foreach q $q_lst {
			lappend cos_lst2 [expr [gets $cos_file2] + [lindex $cos_lst $num_of_pt]]
			lappend sin_lst2 [expr [gets $sin_file2] + [lindex $sin_lst $num_of_pt]]
			incr num_of_pt
		    }
		    set num_of_max [expr $num_of_max + [gets $cos_file2]]
		    set num_of_max2 [expr $num_of_max2 + [gets $sin_file2]]
		    
		    if {$num_of_max != $num_of_max2} {
			error "problem with the number of max"		
		    }
		    
		    set num_of_pt [expr $num_of_pt-1]
		    
		    close $cos_file2
		    close $sin_file2
		    
		    catch {unset cos_lst}
		    catch {unset sin_lst}
		    
		    set cos_lst $cos_lst2
		    set sin_lst $sin_lst2
		}


		set newq_lst {}
		set newcos_lst {}
		set newsin_lst {}
		set index 0
		foreach q $q_lst {
		    set ii [expr $num_of_pt-$index]
		    set newq [expr -1.0*[lindex $q_lst $ii]]
		    set newq [format %.2f $newq]
		    set newcos [expr [lindex $cos_lst $ii]/$num_of_max]
		    set newsin [expr -1.0*[lindex $sin_lst $ii]/$num_of_max]
		    
		    if {$newq < 0} {
			set newq_lst [lappend newq_lst $newq]
			set newcos_lst [lappend newcos_lst $newcos]
			set newsin_lst [lappend newsin_lst $newsin]
		    }
		    
		    incr index 
		}
		
		set index 0
		foreach q $q_lst {
		    set newcos [expr [lindex $cos_lst $index]/$num_of_max]
		    set newsin [expr [lindex $sin_lst $index]/$num_of_max]
		    set newq_lst [lappend newq_lst $q]
		    set newcos_lst [lappend newcos_lst $newcos]
		    set newsin_lst [lappend newsin_lst $newsin]
		    incr index
		}
		
		screate ${name}cosG_${new_num} 0 1 $newcos_lst -xy $newq_lst
		screate ${name}sinG_${new_num} 0 1 $newsin_lst -xy $newq_lst
		
	    }
	}
    }

    return "$a_min $n_oct $n_vox "
}

	

proc sw_comp_part_merge_files {lstname destName} {

    set name1 [lindex $lstname 0]

    # Read general file for the first name of the list
    set res_file [open $name1 r]
    set n_oct [lindex [gets $res_file] 1]
    set n_vox [lindex [gets $res_file] 1]
    gets $res_file
    set a_min [lindex [gets $res_file] 1]
    gets $res_file q
    while {$q != ""} {
	lappend q_lst $q
	gets $res_file q
    }
    puts $q
    close $res_file
    set val [expr $n_vox/2]


    foreach name2 [lrange $lstname 1 end] {
	
	# Read general file for the other name of the list
	set res_file [open $name2 r]
	set n_oct2 [lindex [gets $res_file] 1]
	set n_vox2 [lindex [gets $res_file] 1]
	gets $res_file
	set a_min2 [lindex [gets $res_file] 1]
	gets $res_file q

	catch {unset q_lst2}

	while {$q != ""} {
	    lappend q_lst2 $q
	    gets $res_file q
	}
	close $res_file
	set val2 [expr $n_vox/2]
	
	# Check consistency
	
	if {$n_oct != $n_oct2 } {
	    error "n_oct has different value for $name2"
	}
	if {$n_vox != $n_vox2 } {
	    error "n_vox has different value for $name2"
	}
	if {$a_min != $a_min2 } {
	    error "a_min has different value for $name2"
	}

	set ii 0
	foreach q1 $q_lst q2 $q_lst2 {
	    if {$q1 != $q2} {
		error "q ($q1 $q2) has different value for $name2"
	    }
	}
	if {$val != $val2 } {
	    error "val has different value for $name2"
	}
    }
    
    # Read the data 

    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    if {$vox == $val || $vox == 0} {
		set new_num [format "%.3d" $num]

		# Read data of the first name.
		set cos_file [open ${name1}cosG_${new_num} r]
		set sin_file [open ${name1}sinG_${new_num} r]

		catch {unset cos_lst}
		catch {unset sin_lst}
		
		set num_of_pt 0
		foreach q $q_lst {
		    lappend cos_lst [gets $cos_file]
		    lappend sin_lst [gets $sin_file]
		    incr num_of_pt
		}
		set num_of_max [gets $cos_file]
		set num_of_max2 [gets $sin_file]
		
		if {$num_of_max != $num_of_max2} {
		    error "problem with the number of max"		
		}
		set num_of_pt [expr $num_of_pt-1]
		
		close $cos_file
		close $sin_file	
    

		foreach name2 [lrange $lstname 1 end] {
		    
		    # Read the data of the other files
		    set cos_file2 [open ${name2}cosG_${new_num} r]
		    set sin_file2 [open ${name2}sinG_${new_num} r]
		    
		    catch {unset cos_lst2}
		    catch {unset sin_lst2}
		    
		    set num_of_pt 0
		    foreach q $q_lst {
			lappend cos_lst2 [expr [gets $cos_file2] + [lindex $cos_lst $num_of_pt]]
			lappend sin_lst2 [expr [gets $sin_file2] + [lindex $sin_lst $num_of_pt]]
			incr num_of_pt
		    }
		    set num_of_max [expr $num_of_max + [gets $cos_file2]]
		    set num_of_max2 [expr $num_of_max2 + [gets $sin_file2]]
		    
		    if {$num_of_max != $num_of_max2} {
			error "problem with the number of max"		
		    }
		    
		    set num_of_pt [expr $num_of_pt-1]
		    
		    close $cos_file2
		    close $sin_file2
		    
		    catch {unset cos_lst}
		    catch {unset sin_lst}
		    
		    set cos_lst $cos_lst2
		    set sin_lst $sin_lst2
		}

		# Write the data in destination file.
		set cos_file2 [open ${destName}cosG_${new_num} w]
		set sin_file2 [open ${destName}sinG_${new_num} w]
		
		foreach cos $cos_lst sin $sin_lst {
		    puts $cos_file2 $cos
		    puts $sin_file2 $sin
		}
		puts $cos_file2 $num_of_max
		puts $sin_file2 $num_of_max

		close $cos_file2
		close $sin_file2
	    }
	}
    }

    # Create general file.
    set res_file [open ${destName} w]
    puts $res_file "noct $n_oct"
    puts $res_file "nvoice $n_vox"
    puts $res_file "wavelet Gauss_dx_dy"
    puts $res_file "amin $a_min"
    foreach q $q_lst {
	puts $res_file $q
    }
    close $res_file

    # Create history file.
    set hist_file [open ${destName}_history w]
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

    return "$a_min $n_oct $n_vox "
}

	

# sw_comp_part_load --
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
proc sw_comp_part_load {name} {
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
    set val [expr $n_vox/2]

    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    if {$vox == $val || $vox == 0} {
		set new_num [format "%.3d" $num]
		puts "scale=$new_num"
		# Read existing data.
		set cos_file [open ${name}cosG_${new_num} r]
		set sin_file [open ${name}sinG_${new_num} r]
		
		catch {unset cos_lst}
		catch {unset sin_lst}
		
		set num_of_pt 0
		foreach q $q_lst {
		    lappend cos_lst [gets $cos_file]
		    lappend sin_lst [gets $sin_file]
		    incr num_of_pt
		}
		set num_of_max [gets $cos_file]
		set num_of_max2 [gets $sin_file]
		
		#	    puts "max=$num_of_max"
		if {$num_of_max != $num_of_max2} {
		    error "problem with the number of max"		
		}
		set num_of_pt [expr $num_of_pt-1]
		
		close $cos_file
		close $sin_file
		
		set newq_lst {}
		set newcos_lst {}
		set newsin_lst {}
		
		set index 0
		foreach q $q_lst {
		    set ii [expr $num_of_pt-$index]
		    set newq [expr -1.0*[lindex $q_lst $ii]]
		    set newq [format %.2f $newq]
		    set newcos [expr [lindex $cos_lst $ii]/$num_of_max]
		    set newsin [expr -1.0*[lindex $sin_lst $ii]/$num_of_max]
		    
		    if {$newq < 0} {
			#		    puts "$newq  $newcos $newsin\n"
			set newq_lst [lappend newq_lst $newq]
			set newcos_lst [lappend newcos_lst $newcos]
			set newsin_lst [lappend newsin_lst $newsin]
		    }
		    
		    incr index 
		}

		set index 0
		foreach q $q_lst {
		    #		puts "$newq  $newcos $newsin\n"
		    set newcos [expr [lindex $cos_lst $index]/$num_of_max]
		    set newsin [expr [lindex $sin_lst $index]/$num_of_max]
		    set newq_lst [lappend newq_lst $q]
		    set newcos_lst [lappend newcos_lst $newcos]
		    set newsin_lst [lappend newsin_lst $newsin]
		    incr index
		}
		
		#	    screate ${name}cosGbis_${new_num} 0 1 $cos_lst -xy $q_lst
		#	    screate ${name}sinGbis_${new_num} 0 1 $sin_lst -xy $q_lst
		screate ${name}cosG_${new_num} 0 1 $newcos_lst -xy $newq_lst
		screate ${name}sinG_${new_num} 0 1 $newsin_lst -xy $newq_lst
		#	    puts "max=$num_of_max"
	    }
	}
    }
    return "$a_min $n_oct $n_vox "
}

proc getfg {name a1 a2} {
    set new_a1 [format "%.3d" $a1]
    set new_a2 [format "%.3d" $a2]
    scomb ${name}cosG_${new_a2} ${name}sinG_${new_a2} x*x+y*y modG
    
    scomb ${name}cosG_${new_a1} ${name}cosG_${new_a2} x*y temp1
    scomb ${name}sinG_${new_a1} ${name}sinG_${new_a2} x*y temp2
    scomb temp1 temp2 x+y temp3
    scomb temp3 modG x/y ${name}reG${new_a1}_${new_a2}

    scopy temp1 c1c2
    scopy temp2 s1s2

    scopy temp3 c1c2ps1s2

    scomb ${name}sinG_${new_a1} ${name}cosG_${new_a2} x*y temp1
    scomb ${name}cosG_${new_a1} ${name}sinG_${new_a2} x*y temp2
    scomb temp1 temp2 x-y temp3
    scomb temp3 modG x/y ${name}imG${new_a1}_${new_a2}

    scopy temp1 s1c2
    scopy temp2 c1s2

    scopy temp3 s1c2mc1s2

    #delete temp1 temp2 temp3 modG
}

proc propag {name a1 a2} {
    set new_a1 [format "%.3d" $a1]
    set new_a2 [format "%.3d" $a2]
    getfg $name $a1 $a2
    scomb ${name}reG${new_a1}_${new_a2} ${name}imG${new_a1}_${new_a2} sqrt(x*x+y*y) ${name}modG${new_a1}_${new_a2} 
    scomb ${name}imG${new_a1}_${new_a2} ${name}reG${new_a1}_${new_a2} atan(x/y) ${name}phiG${new_a1}_${new_a2}
    rmdisc ${name}phiG${new_a1}_${new_a2} ${name}phiG${new_a1}_${new_a2}
    scopy ${name}reG${new_a1}_${new_a2} reG
    scopy ${name}imG${new_a1}_${new_a2} imG
    scopy ${name}modG${new_a1}_${new_a2} moG
}

proc nomsg args {
}

proc fitpropag {name a1 a2 xmin xmax {msg yes}} {

    if {[string compare $msg "yes"] == 0} {
	set msgCmd puts
    } else {
	set msgCmd nomsg
    }

    set new_a1 [format "%.3d" $a1]
    set new_a2 [format "%.3d" $a2]
    # First Method
    s2fs ${name}modG${new_a1}_${new_a2} temp1 x -1*log(y)
    sderiv temp1 temp2
    set res [sfit temp2 $xmin $xmax]
    set m1 [expr { [lindex $res 0]*1000.0/1000 }]
    set res [sfit ${name}phiG${new_a1}_${new_a2} $xmin $xmax]
    set m2 [expr { [lindex $res 0]*1000.0/1000 }]
    $msgCmd "First Method :  sigma=$m1 (modulus)\n                m=$m2 (argument)"

    #Second Method
    sthresh ${name}phiG${new_a1}_${new_a2} temp1 $xmin $xmax
    set res [sasymfit temp1 fittemp2]
    set m [lindex $res 0]
    set k3 [expr [lindex $res 1]*6]
    set k5 [lindex $res 2]
    s2fs ${name}modG${new_a1}_${new_a2} temp1 x log(y)
    sthresh temp1 temp2 $xmin $xmax
    set res [ssymfit temp2 fittemp2]
    set sigma [expr { -2*[lindex $res 0] }]
    set beta [lindex $res 1]
    set gamma [lindex $res 2]
    $msgCmd "Second Method : sigma=$sigma beta=$beta gamma=$gamma (modulus)\n                m=$m k3=$k3 k5=$k5 (argument)"
    delete temp*

    set a1_a2 [expr { pow(2,($a2-$a1)/10.0) }]

    return "$sigma $beta $gamma $m $k3 $k5 $m1 $m2 $a1_a2"
    
}

proc getbgl {sigma beta gamma m k3 k5 m1 m2 a1_a2} {
    set beta [expr { exp($k3/$sigma) / log($a1_a2)}]
    set lambda [expr { $sigma/(pow(log($beta),2)*pow(log($a1_a2),2)) }]
    set gamma [expr { $m/log($a1_a2) - $lambda*log($beta) }]

    return [list $beta $gamma $lambda]
}

proc showfitg {name a1 a2 sigma beta gamma m k3 k5} {
    set new_a1 [format "%.3d" $a1]
    set new_a2 [format "%.3d" $a2]
    s2fs ${name}modG${new_a1}_${new_a2} temp x log(y)
    s2fs ${name}modG${new_a1}_${new_a2} fitmodG x $sigma*x*x+$beta*x*x*x*x+$gamma*x*x*x*x*x*x
    s2fs ${name}phiG${new_a1}_${new_a2} fitphiG x $m*x+$k3*x*x*x+$k5*x*x*x*x*x
    set sig_lst1 {}
    set sig_lst2 {}
    set complete_lst {}
    set sig_lst1 [lappend sig_lst1 temp]
    set sig_lst1 [lappend sig_lst1 fitmodG]
    set sig_lst2 [lappend sig_lst2 ${name}phiG${new_a1}_${new_a2}]
    set sig_lst2 [lappend sig_lst2 fitphiG]
    set complete_lst [lappend complete_lst ${sig_lst1}]
    set complete_lst [lappend complete_lst ${sig_lst2}]

    set code [catch {mdisp 1 2 ${complete_lst}} result]
    ${result} setColorsByList {black red}
    set itemList {}
    foreach value {G fit} {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval ${result} setLabelsItemsByList $itemList
    ${result}gr0000 set_label [list black "log(Mod) scale : ${a1}_${a2} , "] allSigLabel
    ${result}gr0001 set_label [list black "Arg scale : ${a1}_${a2} , "] allSigLabel

    return $result
}


proc fitslopeG {name alpha num dep} {

    set xmax 2
    set xmin [expr -1.0*$xmax]

    set ln2 0.69314718
    set r2 1.41421356237
    set dx [expr $ln2/2.0]
    set taille [expr ($num-($dep-1))*2]

    set sig_sigma_lst {}
    set sig_beta_lst {}
    set sig_gammma_lst {}
    set sig_m_lst {}
    set sig_k3_lst {}
    set sig_k5_lst {}
    set sig_sigma_lst2 {}
    set sig_beta_lst2 {}
    set sig_gammma_lst2 {}
    set sig_m_lst2 {}
    set sig_k3_lst2 {}
    set sig_k5_lst2 {}

    for {set oref  0} {$oref <= $num} {incr oref} {
	set oref2 [expr 10*$oref]
	set x0 [expr ($dep-$oref)*$ln2]
	set aref [expr pow(2,$oref)]
	
	set a2alpha [expr pow($aref,$alpha)]
	
	set index 1

	set ax_lst {}
	set sigma_lst {}
	set beta_lst {}
	set gamma_lst {}
	set m_lst {}
	set k3_lst {}
	set k5_lst {}

	for {set oct $dep} {$oct <= $num} {incr oct} {
	    set acur [expr pow(2,$oct)]
	    for {set vo 0} {$vo <= 1} {incr vo} {
		set oct2 [expr 10*$oct]
		if {$vo == 1} {
		    set acur [expr $acur*$r2]
		    set oct2 [expr 10*$oct+5]
		}
		set a1alpha [expr pow($acur,$alpha)]
		set ax [expr ($a1alpha-$a2alpha)/$alpha]
		
		
		puts "oref=$oref2 oct=$oct2 v=$vo"
		propag $name $oct2 $oref2
		set res [fitpropag $name $oct2 $oref2 $xmin $xmax]
		set sigma [lindex $res 0]
		set beta [lindex $res 1]
		set gamma [lindex $res 2]
		set m [lindex $res 3]
		set k3 [lindex $res 4]
		set k5 [lindex $res 5]
		puts $res
		set sigma_lst [lappend sigma_lst $sigma]
		set beta_lst [lappend beta_lst $beta]
		set gamma_lst [lappend gamma_lst $gamma]
		set m_lst [lappend m_lst $m]
		set k3_lst [lappend k3_lst $k3]
		set k5_lst [lappend k5_lst $k5]
		set ax_lst [lappend ax_lst $ax]
	    }
	    
	}
	screate sigma_lna_$oref $x0 $dx $sigma_lst
	screate beta_lna_$oref $x0 $dx $beta_lst
	screate gamma_lna_$oref $x0 $dx $gamma_lst
	screate m_lna_$oref $x0 $dx $m_lst
	screate k3_lna_$oref $x0 $dx $k3_lst
	screate k5_lna_$oref $x0 $dx $k5_lst

	screate sigma_ah_$oref $x0 $dx $sigma_lst -xy $ax_lst
	screate beta_ah_$oref $x0 $dx $beta_lst -xy $ax_lst
	screate gamma_ah_$oref $x0 $dx $gamma_lst -xy $ax_lst
	screate m_ah_$oref $x0 $dx $m_lst -xy $ax_lst
	screate k3_ah_$oref $x0 $dx $k3_lst -xy $ax_lst
	screate k5_ah_$oref $x0 $dx $k5_lst -xy $ax_lst

	set sig_sigma_lst [lappend sig_sigma_lst sigma_lna_$oref]
	set sig_beta_lst [lappend sig_beta_lst beta_lna_$oref]
	set sig_gamma_lst [lappend sig_gamma_lst gamma_lna_$oref]
	set sig_m_lst [lappend sig_m_lst m_lna_$oref]
	set sig_k3_lst [lappend sig_k3_lst k3_lna_$oref]
	set sig_k5_lst [lappend sig_k5_lst k5_lna_$oref]
	set sig_sigma_lst2 [lappend sig_sigma_lst2 sigma_ah_$oref]
	set sig_beta_lst2 [lappend sig_beta_lst2 beta_ah_$oref]
	set sig_gamma_lst2 [lappend sig_gamma_lst2 gamma_ah_$oref]
	set sig_m_lst2 [lappend sig_m_lst2 m_ah_$oref]
	set sig_k3_lst2 [lappend sig_k3_lst2 k3_ah_$oref]
	set sig_k5_lst2 [lappend sig_k5_lst2 k5_ah_$oref]
	
    }


    set complete_lst {}
    set complete_lst [lappend complete_lst $sig_sigma_lst]
    set complete_lst [lappend complete_lst $sig_beta_lst]
    set complete_lst [lappend complete_lst $sig_gamma_lst]
    set complete_lst [lappend complete_lst $sig_m_lst]
    set complete_lst [lappend complete_lst $sig_k3_lst]
    set complete_lst [lappend complete_lst $sig_k5_lst]
    set complete_lst [lappend complete_lst $sig_sigma_lst2]
    set complete_lst [lappend complete_lst $sig_beta_lst2]
    set complete_lst [lappend complete_lst $sig_gamma_lst2]
    set complete_lst [lappend complete_lst $sig_m_lst2]
    set complete_lst [lappend complete_lst $sig_k3_lst2]
    set complete_lst [lappend complete_lst $sig_k5_lst2]

    set code [catch {mdisp 2 6 ${complete_lst}} result]
    if {$code != 0} {
	error $result $result
    }
    ${result} setColorsByList {red violet cyan green blue}

    set itemList {}
    for {set oref  0} {$oref <= $num} {incr oref} {
    	set itemList [lappend itemlist [list %c $oref]]
    }
    eval ${result} setLabelsItemsByList $itemList
    ${result}gr0000 set_label {black "sigma_vs_lna, "} allSigLabel
    ${result}gr0001 set_label {black "beta_vs_lna, "} allSigLabel
    ${result}gr0002 set_label {black "gamma_vs_lna, "} allSigLabel
    ${result}gr0003 set_label {black "m_vs_lna, "} allSigLabel
    ${result}gr0004 set_label {black "k3_vs_lna, "} allSigLabel
    ${result}gr0005 set_label {black "k5_vs_lna, "} allSigLabel

    ${result}gr0100 set_label [list black "sigma_vs_a^$alpha, "] allSigLabel
    ${result}gr0101 set_label [list black "beta_vs_a^$alpha, "] allSigLabel
    ${result}gr0102 set_label [list black "gamma_vs_a^$alpha, "] allSigLabel
    ${result}gr0103 set_label [list black "m_vs_a^$alpha, "] allSigLabel
    ${result}gr0104 set_label [list black "k3_vs_a^$alpha, "] allSigLabel
    ${result}gr0105 set_label [list black "k5_vs_a^$alpha, "] allSigLabel


    return $result
}

