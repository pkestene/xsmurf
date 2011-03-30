#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#
#  $Id: part_fct.tcl,v 1.2 1998/08/25 11:47:01 decoster Exp $
#
#  The part_fct object defined a structure for partition functions of wavelet
# transform.
#

class part_fct
part_fct inherit Base

#
part_fct method init {} {
    # Scales.
    instvar a_min
    instvar n_octave
    instvar n_voice
    instvar first_octave
    instvar last_octave
    instvar first_voice
    instvar last_voice

    # Values of q.
    instvar q_lst

    # Number of studied realisations.
    instvar n_study

    # General informations :

    # Dimension of the signals (1D, 2D, etc).
    instvar dimension
    # Size of the signals.
    instvar size
    # Method of the study (gradient, mexican, harr, etc).
    instvar study_method

    # User defined comments.
    instvar comments

    next

    # Initialisation of the instvars :
    set a_min -1
    set n_octave 0
    set n_voice 0
    set first_octave -1
    set last_octave -1
    set first_voice -1
    set last_voice -1
    set q_lst {}
    set n_study 0
    set size -1
    set dimension -1
    set study_method none
    set comments $self
}

#
part_fct method destroy {} {
    next
    delete_signals
}

#
part_fct method delete_signals {} {
    instvar q_lst

    foreach q $q_lst {
	set q_str [get_q_str $q]
	catch {delete ${self}_STq_$q_str}
	catch {delete ${self}_logSTq_$q_str}
	catch {delete ${self}_STqlogT_$q_str}
    }   
}

#
part_fct method init_parameters {amin noctave nvoice qlst s met} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    if {$n_study  == 0} {
	if {$amin <= 1} {error "bad value of parameter \"amin\" for \"$self\""}
	set a_min $amin
	if {$noctave < 1} {error "bad value of parameter \"noctave\" for \"$self\""}
	set n_octave $noctave
	if {$nvoice < 1} {error "bad value of parameter \"nvoice\" for \"$self\""}
	set n_voice $nvoice
	set first_octave 0
	set last_octave [expr $noctave-1]
	set first_voice 0
	set last_voice [expr $nvoice-1]
	set q_lst $qlst
	if {$s < 1} {error "bad value of parameter \"s\" for \"$self\""}
	set size $s
	set dimension 2D
	set study_method $met
	set comments $self

	delete_signals
    } else {
	error "parameters already initialised for \"$self\""
    }
}


#
part_fct method compute {e_name} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    set options ""
    if {[string compare $study_method "gradient line max"] == 0} {
	set options "-vc"
    }
    sm_fqaqtq $e_name $a_min $n_octave $n_voice $q_lst $self $options
    incr n_study
}

#
part_fct method compute_new {e_name amin noctave nvoice qlst s met} {
    init_parameters $amin $noctave $nvoice $qlst $s $met
    compute $e_name
}

#
part_fct method display {} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    puts "Partition function"
    puts "First scale : $a_min"
    puts "Number of octaves : $n_octave"
    puts "Number of voices : $n_voice"
    puts "First octave : $first_octave"
    puts "Last octave : $last_octave"
    puts "First voice : $first_voice"
    puts "Last voice : $last_voice"
    puts "Sources size : $size"
    puts "Number of sources : $n_study"
    puts "Source dimension : $dimension"
    puts "Method : $study_method"
    puts "List of values of q : $q_lst"
    puts "Comments :\n$comments"
}

part_fct method save {} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    set file [open $self w]

    puts $file "Partition function"
    puts $file "First scale : $a_min"
    puts $file "Number of octaves : $n_octave"
    puts $file "Number of voices : $n_voice"
    puts $file "First octave : $first_octave"
    puts $file "Last octave : $last_octave"
    puts $file "First voice : $first_voice"
    puts $file "Last voice : $last_voice"
    puts $file "Sources size : $size"
    puts $file "Number of sources : $n_study"
    puts $file "Source dimension : $dimension"
    puts $file "Method : $study_method"
    puts $file "List of values of q : $q_lst"

    foreach q $q_lst {
	set q_str [get_q_str $q]
	puts $file "q : $q"
	puts $file "STq : [sgetlst ${self}_STq_${q_str}]"
	puts $file "logSTq : [sgetlst ${self}_logSTq_${q_str}]"
	puts $file "STqlogT : [sgetlst ${self}_STqlogT_${q_str}]"
    }

    puts $file "Comments :\n$comments"
    close $file
}

part_fct method load {{file_name ""}} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    if {$file_name == ""} {
	set file_name $self
    }

    set file [open $file_name r]
    if {$n_study == 0} { # Create new values
	gets $file
	set a_min        [lindex [gets $file] end]
	set n_octave     [lindex [gets $file] end]
	set n_voice      [lindex [gets $file] end]
	set first_octave [lindex [gets $file] end]
	set last_octave  [lindex [gets $file] end]
	set first_voice  [lindex [gets $file] end]
	set last_voice   [lindex [gets $file] end]
	set size         [lindex [gets $file] end]
	set n_study      [lindex [gets $file] end]
	set dimension    [lindex [gets $file] end]
	set study_method [lrange [gets $file] 2 end]
	set q_lst        [lrange [gets $file] 6 end]

	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    gets $file
	    set val_lst [lrange [gets $file] 2 end]
	    screate ${self}_STq_${q_str} 1 [expr 1.0/$n_voice] $val_lst
	    set val_lst [lrange [gets $file] 2 end]
	    screate ${self}_logSTq_${q_str} 1 [expr 1.0/$n_voice] $val_lst
	    set val_lst [lrange [gets $file] 2 end]
	    screate ${self}_STqlogT_${q_str} 1 [expr 1.0/$n_voice] $val_lst
	}

	gets $file
	gets $file comments
	while {[gets $file str] != -1} {
	    set comments "$comments\n$str"
	}
	close $file
    } else { # Add the new values.
	gets $file
	if {$a_min    != [lindex [gets $file] end]} {return "Bad value"}
	if {$n_octave != [lindex [gets $file] end]} {return "Bad value"}
	if {$n_voice  !=  [lindex [gets $file] end]} {return "Bad value"}

	# !!!! ATTENTION -> METTRE A JOUR CES VALEURS !!!!!!!!!!!
	set new_first_octave [lindex [gets $file] end]
	set new_last_octave  [lindex [gets $file] end]
	set new_first_voice  [lindex [gets $file] end]
	set new_last_voice   [lindex [gets $file] end]
	
	if {$size != [lindex [gets $file] end]} {return "Bad value"}
	
	incr n_study [lindex [gets $file] end]
	
	if {[string compare $dimension [lindex [gets $file] end]] != 0} {return "Bad value"}
	if {[string compare $study_method [lrange [gets $file] 2 end]] != 0} {return "Bad value"}
	if {[string compare $q_lst [lrange [gets $file] 6 end]] != 0} {return "Bad value"}
	
	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    gets $file

	    set val_lst [lrange [gets $file] 2 end]
	    screate __tmp 1 [expr 1.0/$n_voice] $val_lst
	    sadd ${self}_STq_${q_str} __tmp ${self}_STq_${q_str}
	    
	    set val_lst [lrange [gets $file] 2 end]
	    screate __tmp 1 [expr 1.0/$n_voice] $val_lst
	    sadd ${self}_logSTq_${q_str} __tmp ${self}_logSTq_${q_str}

	    set val_lst [lrange [gets $file] 2 end]
	    screate __tmp 1 [expr 1.0/$n_voice] $val_lst
	    sadd ${self}_STqlogT_${q_str} __tmp ${self}_STqlogT_${q_str}
	}
	catch {delete __tmp}
	close $file
    }
}

#
#
part_fct method sw_load {{file_name ""}} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    if {$file_name == ""} {
	set file_name $self
    }

    if {$n_study == 0} { # Create new values
	# Read general file.
	set file [open $file_name r]

	set n_octave [lindex [gets $file] 1]
	set n_voice [lindex [gets $file] 1]
	set study_method [lindex [gets $file] 1]
	if {[string compare $study_method "Gauss_dx_dy"] == 0} {
	    set study_method "gradient lines"
	}
	set a_min [lindex [gets $file] 1]
	gets $file q
	while {$q != ""} {
	    lappend q_lst $q
	    gets $file q
	}
	close $file

	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    # Read existing data.
	    set STq_file [open ${file_name}_Pq.$q_str r]
	    set logSTq_file [open ${file_name}_logsP.$q_str r]
	    set STqlogT_file [open ${file_name}_PqlogP.$q_str r]
	    catch {unset STq_lst}
	    catch {unset logSTq_lst}
	    catch {unset STqlogT_lst}
	    for {set o 0} {$o < $n_octave} {incr o} {
		for {set v 0} {$v < $n_voice} {incr v} {
		    lappend STq_lst [gets $STq_file]
		    lappend logSTq_lst [gets $logSTq_file]
		    lappend STqlogT_lst [gets $STqlogT_file]
		}
	    }

	    close $STq_file
	    close $logSTq_file
	    close $STqlogT_file
	    screate ${self}_STq_$q_str 1 [expr 1.0/$n_voice] $STq_lst
	    screate ${self}_logSTq_$q_str 1 [expr 1.0/$n_voice] $logSTq_lst
	    screate ${self}_STqlogT_$q_str 1 [expr 1.0/$n_voice] $STqlogT_lst
	}
	set first_octave 0
	set last_octave [expr $n_octave-1]
	set first_voice 0
	set last_voice [expr $n_voice-1]
	set n_study 1
	set size -1
	set dimension 2D
	set comments $self
    } else {
	return "Can't add from a sw file"
    }
}

#
#
part_fct method sw_save {{file_name ""}} {
    instvar a_min n_octave n_voice
    instvar first_octave last_octave
    instvar first_voice last_voice
    instvar q_lst n_study dimension size
    instvar study_method comments

    if {$file_name == ""} {
	set file_name $self
    }

    # Create general file.
    set file [open $file_name w]
    puts $file "noct $n_octave"
    puts $file "nvoice $n_voice"
    puts $file "wavelet $study_method"
    puts $file "amin $a_min"
    foreach q $q_lst {
	puts $file $q
    }
    close $file

    # Create history file.
    set hist_file [open ${file_name}_history w]
    for {set i 0} {$i < $n_study} {incr i} {
	puts $hist_file "***Computation on a new signal:"
	puts $hist_file "q Values   \[nb = [llength $q_lst]\]:"
	foreach q $q_lst {
	    puts -nonewline $hist_file "$q "
	}
	puts $hist_file ""
	puts $hist_file "Signal size : [expr $n_octave*$n_voice]"
	puts $hist_file "Signal name : $study_method"
	puts $hist_file ""
    }
    close $hist_file


    # Create data files.
    foreach q $q_lst {
	set q_str [get_q_str $q]
	
	ssave ${self}_STq_$q_str ${file_name}_Pq.$q_str -ascii -noheader
	ssave ${self}_logSTq_$q_str ${file_name}_logsP.$q_str -ascii -noheader
	ssave ${self}_STqlogT_$q_str ${file_name}_PqlogP.$q_str -ascii -noheader
    }
}

#
part_fct method set_size {new_size} {
    instvar size
    set size $new_size
}

#
part_fct method set_method {new_method} {
    instvar study_method
    set study_method $new_method
}

#
part_fct method thd {} {
    instvar q_lst

    foreach q $q_lst {
	set q_str [get_q_str $q]

	set STq     ${self}_Pq_$q_str
	set STqlogT ${self}_PqlogP_$q_str

	scomb $STq $STq     log(x)/log(2)              ${self}_tau$q_str
	scomb $STqlogT $STq x/(log(2)*y)               ${self}_h$q_str
	scomb $STqlogT $STq ($q*x-y*log(y))/(y*log(2)) ${self}_D$q_str
    }
}

#
part_fct method tq {a_min a_max} {
    instvar q_lst

    catch {unset fit_lst}
    foreach q $q_lst {
	set q_str [get_q_str $q]

	set fit [sfit ${self}_tau$q_str $a_min $a_max]
	lappend fit_lst [lindex $fit 0]
    }
    screate ${self}_tq [lindex $q_lst 0] 1 $fit_lst -xy $q_lst
}

#
part_fct method name_lst {name} {
    instvar q_lst
    
    foreach q $q_lst {
	set q_str [get_q_str $q]
	lappend lst ${name}${q_str}
    }
    
    return $lst
}
