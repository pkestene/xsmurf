# study.tcl --
#
#       This file implements the Tcl code to compute all kind of study and to
# store its results and its "state".
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: study.tcl,v 1.31 1999/03/29 13:33:08 decoster Exp $
#

package provide study 0.0

namespace eval study {
    variable study

    namespace export init end param compute
}


# st --
#

proc st args {
    if {[llength $args] != 0} {
	set cmd [concat namespace inscope study $args]
	set code [catch {eval $cmd} result]
	if {$code != 0} {
	    return -code error $result
	} else {
	    return $result
	}
    }
}


# study::init --
# usage : study::init [-filename str] [-force] [-desc array]
#
#   Init the parameters of a study. The paramters of the study must be stored
# in a file whose default name is paramters.tcl.
#   The file must contain the base directory for the study. By default this
# procedure must be execute in the base directory. This is done to avoid
# pathname error. This can be escape by the -force option.
#
# Parameters :
#   none.
#
# Options :
#   -filename : gah !
#      string - file name.
#   -force : escape the directory check.
#   -desc : check the validity of parameters.
#      array - descriptor of the parameters. Each element describe a parameter.
#
# Return value :
#   The study id.

proc study::init {args} {
    # Default value of the proc variables.

    set fileName parameters.tcl
    set curDir [pwd]
    set isForce 0
    set isDesc 0

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -filename {
		set fileName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -force {
		set isForce 1
		set args [lreplace $args 0 0]
	    }
	    -desc {
		set isDesc 1
		upvar [lindex $args 1] desc
		set args [lreplace $args 0 1]
	    }
	    --  {
		set args [lreplace $args 0 0]
		break
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    # Get an id and create param array.

    variable study
    if ![info exists study(sid)] {
	set study(sid) 0
    }
    set sid [namespace current]::[incr study(sid)]
    variable $sid
    upvar 0 $sid param

    # Read parameters.

    set f [open $fileName r]
    while {[gets $f l] != -1} {

	if {[llength $l] == 0} {
	    # Avoid empty lines.

	    continue
	}

	if {[regexp {^# *} [lindex $l 0]] == 1} {
	    # Avoid lines that begins with '#'.

	    continue
	}

	if {[string compare [lindex $l 0] set] == 0} {
	    # For backward compatibility, we remove leading "set" string.

	    set l [lreplace $l 0 0]
	}

	set parameter [lindex $l 0]
	set value [lindex $l 1]

	if {$isDesc && [info exists desc($parameter)]} {
	    # If requested, we check the type validity of the parameters
	    # describe by desc array.

	    set type [lindex $desc($parameter) 1]
	    set code [catch {CheckType $value $type} result]
	    if {$code == 1} {
		# Wrong type.

		catch {unset param}
		return -code error "parsing parameter \"$parameter\" : $result"
	    } else {
		set param($parameter) $value
	    }
	} else {
	    set param($parameter) $value
	}
    }
    close $f

    if {$isDesc} {
	# Now we set all the parameters that are describe in desc and that were
	# not defined in fileName.

	set searchId [array startsearch desc]
	while {[array anymore desc $searchId] == 1} {
	    set parameter [array nextelement desc $searchId]
	    if {[info exists param($parameter)] == 0} {
		set default [lindex $desc($parameter) 0] 
		if {[string compare $default "requested"] == 0} {
		    # The parameter _must_ have been defined in fileName.

		    catch {unset param}
		    return -code error "parameter \"$parameter\" is not optional"
		} else {
		    set param($parameter) $default
		}
	    }
	}
	array donesearch desc $searchId
    }

    if {!$isForce} {
	# Check if the base directory of the study is the same than the current
	# directory.

	if {[string compare $curDir $param(baseDir)] != 0} {
	    catch {unset param}
	    return -code error "you must init your study from its base directory. Unless you use -force option"
	}
    }

    return $sid
}


# study::CheckType - PRIVATE
# usage : CheckType list list
#
#   Check the validity of a value according to its decriptor.
#
# Parameters :
#   list - list of values.
#   list - list of types (-boolean, -int, ...). Help on each type will come
#     soon (I hope).
#
# Return value :
#   No value. In case of bad type, an error is raised.

proc study::CheckType {valueLst typeLst} {
    foreach value $valueLst type $typeLst {
	switch -exact -- [lindex $type 0] {
	    -boolean {
		if ![regexp {^(0|1)$} $value] {
		    error "bad type"
		}
	    }
	    -int {
		if ![regexp {^(-+)?[0-9]+$} $value] { 
		    error "bad type"
		}
	    }
	    -posint {
		if ![regexp {^[+]?[0-9]+$} $value] { 
		    error "bad type"
		}
	    }
	    -float {
		if {[scan $value "%f%s" dummy dummy] != 1} {
		    error "bad type"
		}
	    }
	    -posfloat {
		if {[scan $value "%f%s" dummy dummy] != 1} {
		    error "bad type"
		}
		if {$dummy < 0} {
		    error "bad type"
		}
	    }
	    -intfloat {
		if {[scan $value "%f%s" dummy dummy] != 1} {
		    error "bad type"
		}
		if {($dummy < [lindex $type 1]) \
			|| ($dummy > [lindex $type 2])} {
		    error "bad type"
		}
	    }
	    -dir {
		if {[file isdirectory $value] != 1} {
		    error "bad type"
		}
	    }
	    -string {
		return
	    }
	    -list {
		set type [lreplace $type 0 1]
		foreach newValue $value {
		    set code [catch \
			    {CheckType $newValue $type} \
			    result]
		    if {$code == 1} {
			error $result
		    }
		}
	    }
	    default {
		eval $type
	    }
	}
    }
    return
}


# study::end --
# usage : study::end studyId
#
#   End a study and free all memory that can be freed.
#
# Parameters :
#   studyId - the study id.
#
# Return value :
#   None.

proc study::end {sid} {
    if {[CheckSid $sid] == 0} {
	return -code error "wrong study id"
    }
    variable $sid
    upvar 0 $sid param
    unset param
    return
}


# study::CheckSid - PRIVATE
# usage : CheckSid studyId
#
#   Check if a study id is valid.
#
# Parameters :
#   studyId - the study id.
#
# Return value :
#   1 if valid, 0 otherwise.

proc study::CheckSid {sid} {
    # For now this proc only check the existence of sid as an array. In the
    # future we can imagine other checks.

    if [array exists $sid] {
	return 1
    } else {
	return 0
    }
}


# study::param --
# usage : study::param studyId [-channel channelId] [-help array]
#
#   Puts the name and the value of all parameters on stdout.
#
# Parameters :
#   studyId - the study id.
#
# Options :
#   -channel : use another channel.
#      channelId - the channel id.
#   -help : add an help at the end of each line.
#      array - descriptor of the parameters. Each element describe a parameter.
#
# Return value :
#   None.

proc study::param {sid args} {
    if {[CheckSid $sid] == 0} {
	return -code error "wrong study id"
    }
    variable $sid
    upvar 0 $sid param

    # Default values

    set channelId stdout
    set isHelp 0

    # Arguments analysis

    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    -channel {
		set channelId [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -help {
		set isHelp 1
		upvar [lindex $args 1] desc
		set args [lreplace $args 0 1]
	    }
	    --  {
		set args [lreplace $args 0 0]
		break
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    # Search the maximum length of all parameter and value strings to format
    # output.

    set pMaxLength 0
    set vMaxLength 0
    set searchId [array startsearch param]
    while {[array anymore param $searchId] == 1} {
	set parameter [array nextelement param $searchId]
	set length [string length $parameter]
	if {$length > $pMaxLength} {
	    set pMaxLength $length
	}
	set value $param($parameter)
	set length [string length $value]
	if {$length > $vMaxLength} {
	    set vMaxLength $length
	}
    }
    # To be changed...
    if {$vMaxLength > 30} {
	set vMaxLength 30
    }

    # Output...

    array donesearch param $searchId
    set searchId [array startsearch param]
    while {[array anymore param $searchId] == 1} {
	set parameter [array nextelement param $searchId]
	set value $param($parameter)
	if {$isHelp && [info exists desc($parameter)]} {
	    # With help message (if it exists).

	    set help [lindex $desc($parameter) 2]
	    puts $channelId [format \
		    "%-${pMaxLength}s %-${vMaxLength}s %s" \
		    $parameter $value $help]
	} else {
	    puts $channelId [format \
		    "%-${pMaxLength}s %-${vMaxLength}s" \
		    $parameter $value]
	}
    }
    array donesearch param $searchId

    return
}


# study::param2var --
# usage : study::param2var studyId
#
#   Create a variable in the current scope for each parameter.
#
# Parameters :
#   studyId - the study id.
#
# Options :
#
# Return value :
#   None.

proc study::param2var {sid args} {
    if {[CheckSid $sid] == 0} {
	return -code error "wrong study id"
    }
    variable $sid
    upvar 0 $sid param

    # Default values

    set channelId stdout
    set isHelp 0

    # Arguments analysis

    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    # Set the variables.

    set searchId [array startsearch param]
    while {[array anymore param $searchId] == 1} {
	set parameter [array nextelement param $searchId]
	set value $param($parameter)
	uplevel 1 [list set $parameter $value]
    }
    array donesearch param $searchId

    return
}


# study::compute --
# usage : study::compute studyId script
#
#   Compute a script for a study
#
# Parameters :
#   studyId - the study id.
#   script  - the script.
#
# Return value :
#   Result of the script.

proc study::compute {sid script} {
    if {[CheckSid $sid] == 0} {
	return -code error "wrong study id"
    }
    variable $sid
    upvar 0 $sid param

    # First, we modify the script to create variables associated to each
    # parameter.

    set newScript $script
    set searchId [array startsearch param]
    while {[array anymore param $searchId] == 1} {
	set parameter [array nextelement param $searchId]
	set value $param($parameter)
	set newScript "set $parameter [list $value]\n$newScript"
    }
    array donesearch param $searchId

    eval $newScript
    
    return
}



#####################################################################

# Here begins the old version of study.tcl. This is kept for compatibility.


#
#   This procedure execute a complete study of a given statistical set of a kind
# of image. The only parameter of this proc is the directory where the results
# will be saved. In this directory it must be a file (parameters.tcl) that
# contains all the parameters of the study. Example :
#
#    set first_image 0       (number of the first image to study)
#    set last_image 7        (number of the last image to study)
#    set size 512            (size of the images)
#    set amin 2              (value of the minimum scale)
#    set noct 4              (number of octaves)
#    set nvox 8              (number of voices)
#    set nbox_mod 1024       (size of the modulus's histograms)
#    set nbox_arg 512        (size of the gradient's histograms)
#    set calendos_size 512   (size of the french-cheese histograms which smell)
#    set border_percent 0.75 (value which gives the size of the border to cut)
#                            (this size is this value times the greatest scale)
#                            (the best value to avoid border effects is)
#                            (probably 0.75)
#    set similitude 0.8      (similitude for the research of vertical line)
#    set angle 0
#    set isgaussian 0        (Gaussian study)
#    set ismexican 0         (Mexican hat study)
#    set issave 0            (save histo and part fcts beetween each image)
#                            (study so that crashes don't pollute results)
#    set ishisto 0           (compute the continuous histograms)
#    set ismaxhisto 0        (compute the maxima histograms)
#    set ishistoinit 0       (init the histograms)
#    set iscontpart 0        (compute the continuous partition fcts)
#    set q_lst {-2 -1 0 1 2} (list of the values of q, for max partition fcts)
#    set pos_q_lst {0 1 2}   (list of the positives values of q, for images)
#                            (partition fcts)
#    set theta_lst {0 0.1 m0 0.74 0.3 m1}
#                            (list of theta d_theta name triplet. To compute)
#                            (partition fcts conditioned by gradient)
#                            (orientation. Name is the base name of the)
#                            (partition fcts files)
#
#   The directories 00, 01, etc must exist and must contain an image (named
# 'image').
#   The procedure computes the maxima, the complete histograms, the maxima
# histograms and the partition functions of the maxima (depend on the values of
# 'is' parameters).
#   The values of q for the partition functions of the maxima must be stored in
# a file named 'q_values'. For the partition functions of all the wavelet values
# the file is named 'pos_q_values'
#

proc study {dir} {
    cd $dir
    
    # Get parameters
    source ${dir}/parameters.tcl

    # Existence of the parameters
    puts ""
    puts "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
    puts ""
    puts "Parameters verification :"
    puts "  first image :       $first_image"
    puts "  last image :        $last_image"
    puts "  Size of the image : $size"
    puts "  First scale :       $amin"
    puts "  Number of octave :  $noct"
    puts "  Number of voice :   $nvox"
    puts "  Border percent :    $border_percent"
    puts "     (0 -> gaussian, 1 -> gaussiann and mexican hat)"
    puts "  Size of the modulus's histograms :  $nbox_mod"
    puts "  Size of the gradient's histograms : $nbox_arg"
    puts "  Size of the french-cheese histograms which smell : $calendos_size"
    puts "  Initialisation of the histograms ? $ishistoinit"
    puts "  Saving the ext_image ? $issave"
    puts "  Histograms ? $ishisto"
    puts "  similitude ? $similitude"
    puts "  Gaussian study ? $isgaussian"
    puts "  Mexican study ? $ismexican"
    puts "  Functions of partitions conditioned by the angle ? $angle"
    puts ""
    puts "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
    puts ""
    puts ""

    # Validity of the parameters
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int(2*$scale_max*$border_percent)]

    if {$scale_max >= $size} {
	puts " .. are you crazy? the scale max is greater than the size of the image"
	return
    }
    if {$border_size >= $size} {
	puts " .. are you crazy? the borders effects are greater than the size of the image"
	return
    }

    # Initialisation of the histograms
    if {$ishistoinit == 1} {
	catch {exec mkdir histograms}
	if {$isgaussian == 1} {
	    nputs "\nHistograms initialisation (Gaussian).\n"
	    set_hist_limits $dir
	}
	if {$ismexican == 1} {
	    nputs "\nHistograms initialisation (Mexican).\n"
	    set_hist_mexican_limits $dir
	}
    }

    # Now we begin the computation of the WT
    nputs "\nComputation.\n"
    catch {exec mkdir partition}
    catch {exec mkdir sauve}

    # For all images
    for {set i $first_image} {$i <= $last_image} {incr i} {
	set new_i [format "%.2d" $i]
	puts "------- Image number $new_i -------\n"
	if {$isgaussian == 1} {
	    dputs "Compute the WTMM."
	    compute_wtmm ${dir}/$new_i
	    compute_wtmm_max_stat ${dir}/$new_i
	}
	if {$ismexican == 1} {
	    puts ""
	    dputs "Compute the mexican hat."
	    compute_mexican ${dir}/$new_i
	    compute_mexican_max_stat ${dir}/$new_i
	}

	if {$issave == 1} {
	    puts ""
	    nputs "  Save"
	    cd $dir
	    exec cp -r ${dir}/partition ${dir}/sauve
	    exec cp -r ${dir}/histograms ${dir}/sauve
	}
	dputs "  Ok, Baby! Computation of this image completed !\n"
    }
    puts ""
    puts "This is the end."
}


#   Procedure to set the histograms limits of the wavelet coeficients made with
# the Gaussian.
#
proc set_hist_limits {dir} {
    set pi 3.1416
    set m_pi -3.1416

    source ${dir}/parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name min_max_hist

    set hist_min 1e30
    set hist_max 0
    
    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    for {set i $first_image} {$i <= $last_image} {incr i} {
	set new_i [format "%.2d" $i]
	dputs "Image number $new_i."
	iload ${dir}/${new_i}/image image

	igfft   image __ft
	delete  image
	isave   __ft
	delete  __ft

	nputs "  setting the limits of the modulus histograms :"
	dputs "    with the upper scale..."
	set scale [expr $amin*pow(2,$noct-1+(($nvox-1)/double($nvox)))]
	set scale [expr $scale*(6/0.86)]
	__conv_dx $scale
	__conv_dy $scale
	gmod dx dy mod
	cutedge mod mod $border_size
	set result [im_extrema mod]
	set min_upper [lindex $result 0]
	set max_upper [lindex $result 1]

	dputs "    and the lower scale..."
	set scale [expr $amin*(6/0.86)]
	__conv_dx $scale
	__conv_dy $scale
	gmod dx dy mod
	cutedge mod mod $border_size
	set result [im_extrema mod]
	set min_lower [lindex $result 0]
	set max_lower [lindex $result 1]

	delete dx dy mod IMAGE

	if {$min_lower < $hist_min} {
	    set hist_min $min_lower
	}
	if {$min_upper < $hist_min} {
	    set hist_min $min_upper
	}
	if {$max_upper > $hist_max} {
	    set hist_max $max_upper
	}
	if {$max_lower > $hist_max} {
	    set hist_max $max_lower
	}
	puts ""
    }
    dputs "The range of modulus histogram for all images is :"
    nputs "  from $hist_min to $hist_max."
    puts ""

    set hist_file_id [open $hist_file_name w]
    puts $hist_file_id "$hist_min"
    puts $hist_file_id "$hist_max"
    close $hist_file_id

    # Initialisation of the global histograms at each scale.
    sinus histo_tmp 100
    shisto histo_tmp h_mod_tmp $nbox_mod -x $hist_min $hist_max
    shisto histo_tmp h_arg_tmp $nbox_arg -x $m_pi $pi
    sscamult h_mod_tmp 0.0 h_mod_tmp
    sscamult h_arg_tmp 0.0 h_arg_tmp

    inull calendos_temp $calendos_size
    cd ${dir}/histograms
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]

            #  Gradient histograms from all points (mod, arg, gradient vector)
            ssave h_mod_tmp h_mod$new_num -sw
            ssave h_arg_tmp h_arg$new_num -sw
            isave calendos_temp calendos_image$new_num

            #  Gradient histograms from all points on lines (mod, arg,
            # gradient vector)
            ssave h_mod_tmp h_max_mod$new_num -sw
            ssave h_arg_tmp h_max_arg$new_num -sw
            isave calendos_temp calendos_max$new_num

            #  Gradient histograms from all max on lines (mod, arg, gradient
            # vector)
            ssave h_mod_tmp h_max_line_mod$new_num -sw
            ssave h_arg_tmp h_max_line_arg$new_num -sw
            isave calendos_temp calendos_max_line$new_num

	}
    }
    delete histo_tmp
    delete h_mod_tmp
    delete h_arg_tmp
    delete calendos_temp
    cd ${dir}
    exec rm __ft
}

#
# procedure to set the histograms limits of the wavelet coeficients
# made with the Mexican Hat.
#

proc set_hist_mexican_limits {dir} {
    source ${dir}/parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name min_max_hist_mexican

    set hist_min 1e30
    set hist_max -1e30
    
    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    for {set i $first_image} {$i <= $last_image} {incr i} {
	set new_i [format "%.2d" $i]
	dputs "Image number $new_i."
	iload ${dir}/${new_i}/image im

	itofft  im IMAGE
	isave   im im.0
	delete im
	fft     IMAGE
	isave   IMAGE im.1

	nputs "  setting the limits of the histograms :"
	dputs "    with the upper scale..."
	set scale [expr $amin*pow(2,$noct-1+(($nvox-1)/double($nvox)))]
	set scale [expr $scale*(6/0.86)]
	conv_w mexican $scale
	cutedge mexican mexican $border_size
	set result [im_extrema mexican]

	set min_upper [lindex $result 0]
	set max_upper [lindex $result 1]

	dputs "    and the lower scale..."
	set scale [expr $amin*(6/0.86)]
	conv_w mexican $scale
	cutedge mexican mexican $border_size
	set result [im_extrema mexican]
	set min_lower [lindex $result 0]
	set max_lower [lindex $result 1]

	delete mexican IMAGE

	if {$min_lower < $hist_min} {
	    set hist_min $min_lower
	}
	if {$min_upper < $hist_min} {
	    set hist_min $min_upper
	}
	if {$max_upper > $hist_max} {
	    set hist_max $max_upper
	}
	if {$max_lower > $hist_max} {
	    set hist_max $max_lower
	}
	puts ""
    }
    dputs "The range of mexican histogram for all images is :"
    nputs "  from $hist_min to $hist_max."
    puts ""

    set hist_file_id [open $hist_file_name w]
    puts $hist_file_id "$hist_min"
    puts $hist_file_id "$hist_max"
    close $hist_file_id

    # Initialisation of the global histograms at each scale.
    sinus histo_tmp 100
    shisto histo_tmp h_mexican_tmp $nbox_mod -x $hist_min $hist_max

    sscamult h_mexican_tmp 0.0 h_mexican_tmp

    cd ${dir}/histograms
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]
	    ssave h_mexican_tmp h_mexican$new_num -sw
	    ssave h_mexican_tmp h_max_mexican$new_num -sw 
	}
    }
    delete histo_tmp
    delete h_mexican_tmp

    cd ${dir}
    exec rm im.0 im.1
}

proc compute_images {scale new_num} {
    conv_all $scale
    gkapap kapap${new_num} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
    delete dxxx dxxy dxyy dyyy
    gkapa kapa${new_num} dx dxx dy dyy dxy
    delete dx dy dxx dxy dyy
}

# NEW !
proc compute_images_NEW {scale new_num} {
    __conv_all_derivatives $scale
    garg dx dy arg$new_num
    gmod dx dy mod$new_num
    gkapap kapap${new_num} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
    delete dxxx dxxy dxyy dyyy
    gkapa kapa${new_num} dx dxx dy dyy dxy
    delete dx dy dxx dxy dyy
}
# NEW !

proc images_histo {dir new_num} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name ${dir}/../min_max_hist

    # Get histograms parameters
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    set pi 3.1416
    set m_pi -3.1416

    # Loading
    sload h_mod$new_num h_mod$new_num -sw
    sload h_arg$new_num h_arg$new_num -sw
    iload calendos_image$new_num

    # Computing
    ihisto mod$new_num h_mod_current_$new_num $nbox_mod \
	    -x $hist_min $hist_max 
    ihisto arg$new_num h_arg_current_$new_num $nbox_arg \
	    -x $m_pi $pi
    ihisto mod$new_num calendos_current_$new_num $calendos_size \
	    -x -$hist_max $hist_max -grad arg$new_num

    # Adding
    sadd h_mod$new_num h_mod_current_$new_num h_mod$new_num
    sadd h_arg$new_num h_arg_current_$new_num h_arg$new_num
    iadd calendos_image$new_num calendos_current_$new_num 

    # Saving
    ssave h_mod$new_num -sw
    ssave h_arg$new_num -sw
    isave calendos_image$new_num

    # Deleting
    delete h_mod_current_$new_num \
	    h_arg_current_$new_num \
	    calendos_current_$new_num

    delete h_mod$new_num \
	    h_arg$new_num \
	    calendos_image$new_num
}

proc mex_images_histo {dir new_num} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name ${dir}/../min_max_hist_mexican

    # Get histograms parameters
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    # Loading
    sload h_mexican$new_num h_mexican$new_num -sw

    # Computing
    ihisto mexican h_mexican_current_$new_num $nbox_mod \
	    -x $hist_min $hist_max 

    # Adding
    sadd h_mexican$new_num h_mexican_current_$new_num h_mexican$new_num

    # Saving
    ssave h_mexican$new_num -sw

    # Deleting
    delete h_mexican_current_$new_num
    delete h_mexican$new_num
}

proc compute_wtmm {dir} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!

    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    cd $dir

    # Computing the principal images
    iload   image
    igfft   image __ft
    isave   image __image
    delete  image
    isave   __ft
    delete  __ft

    # Compute the wtmm and the images histograms for each scale.
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {

	    # Setting of local parameters
	    set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
	    set scale [expr $scale*(6/0.86)]
	    nputs "  Octave $oct - vox $vox - scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    dputs "    compute mod arg kapa and kapap..."
	    compute_images_NEW $scale $new_num

	    dputs "    research of maxima..."
	    follow kapa${new_num} kapap${new_num} \
                    mod${new_num} arg${new_num} max$new_num $scale

	    # We remove the border of the ext_image
	    set border [expr $size-$border_size]
	    rm_ext max$new_num max$new_num $border_size $border $border_size $border

	    esave max$new_num
	    delete max$new_num

	    # We remove the border of the image
	    cutedge mod$new_num mod$new_num $border_size
	    cutedge arg$new_num arg$new_num $border_size

	    if {$iscontpart == 1} {
		dputs "    continuous partition functions..." 
		cd ${dir}/../partition
		#ipart $amin $noct $nvox $num partcont mod \
			#-qfile ${dir}/../pos_q_values

		isw_add_fqaqtq_one mod $amin $noct $nvox $num $pos_q_lst new_partcont
	    }

	    if {$ishisto == 1} {
		dputs "    computation of complete histograms."
		cd ${dir}/../histograms
		images_histo $dir $new_num
	    }

	    cd $dir

	    # Deleting intermediate images
	    delete arg$new_num
	    delete mod$new_num
	    delete kapa${new_num}
	    delete kapap${new_num}
	}
    }
    
    # Deleting the principal images
    exec rm __image
    exec rm __ft

    cd ${dir}/..
}

proc compute_mexican {dir} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!

    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    cd $dir

    # Computing the principal images
    iload   image im
    itofft  im 	IMAGE
    isave   im im.0
    delete im
    fft     IMAGE
    isave   IMAGE       im.1

    # Compute the wtmm and the images histograms for each scale.
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {

	    # Setting of local parameters
	    set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
	    set scale [expr $scale*(6/0.86)]
	    nputs "  Octave $oct - vox $vox - scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    dputs "    compute mexican..."
	    conv_w mexican $scale

	    dputs "    research of maxima..."
	    locmax mexican mex$new_num $scale -ext

	    # We remove the border of the ext_image
	    set border [expr $size-$border_size]
	    rm_ext mex$new_num mex$new_num $border_size $border $border_size $border

	    esave mex$new_num
	    delete mex$new_num

	    # We remove the border of the image
	    cutedge mexican mexican $border_size

	    if {$iscontpart == 1} {
		dputs "    continuous partition functions..." 
		cd ${dir}/../partition
		#ipart $amin $noct $nvox $num partcont mod \
			#-qfile ${dir}/../pos_q_values

		icopy mexican mexican$new_num
		isw_add_fqaqtq_one mexican $amin $noct $nvox $num $pos_q_lst mex_partcont
		delete mexican$new_num
	    }

	    if {$ishisto == 1} {
		dputs "    computation of complete histograms."
		cd ${dir}/../histograms
		mex_images_histo $dir $new_num
	    }

	    cd $dir

	    # Deleting intermediate images
	    delete mexican
	}
    }
    
    # Deleting the principal images
    delete IMAGE
    exec rm im.0
    exec rm im.1

    cd ${dir}/..
}

proc max_histos {dir new_num} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name ${dir}/../min_max_hist

    # Get histograms parameters
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    set pi 3.1416
    set m_pi -3.1416
    
    # For all the maxima

    # Loading
    sload h_max_mod$new_num h_max_mod$new_num -sw
    sload h_max_arg$new_num h_max_arg$new_num -sw
    iload calendos_max$new_num

    # Computing	
    ehisto max$new_num h_max_mod_current_$new_num \
	    $nbox_mod -x $hist_min $hist_max 
    ehisto max$new_num h_max_arg_current_$new_num \
	    $nbox_arg -x $m_pi $pi -arg
    ehisto max$new_num calendos_max_current_$new_num \
	    $calendos_size -x -$hist_max $hist_max -grad

    # Adding
    sadd h_max_mod$new_num h_max_mod_current_$new_num \
	    h_max_mod$new_num
    sadd h_max_arg$new_num h_max_arg_current_$new_num \
	    h_max_arg$new_num
    iadd calendos_max$new_num calendos_max_current_$new_num

    # Saving
    ssave h_max_mod$new_num -sw		
    ssave h_max_arg$new_num -sw
    isave calendos_max$new_num

    # Deleting
    delete h_max_mod_current_$new_num \
	    h_max_arg_current_$new_num \
	    calendos_max_current_$new_num

    delete h_max_mod$new_num \
	    h_max_arg$new_num \
	    calendos_max$new_num

    # Only for the max _potential_
    
    # Loading
    sload h_max_line_mod$new_num h_max_line_mod$new_num -sw
    sload h_max_line_arg$new_num h_max_line_arg$new_num -sw
    iload calendos_max_line$new_num

    # Computing
    ehisto max$new_num h_max_mod_current_$new_num \
	    $nbox_mod -x $hist_min $hist_max -vc
    ehisto max$new_num h_max_arg_current_$new_num \
	    $nbox_arg -x $m_pi $pi -arg -vc
    ehisto max$new_num calendos_max_current_$new_num \
	    $calendos_size -x -$hist_max $hist_max -grad -vc
    
    # Adding
    sadd h_max_line_mod$new_num h_max_mod_current_$new_num \
	    h_max_line_mod$new_num
    sadd h_max_line_arg$new_num h_max_arg_current_$new_num \
	    h_max_line_arg$new_num
    iadd calendos_max_line$new_num calendos_max_current_$new_num

    # Saving
    ssave h_max_line_mod$new_num -sw
    ssave h_max_line_arg$new_num -sw
    isave calendos_max_line$new_num
    
    # Deleting
    delete h_max_mod_current_$new_num \
	    h_max_arg_current_$new_num \
	    calendos_max_current_$new_num

    delete h_max_line_mod$new_num \
	    h_max_line_arg$new_num \
	    calendos_max_line$new_num
}

proc max_mexican_histos {dir new_num} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name ${dir}/../min_max_hist_mexican

    # Get histograms parameters
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    # For all the maxima

    sload h_max_mexican$new_num h_max_mexican$new_num -sw

    ehisto mex$new_num h_max_mexican_current_$new_num \
	    $nbox_mod -x $hist_min $hist_max 

    sadd h_max_mexican$new_num h_max_mexican_current_$new_num \
	    h_max_mexican$new_num

    ssave h_max_mexican$new_num -sw		

    delete h_max_mexican_current_$new_num
    delete h_max_mexican$new_num
}

proc compute_wtmm_max_stat {dir} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!

    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    cd $dir

    puts ""
    nputs "Horizontal and vertical chain of the maxima."
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    # Setting of local parameters
	    set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "  Octave $oct - vox $vox - scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    set prev_num [expr $num-1]
	    set prev_new_num [format "%.3d" $prev_num]

	    eload max$new_num

	    # We remove the border of the ext_image
	    #set border [expr $size-$border_size]
	    #rm_ext max$new_num max$new_num $border_size $border $border_size $border

	    # Research of maxima lines and maxima line maxima (yes, don't laugh).
	    hsearch max$new_num
	    ssm max$new_num

	    set box_size [expr int(log($scale)*2/log(2))]
	    if { $prev_num == 0} {
		vchain max$prev_new_num max$new_num $box_size $similitude -first
	    } else {
		if {$prev_num > 0} {
		    vchain max$prev_new_num max$new_num $box_size $similitude
		}
	    }
	}   
    }

    puts ""
    nputs "Maxima partition functions and histogramms."
    cd ${dir}/../partition
    
    #partition $amin $noct $nvox l_part max* \
	    #-qfile ${dir}/../q_values

    dputs "  On contour lines."
    sw_add_fqaqtq max $amin $noct $nvox $q_lst l_part

    dputs "  On maxima."
    sw_add_fqaqtq max $amin $noct $nvox $q_lst m_part -vc

    if {[info exists theta_lst] == 1} {
	dputs "  On maxima conditionned by gradient direction :"
	puts -nonewline "              "
	set num 0
	set pi 3.1416
	set m_pi -3.1416
	foreach {theta d_theta name} $theta_lst {
	    puts -nonewline "$theta "
	    set simil [expr 1-$d_theta/(2*$pi)]
	    sw_add_fqaqtq max $amin $noct $nvox $q_lst $name -vc -arg $theta $simil
	    incr num
	}
    }

    puts ""

    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]
	    # We compute the differents maxima's histograms 
	    if {$ismaxhisto == 1} {
		dputs "  Computation of maxima's histograms ($new_num)."

		cd ${dir}/../histograms
		max_histos $dir $new_num
	    }
	}
    }
    
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]
	    delete max$new_num
	}
    }
    
}

proc compute_mexican_max_stat {dir} {
    source ${dir}/../parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!

    # Get the size of the border 
    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]
    set border_size [expr int($scale_max*$border_percent)]

    cd $dir

    puts ""
    nputs "Vertical chain of the mexican maxima."
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    # Setting of local parameters
	    set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "  Octave $oct - vox $vox - scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    set prev_num [expr $num-1]
	    set prev_new_num [format "%.3d" $prev_num]

	    eload mex$new_num

	    # We remove the border of the ext_image
	    set border [expr $size-$border_size]
	    rm_ext mex$new_num mex$new_num $border_size $border $border_size $border

	    # Research of maxima lines and maxima line maxima (yes, don't laugh).
	    set box_size [expr int(log($scale)*2/log(2))]
	    if { $prev_num == 0} {
		vchain2 mex$prev_new_num mex$new_num $box_size $similitude -first
	    } else {
		if {$prev_num > 0} {
		    vchain2 mex$prev_new_num mex$new_num $box_size $similitude
		}
	    }
	}   
    }

    puts ""
    nputs "Mexican maxima partition functions and histogramms."
    cd ${dir}/../partition
    
    sw_add_fqaqtq mex $amin $noct $nvox $q_lst mex_part

    puts ""

    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]
	    # We compute the differents maxima's histograms 
	    if {$ismaxhisto == 1} {
		dputs "  Computation of maxima's histograms ($new_num)."

		cd ${dir}/../histograms
		pwd
		max_mexican_histos $dir $new_num
	    }
	}
    }
    
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    set new_num [format "%.3d" $num]
	    delete mex$new_num
	}
    }
    
}

proc set_hist_to_null {dir} {
    source ${dir}/parameters.tcl
    # !!!!!! Rajouter tests existence/validite des parametres !!!!!!!!
    set hist_file_name ${dir}/../min_max_hist

    # Get histograms parameters
    set hist_file_name min_max_hist
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    set pi 3.1416
    set m_pi -3.1416

    cd ${dir}
    # Initialisation of the global histograms at each scale (with gaussian).
    sinus histo_tmp 100
    shisto histo_tmp h_mod_tmp $nbox_mod -x $hist_min $hist_max
    shisto histo_tmp h_arg_tmp $nbox_arg -x $m_pi $pi
    sscamult h_mod_tmp 0.0 h_mod_tmp
    sscamult h_arg_tmp 0.0 h_arg_tmp

    inull calendos_temp $calendos_size
    cd ${dir}/histograms
    for { set oct 0;set num 0}\
            { $oct < $noct} \
            { incr oct } {
        for {set vox 0} \
                { $vox < $nvox } \
                { incr vox ; incr num} {
            set new_num [format "%.3d" $num]

            #  Gradient histograms from all points (mod, arg, gradient vector)
            ssave h_mod_tmp h_mod$new_num -sw
            ssave h_arg_tmp h_arg$new_num -sw
            isave calendos_temp calendos_image$new_num

            #  Gradient histograms from all points on lines (mod, arg,
            # gradient vector)
            ssave h_mod_tmp h_max_mod$new_num -sw
            ssave h_arg_tmp h_max_arg$new_num -sw

            isave calendos_temp calendos_max$new_num

            #  Gradient histograms from all max on lines (mod, arg, gradient
            # vector)
            ssave h_mod_tmp h_max_line_mod$new_num -sw
            ssave h_arg_tmp h_max_line_arg$new_num -sw
            isave calendos_temp calendos_max_line$new_num

        }
    }
    delete histo_tmp
    delete h_mod_tmp
    delete h_arg_tmp
    delete calendos_temp

    cd ${dir}
    # Initialisation of the global histograms at each scale (mexican).
    set hist_file_name min_max_hist_mexican
    set hist_file_id [open $hist_file_name r]
    gets $hist_file_id hist_min
    gets $hist_file_id hist_max
    close $hist_file_id

    sinus histo_tmp 100
    shisto histo_tmp h_mexican_tmp $nbox_mod -x $hist_min $hist_max

    sscamult h_mexican_tmp 0.0 h_mexican_tmp

    cd ${dir}/histograms
    for { set oct 0;set num 0}\
            { $oct < $noct} \
            { incr oct } {
        for {set vox 0} \
                { $vox < $nvox } \
                { incr vox ; incr num} {
            set new_num [format "%.3d" $num]
            ssave h_mexican_tmp h_mexican$new_num -sw
            ssave h_mexican_tmp h_max_mexican$new_num -sw 
        }
    }
    delete histo_tmp
    delete h_mexican_tmp

    cd ${dir}
}
