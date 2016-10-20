# partFcts.tcl --
#
#       This file implements the Tcl code for partition functions handling.
#
#   Copyright (c) 1998-1999 Nicolas Decoster.
#   Copyright (c) 1998-1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#
#   Copyright (c) 1999-2007 Pierre Kestener.
#   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
#   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
#
#

# last modified by Pierre Kestener (2000/06/21).
# modified by Pierre Kestener (between 2001/03/09 and 2001/03/20)
# to include Tsallis entropies computation :
# thd_tsallis, compute_tsallis, thdFit_tsallis ...


package provide pf 0.0

namespace eval pf {
    variable pf
    variable currentId

    variable allowedMethodsArray 
    array set allowedMethodsArray {
	"Gradient lines"	2D
	"Gradient max"		2D
	"Gradient max - tag"	2D
	"Gradient max - notag"	2D
	"Gradient max - theta"	2D
	"Mexican"		2D
	"gauss"			1D
	"d1 gauss"		1D
	"d2 gauss"		1D
	"d3 gauss"		1D
	"d4 gauss"		1D
	"morlet"		1D
    }
}


# pf --
# usage : pf args
#
#   Execute a script in the pf namespace.
#
# Parameters :
#   args - a list of arg.
#
# Return value :
#   Result of the execution.

proc pf args {
    if {[llength $args] != 0} {
	set cmd [concat namespace inscope pf $args]
	set code [catch {eval $cmd} result]
	if {$code != 0} {
	    return -code error "pf : $result"
	} else {
	    return $result
	}
    }
}


# pf::create --
# usage : pf::create
#
#   Create a pf structure and init its parameter to non-sense values.
#
# Parameters :
#   none.
#
# Return value :
#   The pf id.

proc pf::create {args} {
    # Get an id and create an array.
    variable currentId

    variable pf
    if ![info exists pf(pfid)] {
	set pf(pfid) 0
    }
    set pfid [namespace current]::[incr pf(pfid)]
    variable $pfid
    upvar 0 $pfid state

    array set state {
	a_min		-1
	n_octave	0
	n_voice		0
	first_octave	-1
	last_octave	-1
	first_voice	-1
	last_voice	-1
	q_lst		{}
	n_study		0
	size		-1
	dimension	-1
	study_method	none
	comments	{no comment}
	qtsa_lst        {}
	qturb_lst       {0 1 2 3}
    }
    set state(baseSigName)	_pf$pf(pfid)

    set currentId $pfid

    return $pfid
}


# pf::destroy --
# usage : pf::destroy pfId
#
#   End a pf and free all memory that can be freed.
#
# Parameters :
#   pfId - the pf id.
#
# Return value :
#   None.

proc pf::destroy {pfid} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    DeletePfSig $pfid
    unset state

    return
}


# pf::clear --
# usage : pf::clear pfId
#
#   Re-init a pf and free all memory that can be freed.
#
# Parameters :
#   pfId - the pf id.
#
# Return value :
#   None.

proc pf::clear {pfid} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }

    DeletePfSig $pfid

    variable $pfid
    upvar 0 $pfid state

    array set state {
	a_min		-1
	n_octave	0
	n_voice		0
	first_octave	-1
	last_octave	-1
	first_voice	-1
	last_voice	-1
	q_lst		{}
	n_study		0
	size		-1
	dimension	-1
	study_method	none
	comments	{no comment}
	qtsa_lst	{}
	qturb_lst       {}
    }

    set currentId $pfid

    return
}


# pf::DeletePfSig - PRIVATE
# usage : DeletePfSig pfId
#
#   Delete all signals assoziated with a pf id.
#
# Parameters :
#   pfId - the pf id.
#
# Return value :
#   None.

proc pf::DeletePfSig {pfid} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set base $state(baseSigName)

    # We delete all the signals that begin with the base name. It might be
    # dangerous. Perhaps (in the future...) I will removes signals by their
    # complete names.
    catch {delete ${base}*}
}


# pf::help --
# usage : pf::help [proc]
#
#   Get help on pf pakage.
#
# Parameters :
#   [proc] - help on a proc.
#
# Return value :
#   Help message.


proc pf::help args {
    if {[llength $args] == 0} {
	set resStr "Pakage pf :

  This pakage handle partition functions. Use the \"pf\" command to execute
scripts (or commands) in the pf pakage. Each group of part functions (associated
to a \"study\") is identified by a pf id. Most of the following commands need
this pf id as a first parameter. if you use the character \"c\" (for current)
instead of the pf id, the command takes the last pf id used.

List of internal commands :
  create
  destroy
  clear
  help
  swLoad
  swSave
  thd
  thd_tsallis
  disp
  disp2
  disphqa
  init
  compute
  save
  load
  localfit
  lfDisp
  localslope
  lsDisp
  Dh
  thdDisp
  taudDisp
  getParam
  setcomments
"
    } else {
	global auto_index

	set cmdName [lindex $args 0]
	set code [catch "set auto_index(::pf::$cmdName)" result]
	if {$code != 0} {
	    return -code error "no command \"$cmdName\" in pakage pf"
	}
	set fileName [lindex $result 1]
	if {[string compare [lindex [file split $fileName] end] "partFcts.tcl"] != 0} {
	    return -code error "no command \"$cmdName\" in pakage pf"
	}

	# Set the default message.
	set resStr "Sorry, no help for `$cmdName'."

	set fileId [open $fileName r]

	while {[gets $fileId line] != -1} {
	    if {[string compare $line "# pf::$cmdName --"] == 0} {
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

    return $resStr
}


# pf::CheckPfid - PRIVATE
# usage : CheckPfid pfId
#
#   Check if a pf id is valid.
#
# Parameters :
#   pfId - the pf id.
#
# Return value :
#   This proc returns the pf id if it is valid, 0 otherwise.

proc pf::CheckPfid {pfid} {
    # For now this proc only check the existence of pfid as an array. In the
    # future we can imagine other checks.

    variable currentId

    if {[string compare $pfid c] == 0} {
	return $currentId
    }

    if [array exists $pfid] {
	set currentId $pfid
	return $pfid
    } else {
	return 0
    }
}


# pf::StateCheck -- PRIVATE
# usage : StateCheck pfId list
#
#   Check if a pf state is valid.
#
# Parameters :
#   pfId - The pf id.
#   list - List of the checks to do.
#
# Return value :
#   This proc returns with error if the pf state is not valid.

proc pf::StateCheck {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -dataExist {
		if {$state(n_study) <= 0} {
		    return -code error "no data"
		}
		set args [lreplace $args 0 0]
	    }
	    -thdDone {
		set q [lindex $state(q_lst) 0]
		set qStr [get_q_str $q]
		set sigName $state(baseSigName)_tau${qStr}
		if {[string compare [gettype $sigName] S]} {
		    return -code error "Z, h and D are not computed (use command \"thd\")"
		}
		set args [lreplace $args 0 0]
	    }
	    -thdFitDone {
		set q [lindex $state(q_lst) 0]
		set qStr [get_q_str $q]
		set sigName $state(baseSigName)_hq
		if {[string compare [gettype $sigName] S]} {
		    return -code error "h(q) and D(q) are not computed (use command \"thdFit\")"
		}
		set args [lreplace $args 0 0]
	    }
	    -localfitDone {
		set name [lindex $args 1]
		if {[array get state localfit,$name,q_lst] == ""} {
		    return -code error "no local fit done for $name (use command \"localfit\")"
		}
		set args [lreplace $args 0 1]
	    }
	    -localslopeDone {
		set name [lindex $args 1]
		set q [lindex $args 2]
		if {[array get state localslope,$name,q_lst] == ""} {
		    return -code error "no local slope done for $name and q = $q (use command \"localslope\")"
		}
		set flag 0
		foreach val $state(localslope,$name,q_lst) {
		    if {$val == $q} {
			set flag 1
			break
		    }
		}
		if {$flag == 0} {
		    return -code error "no local slope done for $name and q = $q (use command \"localslope\")"
		}
		set args [lreplace $args 0 2]
	    }
	    default {
		set args [lreplace $args 0 0]
	    }
	}
    }

    return
}


# pf::swLoad --
# usage : pf::swLoad pfId string
#
#   Load signals that contains STq, logSTq and STqlogT from files in the old sw
# partition function format.
#
# Parameters :
#   pfId   - The pf id.
#   string - Base name of the sw files.
#
# Return value :
#   None.

proc pf::swLoad {pfid fileName} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set base $state(baseSigName)

    if {$state(n_study) == 0} {
	# Create new values
	
	# Read general file.
	set file [open $fileName r]

	set state(n_octave) [lindex [gets $file] 1]
	set state(n_voice) [lindex [gets $file] 1]
	set state(study_method) [lindex [gets $file] 1]

	if {[string compare $state(study_method) "Gauss_dx_dy"] == 0} {
	    set state(study_method) "Gradient lines"
	}
	set state(a_min) [lindex [gets $file] 1]
	gets $file q
	while {$q != ""} {
	    lappend state(q_lst) $q
	    gets $file q
	}
	close $file

	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]

	    # Read existing data.
	    set STq_file	[open ${fileName}_Pq.$q_str r]
	    set logSTq_file	[open ${fileName}_logsP.$q_str r]
	    set STqlogT_file	[open ${fileName}_PqlogP.$q_str r]

	    catch {unset STq_lst}
	    catch {unset logSTq_lst}
	    catch {unset STqlogT_lst}
	    for {set o 0} {$o < $state(n_octave)} {incr o} {
		for {set v 0} {$v < $state(n_voice)} {incr v} {
		    lappend STq_lst	[gets $STq_file]
		    set val [gets $logSTq_file]
		    if {[string compare $val "--.-00000e+01"] == 0} {
			set val 0
		    }
		    lappend logSTq_lst $val
		    lappend STqlogT_lst	[gets $STqlogT_file]
		}
	    }

	    close $STq_file
	    close $logSTq_file
	    close $STqlogT_file

	    screate ${base}_STq_$q_str \
		    0 [expr 1.0/$state(n_voice)] $STq_lst
	    screate ${base}_logSTq_$q_str\
		    0 [expr 1.0/$state(n_voice)] $logSTq_lst
	    screate ${base}_STqlogT_$q_str \
		    0 [expr 1.0/$state(n_voice)] $STqlogT_lst
	}

	set state(first_octave)	0
	set state(last_octave)	[expr $state(n_octave)-1]
	set state(first_voice)	0
	set state(last_voice)	[expr $state(n_voice)-1]
	set state(n_study)	1
	set state(size)		-1
	set state(dimension)	2D
	set state(comments)	$fileName
    } else {
	return -code error "can't add from a sw file"
    }
}


# pf::swSave --
# usage pf::swSave pfId str
#
#   Save the partition functions in the sw format.
#
# Parameters :
#   pfId   - The pf id.
#   string - File name.
#
# Return value :
#   None.

proc pf::swSave {pfid name} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    # Create general file.

    set file [open $name w]
    puts $file "noct $state(n_octave)"
    puts $file "nvoice $state(n_voice)"
    puts $file "wavelet $state(study_method)"
    puts $file "amin $state(a_min)"
    foreach q $state(q_lst) {
	puts $file $q
    }
    close $file

    # Create history file.

    set hist_file [open ${name}_history w]
    for {set i 0} {$i < $state(n_study)} {incr i} {
	puts $hist_file "***Computation on a new signal:"
	puts $hist_file "q Values   \[nb = [llength $state(q_lst)]\]:"
	foreach q $state(q_lst) {
	    puts -nonewline $hist_file "$q "
	}
	puts $hist_file ""
	puts $hist_file "Signal size : [expr $state(n_octave)*$state(n_voice)]"
	puts $hist_file "Signal name : $state(study_method)"
	puts $hist_file ""
    }
    close $hist_file

    set base $state(baseSigName)

    # Create data files.
    foreach q $state(q_lst) {
	set q_str [get_q_str $q]
	
	ssave ${base}_STq_$q_str ${name}_Pq.$q_str -ascii -noheader
	ssave ${base}_logSTq_$q_str ${name}_logsP.$q_str -ascii -noheader
	ssave ${base}_STqlogT_$q_str ${name}_PqlogP.$q_str -ascii -noheader
    }
}


# pf::thd --
# usage: pf::thd pfId
#
#   Compute the Z(a,q), h(a,q) and D(a,q) signals from the STq and STqlogT 
# signals.
#
# Parameters:
#   pfId   - The pf id.
#
# Options:
#   -normalize: devide each Z(a,q) by N(a).
#   -turbulence : compute taulog !!
#
# Return value:
#   None.

proc pf::thd {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    if {$state(n_study) == 0} {
	return -code error "no current data for this pf id"
    }

    set isNormalize no
    set isTurbulence no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -normalize {
		set isNormalize yes
		set args [lreplace $args 0 0]
	    }
	    -turbulence {
		set isTurbulence yes
		set args [lreplace $args 0 0]
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


    set base $state(baseSigName)

    foreach q $state(q_lst) {
	set q_str [get_q_str $q]

	set STq      ${base}_STq_$q_str
	set STqlogT  ${base}_STqlogT_$q_str
	set STq2     ${base}_STq2_$q_str
	set STqlogT2 ${base}_STqlogT2_$q_str

	scomb $STq $STq     log(x)/log(2)              ${base}_tau$q_str -xnull
	scomb $STqlogT $STq x/(log(2)*y)               ${base}_h$q_str   -ynull
	scomb $STqlogT $STq ($q*x-y*log(y))/(y*log(2)) ${base}_D$q_str   -ynull
	
	# compute error bars for tau(q,a) from STq2
	# Y=log(X) ==> Delta(Y)=Delta(X)/X
	set N $state(n_study)
	scomb $STq2 $STq sqrt((x/$N)-(y*y)/$N/$N)/x*$N ${base}_var_tau$q_str
	# compute error bars for h(q,a) from STq2 and STqlogT2
	# Z=X/Y ==> Delta(Z)/Z=Delta(X)/X+Delta(Y)/Y
	scomb $STqlogT2 $STqlogT sqrt((x/$N)-(y*y)/$N/$N)/x*$N __tmp
	scomb ${base}_var_tau$q_str __tmp x+y __tmp
	scomb ${base}_h$q_str __tmp x*y ${base}_var_h$q_str
	# compute error bars for D(q,a) from STq2 and STqlogT2
	# Delta(D)=q*Delta(h)+Delta(tau)
	scomb ${base}_var_h$q_str ${base}_var_tau$q_str $q*x+y ${base}_var_D$q_str

    }

    if {$isTurbulence == "yes"} {
	foreach qturb $state(qturb_lst) {
	    set qturb_str [get_q_str $qturb]
	    set SlogTq  ${base}_SlogTq_$qturb_str
	    #scomb $SlogTq $SlogTq   log(x)/log(2)    ${base}_taulog$qturb_str -xnull
	}
    }

    if {$isNormalize == "yes"} {
	set q_str [get_q_str 0]
	set tau0 ${base}_tau$q_str

	set n [ssize $tau0]
	set dx [sgetx0 $tau0]
	set x0 [sgetx0 $tau0]

	set b $x0
	set e [expr { $x0 + $dx*($n-1) }]

	exprr x2 0.2*x $b $e $n
	scomb $tau0 x2 y+x Na
	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]

	    set tauq ${base}_tau$q_str

	    scomb $tauq Na x-y $tauq
	}
    }
}


# pf::thd_tsallis --
# usage: pf::thd_tsallis pfId real
#
#   Compute the Ztsa(a,q), htsa(a,q) and Dtsa(a,q) signals from the SKqqtas, SKqqtsa_logT and SLqqtsa.
# signals.
#
# Parameters:
#   pfId   - The pf id.
#   real   - Tsallis parameter.
#
# Options:
#   -normalize: devide each Ztsa(a,q) by N(a).
#
# Return value:
#   None.

proc pf::thd_tsallis {pfid qtsa args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    if {$state(n_study) == 0} {
	return -code error "no current data for this pf id"
    }

    set isNormalize no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -normalize {
		set isNormalize yes
		set args [lreplace $args 0 0]
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


    set base $state(baseSigName)
    set qtsa_str [get_q_str $qtsa]
    
    # q    : temperature
    # qtsa : parametre de tsallis

    # initialisation
    foreach q $state(q_lst) {
	set q_str [get_q_str $q]
	set SKqqtsa      ${base}_SKqqtsa_${q_str}_${qtsa_str}
	set SKqqtsa_logT ${base}_SKqqtsalogT_${q_str}_${qtsa_str}
	set SLqqtsa      ${base}_SLqqtsa_${q_str}_${qtsa_str}
	
	scomb $SLqqtsa $SLqqtsa        log(x)/log(2)           ${base}_logZtsa${q_str}_${qtsa_str} -xnull
	scomb $SKqqtsa_logT $SKqqtsa   x/(log(2)*y)            ${base}_htsa${q_str}_${qtsa_str}   -ynull
	scomb $SKqqtsa $SLqqtsa (1-(x/pow(y,$qtsa)))/(1-$qtsa) ${base}_Dtsa${q_str}_${qtsa_str}   -ynull
    }
    
#    if {$isNormalize == "yes"} {
#	set q_str [get_q_str 0]
#	set tau0 ${base}_tau$q_str
#
#	set n [ssize $tau0]
#	set dx [sgetx0 $tau0]
#	set x0 [sgetx0 $tau0]
#
#	set b $x0
#	set e [expr { $x0 + $dx*($n-1) }]
#
#	exprr x2 0.2*x $b $e $n
#	scomb $tau0 x2 y+x Na
#	foreach q $state(q_lst) {
#	    set q_str [get_q_str $q]
#
#	    set tauq ${base}_tau$q_str
#
#	    scomb $tauq Na x-y $tauq
#	}
#    }
}


# pf::GetRange --
# usage: pf::GetRange pfId ...
#

proc pf::GetRange {q defAMin defAMax rangeLst} {
    if {$rangeLst == {}} {
	return [list $defAMin $defAMax]
    }

    set zeMin $defAMin
    set zeMax $defAMax
    foreach {q2 aMin aMax} $rangeLst {
	if {$q < $q2} {
	    return [list $zeMin $zeMax]
	}
	set zeMin $aMin
	set zeMax $aMax
    }

    return [list $zeMin $zeMax]
}


# pf::thdFit --
# usage: pf::thdFit pfId real real [-rangelist list] [-onlyerror] [-hrange real real]
#
#   Fit Z, h and D.
#
# Parameters:
#   pfId    - The pf id.
#   2 reals - Range of the fit
#
# Options:
#   -rangelist: Use different ranges of fit depending on the vqlue of q.
#      list - list of triplets: q, min scale, max scale. Example: {-2 1 3 0 0.2
#      2} means that for q up to -2 (exclude) we use the defaukt range, for q
#      in [-2,0[ we use range [1,3] and for q greater or equal to 0 we use range
#      [0.2,2].
#   -onlyerror: Compute only the error for Z, h and D.
#   -qlst     : Use only q_values between qmin and qmax
#      - real: qmin
#      - real: qmax
#   -hrange: use a different range of fit the partition functions in h
#      - real : aMin_h
#      - real : aMax_h 
#
# Return value:
#   None.

proc pf::thdFit {pfid aMin aMax args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set rangeLst {}
    set isOnlyerror no
    set isqlst no

    set aMinH $aMin
    set aMaxH $aMax

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -rangelist {
		set rangeLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -onlyerror {
		set isOnlyerror yes
		set args [lreplace $args 0 0]
	    }
	    -qlst {
		set isqlst yes
		# q_lst2
		set qMin [lindex $args 1]
		set qMax [lindex $args 2]
		set theqlst ""
		foreach q $state(q_lst) {
		    if {$q >= $qMin & $q <= $qMax} {
			lappend theqlst $q
		    }
		}
		#echo $theqlst $state(q_lst)
		set args [lreplace $args 0 2]
	    }
	    -hrange {
		set aMinH [lindex $args 1]
		set aMaxH [lindex $args 2]
		set args [lreplace $args 0 2]
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


    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    if { $isqlst == "yes"} {
	set q_lst $theqlst
    } else {
	set q_lst $state(q_lst)
    }

    set name $state(baseSigName)

    set Z_fitLst [list]
    set sigmaZ_Lst [list]
    set D_fitLst [list]
    set sigmaD_fitLst [list]
    set h_fitLst [list]
    set sigmah_fitLst [list]
    foreach q $q_lst {
	set q_str [get_q_str $q]
	
	mylassign {a_min a_max} [GetRange $q $aMin $aMax $rangeLst]
	mylassign {a_minH a_maxH} [GetRange $q $aMinH $aMaxH $rangeLst]
	# normalisation
	#scomb ${name}_tau$q_str ${name}_tau0p00 x-y ${name}_tau$q_str
	# 
	mylassign {a b sigA} [sfit ${name}_tau$q_str $a_min $a_max]

	lappend Z_fitLst $a
	lappend sigmaZ_fitLst $sigA

	mylassign {a b sigA} [sfit ${name}_D$q_str $a_min $a_max]
	lappend D_fitLst $a
	lappend sigmaD_fitLst $sigA
	#echo toto
	mylassign {a b sigA} [sfit ${name}_h$q_str $a_minH $a_maxH]
	lappend h_fitLst $a
	lappend sigmah_fitLst $sigA
    }
    if {$isOnlyerror == "no"} {
	screate ${name}_tq 0 1 $Z_fitLst -xy $q_lst
	screate ${name}_Dq 0 1 $D_fitLst -xy $q_lst
	screate ${name}_hq 0 1 $h_fitLst -xy $q_lst
    }
    
    screate ${name}_stq 0 1 $sigmaZ_fitLst -xy $q_lst
    screate ${name}_sDq 0 1 $sigmaD_fitLst -xy $q_lst
    screate ${name}_shq 0 1 $sigmah_fitLst -xy $q_lst

    return
}


# pf::ess --
# usage: pf::ess pfId real real
#
#   Fit Z(q,a)/Z(q=0,a)
#
# Parameters:
#   pfId    - The pf id.
#   2 reals - Range of the fit
#
# Options:
#
# Return value:
#   None.

proc pf::ess {pfid aMin aMax args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set rangeLst {}
    set isZ no
    set isBoite no
    set q_lst $state(q_lst)

    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set q_lst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -Z {
		set isZ "yes"
		set args [lreplace $args 0 0]
	    }
	    -boite {
		set isBoite "yes"
		set args [lreplace $args 0 0]
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
    
    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }
    
    #set q_lst $state(q_lst)
    

    set name $state(baseSigName)

    set Z_fitLst [list]
    set sigmaZ_Lst [list]
 
    set completeLst {}
    set sigLst {}

    if {$isBoite == "yes"} {
	s2fs ${name}_tau0p00 dd0 x y-3*x
    } else {
	s2fs ${name}_tau0p00 dd0  x y
    }

    foreach q $q_lst {
	
	set q_str [get_q_str $q]
	
	mylassign {a_min a_max} [GetRange $q $aMin $aMax $rangeLst]
	
	if {$q_str=="0p00"} {
	    echo $q_str
	    scopy dd0 ${name}_tau2$q_str
	    set val [lindex [sget ${name}_tau2${q_str} 0] 0]
	    s2fs ${name}_tau2$q_str ${name}_tau2$q_str x y-$val
	} else {
	    # normalisation
	    if {$isBoite == "yes"} {
		s2fs ${name}_tau$q_str ${name}_tau$q_str x y-0*x
		scomb ${name}_tau$q_str dd0 x-$q*y ${name}_tau2$q_str
		set val [lindex [sget ${name}_tau2${q_str} 0] 0]
		#s2fs ${name}_tau2$q_str ${name}_tau2$q_str x y+(0-3*$q)*x-$val
		s2fs ${name}_tau2$q_str ${name}_tau2$q_str x y-$val
	    } else {
		s2fs ${name}_tau$q_str ${name}_tau$q_str x y
		scomb ${name}_tau$q_str dd0 x-$q*y ${name}_tau2$q_str
		set val [lindex [sget ${name}_tau2${q_str} 0] 0]
		echo tata $q
		s2fs ${name}_tau2$q_str ${name}_tau2$q_str x y-$val
	    }
	
	}  
	mylassign {a b sigA} [sfit ${name}_tau2$q_str $a_min $a_max]
	
	lappend Z_fitLst $a
	lappend sigmaZ_fitLst $sigA
	
	lappend sigLst ${name}_tau2${q_str}
	
    }
    set completeLst [lappend completeLst ${sigLst}]
    
    echo $completeLst

    screate ${name}_tqess 0 1 $Z_fitLst -xy $q_lst

    legendre2 ${name}_tqess -dh ${name}_Dhess
    scomb ${name}_Dhess ${name}_Dhess x+0 ${name}_Dhess
    
    set zeliste $sigLst

    echo qqq $isZ

    if {$isZ=="no"} {
	sdisp ${name}_Dhess
	#mdisp 1 1 [list ${sigLst}]
    } else {
	set code [catch {mdisp 1 1 [list $sigLst]} w]
	if {$code != 0} {
	    error $w $w
	}
	
	${w} switch_allgraph_flag
	${w} gr set_disp_mode all
	
	$w setColorsByList {black red orange yellow green darkgreen blue slateblue brown }
	
	set itemList {}
	foreach value $q_lst {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval $w setLabelsItemsByList $itemList
	
	${w} setLabel "$state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study)\n " 3
	
	${w}gr0000 set_label {black "Z(a, q)/Z^q(a,q=0), q = "} allSigLabel
	
	${w} gr init_disp
	
	${w} gr set_box_coord 0 5 -1.4 1
	
	return $w
    }
}


# pf::thdFit_tsallis --
# usage: pf::thdFit_tsallis pfId real real real [-rangelist list] [-onlyerror]
#
#   Fit Ztsa, htsa and Dtsa.
#
# Parameters:
#   pfId    - The pf id.
#   2 reals - Range of the fit
#   1 real  - qtsa (tsallis parameter)
#
# Options:
#   -rangelist: Use different ranges of fit depending on the vqlue of q.
#      list - list of triplets: q, min scale, max scale. Example: {-2 1 3 0 0.2
#      2} means that for q up to -2 (exclude) we use the defaukt range, for q
#      in [-2,0[ we use range [1,3] and for q greater or equal to 0 we use range
#      [0.2,2].
#   -onlyerror: Compute only the error for Z, h and D.
#   -qlst     : Use only q_values between qmin and qmax
#      - real: qmin
#      - real: qmax
#
# Return value:
#   None.

proc pf::thdFit_tsallis {pfid aMin aMax qtsa args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set rangeLst {}
    set isOnlyerror no
    set isqlst no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -rangelist {
		set rangeLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -onlyerror {
		set isOnlyerror yes
		set args [lreplace $args 0 0]
	    }
	    -qlst {
		set isqlst yes
		# q_lst2
		set qMin [lindex $args 1]
		set qMax [lindex $args 2]
		set theqlst ""
		foreach q $state(q_lst) {
		    if {$q >= $qMin & $q <= $qMax} {
			lappend theqlst $q
		    }
		}
		#echo $theqlst $state(q_lst)
		set args [lreplace $args 0 2]
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

    # evenetuellement modifie ce qui suit en ajoutant un thdtsallisDone !!
    set code [catch {
	StateCheck $pfid \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    if { $isqlst == "yes"} {
	set q_lst $theqlst
    } else {
	set q_lst $state(q_lst)
    }

    set name $state(baseSigName)

    set Ztsa_fitLst       [list]
    set sigmaZtsa_Lst     [list]
    set Dtsa_fitLst       [list]
    set sigmaDtsa_fitLst  [list]
    set htsa_fitLst       [list]
    set sigmahtsa_fitLst  [list]
    
    set qtsa_str [get_q_str $qtsa]
    foreach q $q_lst {
	set q_str [get_q_str $q]
	
	mylassign {a_min a_max} [GetRange $q $aMin $aMax $rangeLst]
#	echo $q $aMin $aMax
	mylassign {a b sigA} [sfit ${name}_logZtsa${q_str}_${qtsa_str} $a_min $a_max]
#	echo $a
	lappend Ztsa_fitLst $a
	lappend sigmaZtsa_fitLst $sigA

	mylassign {a b sigA} [sfit ${name}_Dtsa${q_str}_${qtsa_str} $a_min $a_max]
	lappend Dtsa_fitLst $a
	lappend sigmaDtsa_fitLst $sigA
	#echo toto
	mylassign {a b sigA} [sfit ${name}_htsa${q_str}_${qtsa_str} $a_min $a_max]
	lappend htsa_fitLst $a
	lappend sigmahtsa_fitLst $sigA
    }
    if {$isOnlyerror == "no"} {
	screate ${name}_tq_${qtsa_str} 0 1 $Ztsa_fitLst -xy $q_lst
	screate ${name}_Dq_${qtsa_str} 0 1 $Dtsa_fitLst -xy $q_lst
	screate ${name}_hq_${qtsa_str} 0 1 $htsa_fitLst -xy $q_lst
    }
    
    screate ${name}_stq_${qtsa_str} 0 1 $sigmaZtsa_fitLst -xy $q_lst
    screate ${name}_sDq_${qtsa_str} 0 1 $sigmaDtsa_fitLst -xy $q_lst
    screate ${name}_shq_${qtsa_str} 0 1 $sigmahtsa_fitLst -xy $q_lst

    return
}


# pf::disp --
# usage : pf::disp pfId [-qlist list] [-h float]
#
#  Display Z(a,q), h(a,q), and D(a,q) signals.
#
# Parameters :
#   pfId   - The pf id.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     : Display h(a,q)-h0*ln(a)
#     h0   - float
#   -tsallis real : Display logZtsa(a,q), htsa(a,q) and Dtsa(a,q)
#                   for some value of tsallis parameter.
#
# Return value :
#   Name of the object that handle the window.

proc pf::disp {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state(q_lst)
    set base $state(baseSigName)
    set h0 0
    set ish no
    set isTsallis no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -tsallis {
		set qtsa [lindex $args 1]
		set args [lreplace $args 0 1]
		set isTsallis yes
		set qtsa_str [get_q_str $qtsa]
		#dputs "istsallis $isTsallis"
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

    # Create the list of signals to pass to mdisp command.
    set completeLst {}
    set sigLst {}

    if {$isTsallis == "yes"} {
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_logZtsa${q_str}_${qtsa_str}]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set value_lst {}
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_htsa${q_str}_${qtsa_str}]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_Dtsa${q_str}_${qtsa_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	
	#if {[gettype ${base}_tq_${qtsa_str}] == "S"} {
	#    lappend completeLst [list ${base}_tq ${base}_hq ${base}_Dq]
	#}
	
	set code [catch {mdisp 1 3 ${completeLst}} w]
	if {$code != 0} {
	    error $w $w
	}
	
	${w} switch_allgraph_flag
	${w} gr set_disp_mode all
	if {$state(dimension) == "2D"} {
	    set source "image(s)"
	} else {
	    set source "signal(s)"
	}
	${w} setLabel "Tsallis : qtsallis = $qtsa; $state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	$w setColorsByList {black red green blue yellow brown slateblue}
	
	set itemList {}
	foreach value $qLst {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval $w setLabelsItemsByList $itemList
	
	${w}gr0000 set_label {black "logZtsa(a, q), q = "} allSigLabel
	${w}gr0002 set_label {black "Dtsa(a, q), q = "} allSigLabel
	${w}gr0001 set_label {black "htsa(a, q)-h0*ln(a), q = "} allSigLabel
    } else {	
	# create signal h0loga
	set q0 [lindex $qLst 0]
	set q0_str [get_q_str $q0]
	set a0 [sgetx0 ${base}_h${q0_str}]
	set da0 [sgetdx ${base}_h${q0_str}]
	set asize [ssize ${base}_h${q0_str}]
	
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_tau${q_str}]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set value_lst {}
	    set q_str [get_q_str $q]
	    #set a0 [sgetx0 ${base}_h${q_str}]
	    #set da0 [sgetdx ${base}_h${q_str}]
	    #set asize [ssize ${base}_h${q_str}]
	    if { $ish == "yes"} {
		set cste [lindex [sget ${base}_h${q_str} 0] 0]
	    } else {
		set cste 0
		#echo $cste
	    }
	    for {set k 0} { $k < $asize} {incr k} {
		set hloga_value [expr $h0*($a0+$k*$da0)+$cste]
		lappend value_lst $hloga_value
	    }
	    screate h0loga $a0 $da0 $value_lst
	    
	    scomb ${base}_h${q_str} h0loga x-y ${base}_h${q_str}_disp
	    set sigLst [lappend sigLst ${base}_h${q_str}_disp]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_D${q_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	
	if {[gettype ${base}_tq] == "S"} {
	    lappend completeLst [list ${base}_tq ${base}_hq ${base}_Dq]
	}
	
	set code [catch {mdisp 1 3 ${completeLst}} w]
	if {$code != 0} {
	    error $w $w
	}
	
	${w} switch_allgraph_flag
	${w} gr set_disp_mode all
	if {$state(dimension) == "2D"} {
	    set source "image(s)"
	} else {
	    set source "signal(s)"
	}
	${w} setLabel "$state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	$w setColorsByList {black red green blue yellow brown slateblue}
	
	set itemList {}
	foreach value $qLst {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval $w setLabelsItemsByList $itemList
	
	${w}gr0000 set_label {black "Z(a, q), q = "} allSigLabel
	#${w} gr0000 set_box_coord 0 5 -1.4 1
	
	${w}gr0002 set_label {black "D(a, q), q = "} allSigLabel
	#${w} gr0002 set_box_coord 0 5 -1.4 1
	
	${w}gr0001 set_label {black "h(a, q)-h0*ln(a), q = "} allSigLabel
	#${w} gr0001 set_box_coord 0 5 -1.4 1
    }

    ${w} gr init_disp
    eval ${w} print zhd.eps
    return $w
}


# pf::disp2 --
# usage : pf::disp pfId [-qlist list] [-h float]
#
#  Display h(a,q), and D(a,q) signals.
#
# Parameters :
#   pfId   - The pf id.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     : Display h(a,q)-h0*ln(a)
#     h0   - float
#   -tsallis real : Display logZtsa(a,q), htsa(a,q) and Dtsa(a,q)
#                   for some value of tsallis parameter.
#
# Return value :
#   Name of the object that handle the window.

proc pf::disp2 {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state(q_lst)
    set base $state(baseSigName)
    set h0 0
    set ish no
    set isTsallis no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -tsallis {
		set qtsa [lindex $args 1]
		set args [lreplace $args 0 1]
		set isTsallis yes
		set qtsa_str [get_q_str $qtsa]
		#dputs "istsallis $isTsallis"
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

    # Create the list of signals to pass to mdisp command.
    set completeLst {}
    set sigLst {}

    if {$isTsallis == "yes"} {
	foreach q $qLst {
	    set value_lst {}
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_htsa${q_str}_${qtsa_str}]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_Dtsa${q_str}_${qtsa_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	
	#if {[gettype ${base}_tq_${qtsa_str}] == "S"} {
	#    lappend completeLst [list ${base}_tq ${base}_hq ${base}_Dq]
	#}
	
	set code [catch {mdisp 1 2 ${completeLst}} w]
	if {$code != 0} {
	    error $w $w
	}
	
	${w} switch_allgraph_flag
	${w} gr set_disp_mode all
	if {$state(dimension) == "2D"} {
	    set source "image(s)"
	} else {
	    set source "signal(s)"
	}
	${w} setLabel "Tsallis : qtsallis = $qtsa; $state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	$w setColorsByList {black red green blue yellow brown slateblue}
	
	set itemList {}
	foreach value $qLst {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval $w setLabelsItemsByList $itemList
	
	${w}gr0001 set_label {black "Dtsa(a, q), q = "} allSigLabel
	${w}gr0000 set_label {black "htsa(a, q), q = "} allSigLabel
    } else {	
	# create signal h0loga
	set q0 [lindex $qLst 0]
	set q0_str [get_q_str $q0]
	set a0 [sgetx0 ${base}_h${q0_str}]
	set da0 [sgetdx ${base}_h${q0_str}]
	set asize [ssize ${base}_h${q0_str}]
	
	foreach q $qLst {
	    set value_lst {}
	    set q_str [get_q_str $q]
	    #set a0 [sgetx0 ${base}_h${q_str}]
	    #set da0 [sgetdx ${base}_h${q_str}]
	    #set asize [ssize ${base}_h${q_str}]
	    if { $ish == "yes"} {
		set cste [lindex [sget ${base}_h${q_str} 0] 0]
	    } else {
		set cste 0
		#echo $cste
	    }
	    for {set k 0} { $k < $asize} {incr k} {
		set hloga_value [expr $h0*($a0+$k*$da0)+$cste]
		lappend value_lst $hloga_value
	    }
	    screate h0loga $a0 $da0 $value_lst
	    
	    scomb ${base}_h${q_str} h0loga x-y ${base}_h${q_str}_disp
	    set sigLst [lappend sigLst ${base}_h${q_str}_disp]
	}
	
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst {}
	foreach q $qLst {
	    set q_str [get_q_str $q]
	    set sigLst [lappend sigLst ${base}_D${q_str}]
	}
	set completeLst [lappend completeLst ${sigLst}]
	
	if {[gettype ${base}_tq] == "S"} {
	    lappend completeLst [list ${base}_tq ${base}_hq ${base}_Dq]
	}
	
	set code [catch {mdisp 1 2 ${completeLst}} w]
	if {$code != 0} {
	    error $w $w
	}
	
	${w} switch_allgraph_flag
	${w} gr set_disp_mode all
	if {$state(dimension) == "2D"} {
	    set source "image(s)"
	} else {
	    set source "signal(s)"
	}
	#${w} setLabel "$state(comments)\n($state(a_min), $state(n_octave), \
	#	$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	${w} setLabel "" 
	
	$w setColorsByList {black red green blue yellow brown slateblue}
	
	set itemList {}
	foreach value $qLst {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval $w setLabelsItemsByList $itemList
	
	${w}gr0001 set_label {black "D(a, q), q = "} allSigLabel
	#${w} gr0002 set_box_coord 0 5 -1.4 1
	
	${w}gr0000 set_label {black "h(a, q), q = "} allSigLabel
	#${w} gr0001 set_box_coord 0 5 -1.4 1
    }

    ${w} gr init_disp
    eval ${w} print zhd.eps
    return $w
}


# pf::disp3 --
# usage : pf::disp3 pfId1 pfId2 [-qlist list] [-h float]
#
#  Display Z(a,q), h(a,q), and D(a,q) signals.
#
# Parameters :
#   pfId's   - The pf id's.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     : Display h(a,q)-h0*ln(a)
#     h0   - float
#   -shift : shift partition functions h(a,q) of pfId2 by given parameter. 
#     s    - float
#
# Return value :
#   Name of the object that handle the window.

proc pf::disp3 {pfid1 pfid2 args} {
    set pfid1 [CheckPfid $pfid1]
    if {$pfid1 == 0} {
	return -code error "wrong pf id1"
    }
    variable $pfid1
    upvar 0 $pfid1 state1

    set pfid2 [CheckPfid $pfid2]
    if {$pfid2 == 0} {
	return -code error "wrong pf id2"
    }
    variable $pfid2
    upvar 0 $pfid2 state2

    set code [catch {
	StateCheck $pfid1 \
		-dataExist
	StateCheck $pfid2 \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state1(q_lst)
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    set h0 0
    set ish no
    set isshift no
    set shift 0.0

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -shift {
		set shift [lindex $args 1]
		set args [lreplace $args 0 1]
		set isshift yes
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
    
    # Create the list of signals to pass to mdisp command.
    set completeLst {}
    set sigLst {}
    
    # create signal h0loga
    set q0 [lindex $qLst 0]
    set q0_str [get_q_str $q0]
    set a0 [sgetx0 ${base1}_h${q0_str}]
    set da0 [sgetdx ${base1}_h${q0_str}]
    set asize [ssize ${base1}_h${q0_str}]
    
    foreach q $qLst {
	set q_str [get_q_str $q]
	set sigLst [lappend sigLst ${base1}_tau${q_str}]
	set sigLst [lappend sigLst ${base2}_tau${q_str}]
    }
    
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    foreach q $qLst {
	set value_lst {}
	set q_str [get_q_str $q]
	#set a0 [sgetx0 ${base}_h${q_str}]
	#set da0 [sgetdx ${base}_h${q_str}]
	#set asize [ssize ${base}_h${q_str}]
	if { $ish == "yes"} {
	    set cste [lindex [sget ${base1}_h${q_str} 0] 0]
	} else {
	    set cste 0
	    #echo $cste
	}
	for {set k 0} { $k < $asize} {incr k} {
	    set hloga_value [expr $h0*($a0+$k*$da0)+$cste]
	    lappend value_lst $hloga_value
	}
	screate h0loga $a0 $da0 $value_lst
	
	scomb ${base1}_h${q_str} h0loga x-y ${base1}_h${q_str}_disp
	scomb ${base2}_h${q_str} h0loga x-y-$shift ${base2}_h${q_str}_disp
	set sigLst [lappend sigLst ${base1}_h${q_str}_disp]
	set sigLst [lappend sigLst ${base2}_h${q_str}_disp]
    }
    
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    foreach q $qLst {
	set q_str [get_q_str $q]
	set sigLst [lappend sigLst ${base1}_D${q_str}]
	set sigLst [lappend sigLst ${base2}_D${q_str}]
    }
    set completeLst [lappend completeLst ${sigLst}]
	
    if {[gettype ${base1}_tq] == "S"} {
	lappend completeLst [list ${base1}_tq ${base1}_hq ${base1}_Dq]
    }
    
    set code [catch {mdisp 1 3 ${completeLst}} w]
    if {$code != 0} {
	error $w $w
    }
    
    ${w} switch_allgraph_flag
    ${w} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    ${w} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) \
	    $source\n$state1(study_method)" 3
    
    #$w setColorsByList {red blue red blue red blue red blue red blue red blue red blue red blue }
    $w setColorsByList {red blue magenta slateblue red blue magenta slateblue red blue red blue red blue red blue }
    
    set itemList {}
    foreach value $qLst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $w setLabelsItemsByList $itemList
    
    ${w}gr0000 set_label {black "Z(a, q), q = "} allSigLabel
    #${w} gr0000 set_box_coord 0 5 -1.4 1
    
    ${w}gr0002 set_label {black "D(a, q), q = "} allSigLabel
    #${w} gr0002 set_box_coord 0 5 -1.4 1
    
    ${w}gr0001 set_label {black "h(a, q)-h0*ln(a), q = "} allSigLabel
    #${w} gr0001 set_box_coord 0 5 -1.4 1

    
    ${w} gr init_disp
    #eval ${w} print zhd.eps
    return $w
}

# pf::disp3b --
# usage : pf::disp3 pfId1 pfId2 pfId3 [-qlist list] [-h float]
#
#  Display Z(a,q), h(a,q), and D(a,q) signals.
#
# Parameters :
#   pfId's   - The pf id's.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     : Display h(a,q)-h0*ln(a)
#     h0   - float
#
# Return value :
#   Name of the object that handle the window.

proc pf::disp3b {pfid1 pfid2 pfid3 args} {
    set pfid1 [CheckPfid $pfid1]
    if {$pfid1 == 0} {
	return -code error "wrong pf id1"
    }
    variable $pfid1
    upvar 0 $pfid1 state1

    set pfid2 [CheckPfid $pfid2]
    if {$pfid2 == 0} {
	return -code error "wrong pf id2"
    }
    variable $pfid2
    upvar 0 $pfid2 state2

    set pfid3 [CheckPfid $pfid3]
    if {$pfid3 == 0} {
	return -code error "wrong pf id3"
    }
    variable $pfid3
    upvar 0 $pfid3 state3

    set code [catch {
	StateCheck $pfid1 \
		-dataExist
	StateCheck $pfid2 \
		-dataExist
	StateCheck $pfid3 \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state1(q_lst)
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    set base3 $state3(baseSigName)
    set h0 0
    set ish no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
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
    
    # Create the list of signals to pass to mdisp command.
    set completeLst {}
    set sigLst {}
    
    # create signal h0loga
    set q0 [lindex $qLst 0]
    set q0_str [get_q_str $q0]
    set a0 [sgetx0 ${base1}_h${q0_str}]
    set da0 [sgetdx ${base1}_h${q0_str}]
    set asize [ssize ${base1}_h${q0_str}]
    
    foreach q $qLst {
	set q_str [get_q_str $q]
	set sigLst [lappend sigLst ${base1}_tau${q_str}]
	set sigLst [lappend sigLst ${base2}_tau${q_str}]
	set sigLst [lappend sigLst ${base3}_tau${q_str}]
    }
    
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    foreach q $qLst {
	set value_lst {}
	set q_str [get_q_str $q]
	#set a0 [sgetx0 ${base}_h${q_str}]
	#set da0 [sgetdx ${base}_h${q_str}]
	#set asize [ssize ${base}_h${q_str}]
	if { $ish == "yes"} {
	    set cste [lindex [sget ${base1}_h${q_str} 0] 0]
	} else {
	    set cste 0
	    #echo $cste
	}
	for {set k 0} { $k < $asize} {incr k} {
	    set hloga_value [expr $h0*($a0+$k*$da0)+$cste]
	    lappend value_lst $hloga_value
	}
	screate h0loga $a0 $da0 $value_lst
	
	scomb ${base1}_h${q_str} h0loga x-y ${base1}_h${q_str}_disp
	scomb ${base2}_h${q_str} h0loga x-y ${base2}_h${q_str}_disp
	scomb ${base3}_h${q_str} h0loga x-y ${base3}_h${q_str}_disp
	set sigLst [lappend sigLst ${base1}_h${q_str}_disp]
	set sigLst [lappend sigLst ${base2}_h${q_str}_disp]
	set sigLst [lappend sigLst ${base3}_h${q_str}_disp]
    }
    
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    foreach q $qLst {
	set q_str [get_q_str $q]
	set sigLst [lappend sigLst ${base1}_D${q_str}]
	set sigLst [lappend sigLst ${base2}_D${q_str}]
	set sigLst [lappend sigLst ${base3}_D${q_str}]
    }
    set completeLst [lappend completeLst ${sigLst}]
	
    if {[gettype ${base1}_tq] == "S"} {
	lappend completeLst [list ${base1}_tq ${base1}_hq ${base1}_Dq]
    }
    
    set code [catch {mdisp 1 3 ${completeLst}} w]
    if {$code != 0} {
	error $w $w
    }
    
    ${w} switch_allgraph_flag
    ${w} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    ${w} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) \
	    $source\n$state1(study_method)" 3
    
    $w setColorsByList {red blue black red blue black red blue black red blue black red blue black red blue black red blue black red blue }
    
    set itemList {}
    foreach value $qLst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $w setLabelsItemsByList $itemList
    
    ${w}gr0000 set_label {black "Z(a, q), q = "} allSigLabel
    #${w} gr0000 set_box_coord 0 5 -1.4 1
    
    ${w}gr0002 set_label {black "D(a, q), q = "} allSigLabel
    #${w} gr0002 set_box_coord 0 5 -1.4 1
    
    ${w}gr0001 set_label {black "h(a, q)-h0*ln(a), q = "} allSigLabel
    #${w} gr0001 set_box_coord 0 5 -1.4 1

    
    ${w} gr init_disp
    #eval ${w} print zhd.eps
    return $w
}


# pf::disphaq --
# usage : pf::disphaq pfId [-qlist list] [-h float]
#
#  Display hh(a,q) = h(a,q) [ -h0*ln(a) - hh(a0,q)] signals.
#  notice that : at scale a0 hh(a,q) equals ZERO !!
#  a0 is set at scale 1 (log(a)=1 base 2)
# Parameters :
#   pfId   - The pf id.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     : Display h(a,q)-h0*ln(a)
#     h0   - float (default is zero)
#   -a0    : Display h(a,q)-cste with cste = h(a0,q)
#     oct     - integer (between 0 and noct-1   -> see parameters.tcl)
#     voice   - integer (between 0 and nvoice-1 -> see parameters.tcl)
#            log(a0) = $oct+($voice/double($nvoice))
# Return value :
#   Name of the object that handle the window.

proc pf::disphaq {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state(q_lst)
    set base $state(baseSigName)
    set h0 0

    set ish no
    set isa0 no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -a0 {
		set oct [lindex $args 1]
		set voice [lindex $args 2]
		set args [lreplace $args 0 2]
		set isa0 yes
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
    #echo $h0

    # create signal h0loga
    set q0 [lindex $qLst 0]
    set q0_str [get_q_str $q0]
    set a0 [sgetx0 ${base}_h${q0_str}]
    set da0 [sgetdx ${base}_h${q0_str}]
    set asize [ssize ${base}_h${q0_str}]
    set noct $state(n_octave)
    set nvoice $state(n_voice)

    if { $isa0 == "yes" } {
	set loga0 [expr $oct*$nvoice+$voice]
	if { ($loga0 >= $asize) || ($loga0<0)} {
	    return -code error "wrong oct and voice"
	} else {
	}
    } else {
	set loga0 0
    }


    # Create the list of signals to pass to mdisp command.

    set completeLst {}
    set sigLst {}
    foreach q $qLst {
	set value_lst {}
	set q_str [get_q_str $q]
	
	# on va retrancher hh(a0,q)
	if { $isa0 == "yes"} {
	    set cste [lindex [sget ${base}_h${q_str} $loga0] 0]
	} else {
	    set cste 0
	}
	for {set k 0} { $k < $asize} {incr k} {
	    set hloga_value [expr $h0*($a0+$k*$da0-$loga0*1.0/$nvoice)+$cste]
	    lappend value_lst $hloga_value
	}
	screate h0loga $a0 $da0 $value_lst
	
	scomb ${base}_h${q_str} h0loga x-y ${base}_h${q_str}_disp2
	set sigLst [lappend sigLst ${base}_h${q_str}_disp2]
    }

    set completeLst [lappend completeLst ${sigLst}]

    if {[gettype ${base}_tq] == "S"} {
	lappend completeLst [list ${base}_hq ]
    }

    set code [catch {mdisp 1 1 ${completeLst}} w]
    if {$code != 0} {
	error $w $w
    }

    ${w} switch_allgraph_flag
    ${w} gr set_disp_mode all
    if {$state(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    set a00 [format "%.2f" [expr ($loga0)*1.0/($nvoice)]]

    ${w} setLabel "$state(comments)\n($state(a_min), $state(n_octave), \
	    $state(n_voice)) ; $state(n_study) $source\n h0 = $h0 et \
	    a0 = $a00" 4

    $w setColorsByList {black red green blue yellow brown slateblue}

    set itemList {}
    foreach value $qLst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $w setLabelsItemsByList $itemList

#    ${w}gr0000 set_label {black "Z(a, q), q = "} allSigLabel
#    ${w}gr0002 set_label {black "D(a, q), q = "} allSigLabel
    ${w}gr0000 set_label {black "h(a, q) -h0*ln(a) -h(a0,q), q = "} allSigLabel

    ${w} gr init_disp

    ${w} gr set_box_coord 0 5 -1.4 1
    eval ${w} print haq.eps
    return $w
}


# pf::disp2pf --
# usage : pf::disp2pf pfId1 pfId2 [-qlist list]
#
#  Display some signals corresponding to pfId1 and pfId2.
# Parameters :
#   pfId1 pfId2   - The pf id's.
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#
# Return value :
#   Name of the object that handle the window.

proc pf::disp2pf {pfid1 pfid2 args} {
    set pfid1 [CheckPfid $pfid1]
    set pfid2 [CheckPfid $pfid2]
    if {$pfid1 == 0} {
	return -code error "wrong first pf id"
    }
    if {$pfid2 == 0} {
	return -code error "wrong second pf id"
    }
    variable $pfid1
    upvar 0 $pfid1 state1
    variable $pfid2
    upvar 0 $pfid2 state2

    set code [catch {
	StateCheck $pfid1 \
		-dataExist \
		-thdDone
	StateCheck $pfid2 \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state1(q_lst)
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    set h0 0

    set ish no
    set isa0 no

    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -a0 {
		set oct [lindex $args 1]
		set voice [lindex $args 2]
		set args [lreplace $args 0 2]
		set isa0 yes
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

    # create signal h0loga
    set q0 [lindex $qLst 0]
    set q0_str [get_q_str $q0]
    set a0 [sgetx0 ${base1}_h${q0_str}]
    set da0 [sgetdx ${base1}_h${q0_str}]
    set asize [ssize ${base1}_h${q0_str}]
    set noct $state1(n_octave)
    set nvoice $state1(n_voice)

    if { $isa0 == "yes" } {
	set loga0 [expr $oct*$nvoice+$voice]
	if { ($loga0 >= $asize) || ($loga0<0)} {
	    return -code error "wrong oct and voice"
	} else {
	}
    } else {
	set loga0 0
    }

    # Create the list of signals to pass to mdisp command.
    set completeLst {}
    set sigLst {}
    foreach q $qLst {
	set value_lst {}
	set q_str [get_q_str $q]
	
	# on va retrancher hh(a0,q)
	if { $isa0 == "yes"} {
	    set cste1 [lindex [sget ${base1}_h${q_str} $loga0] 0]
	    set cste2 [lindex [sget ${base2}_h${q_str} $loga0] 0]
	} else {
	    set cste1 0
	    set cste2 0
	}
	for {set k 0} { $k < $asize} {incr k} {
	    set hloga_value [expr $h0*($a0+$k*$da0-$loga0*1.0/$nvoice)]
	    lappend value_lst $hloga_value
	}
	screate h0loga $a0 $da0 $value_lst
	
	scomb ${base1}_h${q_str} h0loga x-y-$cste1 ${base1}_h${q_str}_disp2
	set sigLst [lappend sigLst ${base1}_h${q_str}_disp2]
	scomb ${base2}_h${q_str} h0loga x-y-$cste2 ${base2}_h${q_str}_disp2
	set sigLst [lappend sigLst ${base2}_h${q_str}_disp2]
    }

    set completeLst [lappend completeLst ${sigLst}]

    if {[gettype ${base1}_tq] == "S"} {
	lappend completeLst [list ${base1}_hq ]
    }

    set code [catch {mdisp 1 1 ${completeLst}} w]
    if {$code != 0} {
	error $w $w
    }

    ${w} switch_allgraph_flag
    ${w} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    set a00 [format "%.2f" [expr ($loga0)*1.0/($nvoice)]]

    ${w} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) $source\n h0 = $h0 et \
	    a0 = $a00" 4

    $w setColorsByList {red blue red blue red blue red blue}

    set itemList {}
    foreach value $qLst {
	set itemList [lappend itemlist [list %c $value]]
    }
    eval $w setLabelsItemsByList $itemList

#    ${w}gr0000 set_label {black "Z(a, q), q = "} allSigLabel
#    ${w}gr0002 set_label {black "D(a, q), q = "} allSigLabel
    ${w}gr0000 set_label {black "h(a, q) -h0*ln(a) -h(a0,q), q = "} allSigLabel

    ${w} gr init_disp

    ${w} gr set_box_coord 0 5 -1.4 1
    eval ${w} print haq.eps
    return $w
}


# pf::integrate_haq --
# usage pf::integrate_haq pfId integer integer
#
#  Compute the integrale value of function h(a,q)
#  between a_min and a_max for each value of q.
# Parameters :
#   pfId     - The pf id.
#   integer  - indice of amin
#   integer  - indice of amax
#
# Options :
#   -qlist : Display only some values of q.
#     list - list of the values of q to display. The default is all the values.
#   -h     :
#

proc pf::integrate_haq {pfid amin amax args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set qLst $state(q_lst)
    set base $state(baseSigName)
    set integrale_lst {}
    set h0 0
    set ish no
    set isa0 no
    
    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -h {
		set h0 [lindex $args 1]
		set args [lreplace $args 0 1]
		#echo $h0
		set ish yes
	    }
	    -a0 {
		set oct [lindex $args 1]
		set voice [lindex $args 2]
		set args [lreplace $args 0 2]
		set isa0 yes
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

    set noct $state(n_octave)
    set nvoice $state(n_voice)
    set q0 [lindex $qLst 0]
    set q0_str [get_q_str $q0]
    set asize [ssize ${base}_h${q0_str}]

    if { $isa0 == "yes" } {
	set loga0 [expr $oct*$nvoice+$voice]
	if { ($loga0 >= $asize) || ($loga0<0)} {
	    return -code error "wrong oct and voice"
	} else {
	}
    } else {
	set loga0 0
    }
    
    foreach q $qLst {
	set value_lst {}
	set q_str [get_q_str $q]
	set a0 [sgetx0 ${base}_h${q_str}]
	set da0 [sgetdx ${base}_h${q_str}]
	set asize [ssize ${base}_h${q_str}]

	# take care sintegrate only sum y_value of signal
	# without multiplying by dx !!
		# on va retrancher hh(a0,q)
	if { $isa0 == "yes"} {
	    set cste [lindex [sget ${base}_h${q_str} $loga0] 0]
	} else {
	    set cste 0
	}
	for {set k 0} { $k < $asize} {incr k} {
	    set hloga_value [expr $h0*($a0+$k*$da0-$loga0*1.0/$nvoice)+$cste]
	    lappend value_lst $hloga_value
	}
	screate h0loga $a0 $da0 $value_lst
	scomb ${base}_h${q_str} h0loga x-y ${base}_h${q_str}_2

	#echo [lindex [sget ${base}_h${q_str}_2 0] 0]

	# take care sintegrate sums  y_value of signal (without multiplying
	# the result by dx)
	sintegrate ${base}_h${q_str}_2 int_${base}_h${q_str}
	set thevalue [expr [lindex [sget int_${base}_h${q_str} $amax] 0] - [lindex [sget int_${base}_h${q_str} $amin] 0]]
	set thevalue [expr 2*$thevalue/($amax-$amin)/($amax-$amin+1)/$da0]
	lappend integrale_lst $thevalue
    }
    
    foreach q $qLst {
	set q_str [get_q_str $q]
	delete int_${base}_h${q_str}
    }
#    delete thevalue

    return $integrale_lst
}


# pf::get_haq_value --
# usage pf::get_haq_value pfId integer float
#
#  Returns the value of function h(a=a0,q=q0)
#
# Parameters :
#   pfId     - The pf id.
#   integer  - indice of a0
#   float    - value of q0
#
# Options :
#

proc pf::get_haq_value {pfid thea theq args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state
    
    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	    return -code error $result
	}

    set qLst $state(q_lst)
    set base $state(baseSigName)
    
    # Arguments analysis

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set qLst [lindex $args 1]
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

    set q_str [get_q_str $theq]

    set haq_value [lindex [sget ${base}_h${q_str} $thea] 0]	    	    
    return $haq_value
}
	    

# pf::init --
# usage pf::init pfId float int int list int string string args
#
#  Init the parameters of a pf.
#
# Parameters :
#   pfId   - The pf id.
#   float  - The first scale.
#   int    - The number of octaves.
#   int    - The number of voice.
#   list   - The list of values of q (floats).
#   int    - The size of the original data.
#   string - The method used to create partition functions data. Must be one of
#            the following : "Gradient lines", "Gradient max", "Gradient max 
#            -tag", "Gradient max - notag", "Gradient max - theta", "Mexican", #            "gauss" or any kind of 1d wavelet.
#   string - Comments.
#
# Options :
#   -tsallis    list : init the state variable state(qtsa_lst)
#   -turbulence
#
# Return value :
#   None.

proc pf::init {pfid amin noctave nvoice qlst s met comments args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set isTsallis no
    set isTurbulence no

    while {[llength $args] != 0} {
	switch -exact -- [lindex $args 0] {
	    -tsallis {
		set isTsallis yes
		set  qtsalst [lindex $args 1]
		#echo $qtsalst
		set args [lreplace $args 0 1]
	    }
	    -turbulence {
		set isTurbulence yes
		set args [lreplace $args 0 0]
	    }
	    default {
		#lappend options [lindex $args 0]
		set args [lreplace $args 0 0]
	    }
	}
    }

    if {$state(n_study)  != 0} {
	return -code error \
		"parameters already initialised for this pf id"
    }
    if {$amin < 1} {
	return -code error \
		"bad first scale value"
    }
    if {$noctave < 1} {
	return -code error "bad value of parameter \"noctave\""
    }
    if {$nvoice < 1} {
	return -code error "bad value of parameter \"nvoice\""
    }
    if {$s < 1} {
	return -code error "bad value of parameter \"s\""
    }

    variable allowedMethodsArray
    set str [lrange $met 0 3]
    set elt [array get allowedMethodsArray $str]
    if {$elt == ""} {
	return -code error "bad method"
    }

    set state(a_min)		$amin
    set state(n_octave)		$noctave
    set state(n_voice)		$nvoice
    set state(first_octave)	0
    set state(last_octave)	[expr $noctave-1]
    set state(first_voice)	0
    set state(last_voice)	[expr $nvoice-1]
    set state(q_lst)		$qlst
    set state(size)		$s
    set state(dimension)	[lindex $elt 1]
    set state(study_method)	$met
    set state(comments)		$comments
    if {$isTsallis == "yes"} {
	set state(qtsa_lst) $qtsalst
	#echo $state(qtsa_lst)
    }
    if {$isTurbulence == "yes"} {
	#echo $state(qturb_lst) 
    }
    
    DeletePfSig $pfid

    return
}


# pf::OnePf -- PRIVATE
# usage pf::onePf pfId str str str str args [-logS str]
#
#   Compute one partition function from data and add the result to a pf. This
# command creates a signal for each value of q. The mathematical expression is
# computed for each point of one scale, then we summ all the points. This gives
# one point of the signal. This computation is add to the current values of
# the signal.
#
# Parameters :
#   pfId   - The pf id.
#   string - Base name of the source object (signals, images or ext images)
#            that contains the data.
#   string - Mathematical expression (a la defunc) of the function to apply to
#            data at one scale. 
#   string - Name that identifies this expression (function). Example : STq,
#            STqlogT, ...
#   args   - Additionnal options to pass to the command (sfct, ...).
#
# Options :
#   -logS : Compute an other signal with the log of the sum before adding it to
#           the current values of the signal. With this option one can compute
#           STq and logSTq at the same time.
#     string - Name that identifies this 2nd function. Example : logSTq.
#   -turbulence
#   -firstsid [d] : see "pf compute" command...
#
# Return value :
#   None.

proc pf::OnePf {pfid sourceBaseName mathExpr fctName args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state


    dputs "one pf $args"


    set isLogS 0
    set options ""
    set isTurbulence no
    set firstSid 0
    set lastSid [expr {  $state(n_octave)* $state(n_voice) }]


    while {[llength $args] != 0} {
	switch -exact -- [lindex $args 0] {
	    -logS {
		set isLogS 1
		set logFctName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -turbulence {
	    	set isTurbulence yes
		set args [lreplace $args 0 0]
	    }
	    -firstsid {
		set firstSid [lindex $args 1]
		set args [lreplace $args 0 1]
		dputs "onepf firstsid $firstSid"
	    }
	    default {
		lappend options [lindex $args 0]
		set args [lreplace $args 0 0]
	    }
	}
    }
    set zeId [format "%.3d" $firstSid]
    dputs "zeId $zeId $options"
    switch [gettype ${sourceBaseName}${zeId}] {
	S {
	    set cmdName sfct
	}
	I {
	    set cmdName ifct
	}
	E {
	    set cmdName efct
	}
	default {
	    return -code error "wrong data object"
	}
    }



    if {$isTurbulence == "yes"} {
	set the_q_liste $state(qturb_lst)
	#echo $the_q_liste
    } else {
	set the_q_liste $state(q_lst)
    }

    foreach q $the_q_liste {
	set q_str [get_q_str $q]
	
	#dputs "$cmdName : q $q"
	
        catch {unset theList}
	set memSid 0
	for { set oct 0;set num 0} { $oct < $state(n_octave)} { incr oct } {
	    for {set vox 0} { $vox < $state(n_voice) } { incr vox ; incr num; incr memSid} {
		if {$memSid < $firstSid || $memSid > $lastSid} {
		    continue
		}
		set new_num [format "%.3d" $num]
		#dputs "$cmdName : q $q ${new_num}"
		set theVal [eval $cmdName ${sourceBaseName}${new_num} $mathExpr $q $options]
		lappend theList $theVal
		if {$isTurbulence == "yes" && $q == 0} {
			#dputs "$cmdName ${sourceBaseName}${new_num} $mathExpr $q $options $theVal"
		}
	    }
	}

	# Add the lists to signal.
	set base $state(baseSigName)

	# fctName is either STq or STqlogT 
	set sigName ${base}_${fctName}_$q_str 
	if {[string compare [gettype $sigName] S]} {
	    screate $sigName 0 [expr 1.0/$state(n_voice)] $theList
	} else {
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $theList
	    sadd $sigName __tmp $sigName
	}

	if {$isLogS} {
	    set sigName ${base}_${logFctName}_$q_str
	    set theList2 ""
	    #dputs "thelist $theList"
	    foreach val $theList {
		lappend theList2 [expr ($val?log($val):0)]
	    }
	    if {[string compare [gettype $sigName] S]} {
		screate $sigName 0 [expr 1.0/$state(n_voice)] $theList2
	    } else {
		screate __tmp 0 [expr 1.0/$state(n_voice)] $theList2
		sadd $sigName __tmp $sigName
	    }
	}

	catch {delete __tmp}
    }

#    if {$isTurbulence == "yes"} {
#	set base $state(baseSigName)
#	set q0_str [get_q_str 0]
#	set sigName0 ${base}_${fctName}_$q0_str
#	foreach q {1 2 3} {
#	    set q_str [get_q_str $q]
#	    set sigName ${base}_${fctName}_$q_str
#	    scomb $sigName0 $sigName y/x $sigName
#	}
#    }
    

}


## ATTENTION
# il y a des trucs pas tres propres dans tout ce qui suit!!!
# en particulier l'option -SK_logT qui est passe en option a
# la commande efct_tsallis, et qui sert au calcul des signaux
# SKqqtsa_logT, qui est le denominateur de l'energie de tsallis.


# pf::OnePf_tsallis -- PRIVATE
# usage pf::onePf_tsallis pfId str str str str args [-logS str]
#
#   Compute one tsallis partition function from data and add the result
# to a pf. This command creates a signal for each value of q (temperature)
# and qtsa (tsallis parameter). The mathematical expression is
# computed for each point of one scale, then we summ all the points. This gives
# one point of the signal. This computation is add to the current values of
# the signal.
#
# TAKE CARE THAT :
# As for now, this is ONLY for partition functions on WTMMM.!!!
# This routine is called by pf::compute.
#
# Parameters :
#   pfId   - The pf id.
#   string - Base name of the source object (signals, images or ext images)
#            that contains the data.
#   string - First  Mathematical expression (a la defunc).
#   string - Second Mathematical expression (a la defunc). 
#   string - Name that identifies this expression (function). Example :
#            SKqqtsa, SKqqtsa_logT, SLqqtsa ...
#   args   - Additionnal options to pass to the command (efct_tsallis, ...).
#
# Options :
#   -logS : Compute an other signal with the log of the sum before adding it to
#           the current values of the signal. With this option one can compute
#           SLqqtsa and logSLqqtsa at the same time.
#     string - Name that identifies this 2nd function. Example : logSLqqtsa.
#  -SK_logT : option to append to efct_tsallis command.
#
#
# Return value :
#   None.

proc pf::OnePf_tsallis {pfid sourceBaseName mathExpr1 mathExpr2 fctName args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    switch [gettype ${sourceBaseName}000] {
	S {
	    set cmdName sfct
	}
	I {
	    set cmdName ifct
	}
	E {
	    set cmdName efct_tsallis
	}
	default {
	    return -code error "wrong data object"
	}
    }

    #echo toto
    set isLogS 0
    set options ""
    while {[llength $args] != 0} {
	switch -exact -- [lindex $args 0] {
	    -logS {
		set isLogS 1
		set logFctName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	     -SK_logT {
		lappend options -SK_logT
		set args [lreplace $args 0 0]
	    }
	    default {
		lappend options [lindex $args 0]
		set args [lreplace $args 0 0]
	    }
	}
    }

    #echo $options

    foreach q $state(q_lst) {
	set q_str [get_q_str $q]
	#echo q $q
	foreach qtsa $state(qtsa_lst) {
	    set qtsa_str [get_q_str $qtsa]

	    #echo $cmdName
	    catch {unset theList}
	    # scale's loop to compute signal values
	    for { set oct 0;set num 0}\
		    { $oct < $state(n_octave)} \
		    { incr oct } {
		for {set vox 0} \
			{ $vox < $state(n_voice) } \
			{ incr vox ; incr num} {
		    set new_num [format "%.3d" $num]
		    set theVal [eval $cmdName ${sourceBaseName}${new_num} $mathExpr1 $mathExpr2 $q $qtsa $options]
		    #echo $theVal
		    lappend theList $theVal
		}
	    }
	    #echo toto $theList

	    # Add the lists to signal.
	    #
	    
	    set base $state(baseSigName)
	    # example of a valid base : _pf1

	    # we create signal; fctname := SKqqtsa for example.
	    set sigName ${base}_${fctName}_${q_str}_${qtsa_str}
	    
	    if {[string compare [gettype $sigName] S]} {
		screate $sigName 0 [expr 1.0/$state(n_voice)] $theList
	    } else {
		screate __tmp 0 [expr 1.0/$state(n_voice)] $theList
		sadd $sigName __tmp $sigName
	    }
	    
	    if {$isLogS} {
		set sigName ${base}_${logFctName}_$q_str
		set theList2 ""
		foreach val $theList {
		    lappend theList2 [expr ($val?log($val):0)]
		}
		if {[string compare [gettype $sigName] S]} {
		    screate $sigName 0 [expr 1.0/$state(n_voice)] $theList2
		} else {
		    screate __tmp 0 [expr 1.0/$state(n_voice)] $theList2
		    sadd $sigName __tmp $sigName
		}
	    }
	    
	    catch {delete __tmp}
	}
    }
}


# pf::compute --
# usage pf::compute pfId str
#
#   Compute the partition functions from data and add the result to a pf.
#
# Parameters :
#   pfId   - The pf id.
#   string - Base name of the source object (signals, images or ext images) that
#            contains the data.
#   options :
#     -domain [dddd] : use only maxima that are inside a given domain
#                      (specified as in ecut command).
#     -tsallis       : compute tsallis partition functions.
#     -turbulence    : compute cumulants partition functions.
#     -firstsid [d]  : to specify the scale at which we start computations
#                      (see same option in "chain" command).
#
# Return value :
#   The number of studies.
#
# modified by pierre kestener on 12 feb 2001
# option -domain [dddd]
#
# modified by pierre kestener on 21 mar 2001
# option -tsallis
# abandoned !
#
# modified by pierre kestener on 07 nov 2001
# option -turbulence
# abandoned !
#
# modified by PK (21/04/2006)
# option -mask [I]
#

proc pf::compute {pfid sourceBaseName args} {

    # exemple : pfid           -> ::pf::1
    #         : sourceBaseName -> m
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set istsallis no
    set isTurbulence no
    set firstSid 0

    set options ""
    while {[llength $args] != 0} {
	switch -exact -- [lindex $args 0] {
	    -domain {
		lappend options -domain
		set x1 [lindex $args 1]
		set y1 [lindex $args 2]
		set x2 [lindex $args 3]
		set y2 [lindex $args 4]
		lappend options $x1 $y1 $x2 $y2
		set args [lreplace $args 0 4]
	    }
	    -mask {
		lappend options -mask
		set mask [lindex $args 1]
		lappend options $mask
		set args [lreplace $args 0 1]
	    }
	    -tsallis {
	    	set istsallis yes
	    	set args [lreplace $args 0 0]
	    }
	    -turbulence {
	    	set isTurbulence yes
		set args [lreplace $args 0 0]
	    }
	    -dx {
		lappend options "-dx"
		set args [lreplace $args 0 0]
	    }
	    -dy {
		lappend options "-dy"
		set args [lreplace $args 0 0]
	    }
	    -firstsid {
		set firstSid [lindex $args 1]
		set args [lreplace $args 0 1]
		lappend options "-firstsid"
		lappend options "$firstSid"
		#dputs "compute firstsid $firstSid"
	    }
	    default {
		set args [lreplace $args 0 0]
	    }
	}
    }
    
    switch -glob $state(study_method) {
	{Gradient max} {
	    lappend options "-vc"
	}
	{Gradient max - tag} {
	    lappend options -vc -tag
	}
	{Gradient max - notag} {
	    lappend options -vc -notag
	}
	{Gradient max - theta * dtheta *} {
	    set theta  [lindex $state(study_method) 4]
	    set dTheta [lindex $state(study_method) 6]
	    set pi [expr { acos(-1) }]
	    set simil [expr 1-$dTheta/(2*$pi)]
	    lappend options -vc -arg $theta $simil
	}
	{Mexican} {
	    lappend options "-vc"
	}
    }
    
    if { $istsallis == "yes" } {
	eval OnePf_tsallis $pfid $sourceBaseName x*log(abs(y)) \
		(1+(1-x)*y)^(x/(1-x)) SKqqtsa $options -logS logSKqqtsa
	eval OnePf_tsallis $pfid $sourceBaseName  x*log(abs(y)) \
		(1+(1-x)*y)^(x/(1-x)) SKqqtsalogT -SK_logT $options
	eval OnePf_tsallis $pfid $sourceBaseName x*log(abs(y)) \
		(1+(1-x)*y)^(1/(1-x)) SLqqtsa $options
    } else {
	#eval OnePf $pfid $sourceBaseName exp(y*log(abs(x))) \
	#	STq $options -logS logSTq
	eval OnePf $pfid $sourceBaseName (abs(x))^y \
		STq $options -logS logSTq
	# abs(x)^y means (wavelet_modulus)^q.
	eval OnePf $pfid $sourceBaseName log(abs(x))*(abs(x))^y \
		STqlogT $options
	# log(abs(x))*abs(x)^y means (wavelet_modulus)^q*log(wtm).
	if {$isTurbulence == "yes"} {
	    #eval OnePf $pfid $sourceBaseName exp(y*log(log(abs(x))/log(2))) \
	    #	    SlogTq -turbulence $options 
	    eval OnePf $pfid $sourceBaseName (log(abs(x))/log(2))^y \
		    SlogTq -turbulence $options 
	    # (log(abs(x)))^y means (log of wavelet_modulus)^q.
	}
    }
    incr state(n_study)
}


# pf::save --
# usage pf::save pfId str args
#
#   Save the partition functions.
#
# Parameters :
#   pfId   - The pf id.
#   string - File name.
#
#   options :
#     -tsallis
#     -turbulence
#     -nstudy
#
# Return value :
#   None.

proc pf::save {pfid name args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    # Options analysis.
    set isTsallis no
    set isTurbulence no
    set isNstudy no

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -tsallis {
		set isTsallis yes
		set qtsa [lindex $args 1]
		set args [lreplace $args 0 1]
		set qtsa_str [get_q_str $qtsa]
	    }
	    -turbulence {
		set isTurbulence yes
		set args [lreplace $args 0 0]
	    }
	    -nstudy {
		set isNstudy yes
		set args [lreplace $args 0 0]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    if {[file exists $name]} {
	file rename -force $name $name.bak
    }

    set file [open $name w]

    if {$isTsallis == "yes"} {
	puts $file "Tsallis Partition function"
    } else {
	puts $file "Partition function"
    }
    puts $file "First scale : $state(a_min)"
    puts $file "Number of octaves : $state(n_octave)"
    puts $file "Number of voices : $state(n_voice)"
    puts $file "First octave : $state(first_octave)"
    puts $file "Last octave : $state(last_octave)"
    puts $file "First voice : $state(first_voice)"
    puts $file "Last voice : $state(last_voice)"
    puts $file "Sources size : $state(size)"
    if {$isNstudy == "yes"} {
	puts $file "Number of sources : 1"
    } else {
	puts $file "Number of sources : $state(n_study)"
    }
    puts $file "Source dimension : $state(dimension)"
    puts $file "Method : $state(study_method)"
    puts $file "List of values of q : $state(q_lst)"
    if {$isTsallis == "yes"} {
	puts $file "Tsallis parameter qtsa : $qtsa"
    }
    


    set base $state(baseSigName)

    if {$isTsallis == "yes"} {
	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    puts $file "q : $q"
	    puts $file "SKqqtsa : [sgetlst ${base}_SKqqtsa_${q_str}_${qtsa_str}]"
	    puts $file "SKqqtsalogT : [sgetlst ${base}_SKqqtsalogT_${q_str}_${qtsa_str}]"
	    puts $file "SLqqtsa : [sgetlst ${base}_SLqqtsa_${q_str}_${qtsa_str}]"
	}
    } else {
	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    puts $file "q : $q"
	    puts $file "STq : [sgetlst ${base}_STq_${q_str}]"
	    puts $file "logSTq : [sgetlst ${base}_logSTq_${q_str}]"
	    puts $file "STqlogT : [sgetlst ${base}_STqlogT_${q_str}]"
	}
	if {$isTurbulence == "yes"} {
	    foreach q $state(qturb_lst) {
		set q_str [get_q_str $q]
		puts $file "q : $q"
		puts $file "SlogTq : [sgetlst ${base}_SlogTq_${q_str}]"
	    }
	}
    }
    puts $file "Comments :\n$state(comments)"
    close $file
}


# pf::load --
# usage pf::load pfId str
#
#   Load the partition functions.
#
# Parameters :
#   pfId   - The pf id.
#   string - File name.
#
#   options :
#     -turbulence
#
# Return value :
#   None.

proc pf::load {pfid name args} {
    # Command to rewrite to handle file format problems.

    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    # Options analysis.

    set isForce no
    set isTurbulence no

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -force {
		set isForce yes
		set args [lreplace $args 0 0]
	    }
	    -turbulence {
		set isTurbulence yes
		set args [lreplace $args 0 0]
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

    set base $state(baseSigName)

    set file [open $name r]
    if {$state(n_study) == 0} { # Create new values.
	gets $file l
	if {[string compare $l "Partition function"] != 0} {
	    return -code error "wrong file format"
	}
	set state(a_min)        [lindex [gets $file] end]
	set state(n_octave)     [lindex [gets $file] end]
	set state(n_voice)      [lindex [gets $file] end]
	set state(first_octave) [lindex [gets $file] end]
	set state(last_octave)  [lindex [gets $file] end]
	set state(first_voice)  [lindex [gets $file] end]
	set state(last_voice)   [lindex [gets $file] end]
	set state(size)         [lindex [gets $file] end]
	set state(n_study)      [lindex [gets $file] end]
	set state(dimension)    [lindex [gets $file] end]
	set state(study_method) [lrange [gets $file] 2 end]
	set state(q_lst)        [lrange [gets $file] 6 end]
	set state(qturb_lst)    {0 1 2 3}

	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    gets $file
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_STq_${q_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb ${base}_STq_${q_str} ${base}_STq_${q_str} x*x ${base}_STq2_${q_str}

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_logSTq_${q_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb ${base}_logSTq_${q_str} ${base}_logSTq_${q_str} x*x ${base}_logSTq2_${q_str}

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_STqlogT_${q_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb ${base}_STqlogT_${q_str} ${base}_STqlogT_${q_str} x*x ${base}_STqlogT2_${q_str}	    

	}
	if {$isTurbulence == "yes"} {
	    foreach q $state(qturb_lst) {
		gets $file
		set q_str [get_q_str $q]
		set val_lst [lrange [gets $file] 2 end]
		regsub -all {\+\+} $val_lst 0 val_lst
		regsub -all {\-\-\-} $val_lst 0 val_lst
		screate ${base}_SlogTq_${q_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    }
	}

	gets $file
	gets $file comments
	while {[gets $file str] != -1} {
	    set state(comments) "$comments\n$str"
	}
	close $file
    } else { # Add the new values.
	gets $file
	if {$state(a_min)    != [lindex [gets $file] end]} {return -code error "Bad value"}
	if {$state(n_octave) != [lindex [gets $file] end]} {return -code error "Bad value"}
	if {$state(n_voice)  != [lindex [gets $file] end]} {return -code error "Bad value"}

	# !!!! ATTENTION -> METTRE A JOUR CES VALEURS !!!!!!!!!!!
	set new_first_octave [lindex [gets $file] end]
	set new_last_octave  [lindex [gets $file] end]
	set new_first_voice  [lindex [gets $file] end]
	set new_last_voice   [lindex [gets $file] end]
	
	if {$state(size) != [lindex [gets $file] end]} {return -code error "Bad value"}
	
	incr state(n_study) [lindex [gets $file] end]
	
	if {[string compare $state(dimension) [lindex [gets $file] end]] != 0} {return -code error "Bad value"}
	if {$isForce == "no"} {
	    if {[string compare $state(study_method) [lrange [gets $file] 2 end]] != 0} {return -code error "Bad value for method"}
	} else {
	    gets $file
	}
	if {[string compare $state(q_lst) [lrange [gets $file] 6 end]] != 0} {return -code error "Bad value for q list"}
	
	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    gets $file

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb __tmp __tmp x*x __tmp2
	    sadd ${base}_STq_${q_str} __tmp ${base}_STq_${q_str}
	    sadd ${base}_STq2_${q_str} __tmp2 ${base}_STq2_${q_str}
	    
	    
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb __tmp __tmp x*x __tmp2
	    sadd ${base}_logSTq_${q_str} __tmp ${base}_logSTq_${q_str}
	    sadd ${base}_logSTq2_${q_str} __tmp2 ${base}_logSTq2_${q_str}

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    scomb __tmp __tmp x*x __tmp2
	    sadd ${base}_STqlogT_${q_str} __tmp ${base}_STqlogT_${q_str}
	    sadd ${base}_STqlogT2_${q_str} __tmp2 ${base}_STqlogT2_${q_str}
	}
	if {$isTurbulence == "yes"} {
	    foreach q $state(qturb_lst) {
		gets $file
		set q_str [get_q_str $q]
		set val_lst [lrange [gets $file] 2 end]
		regsub -all {\+\+} $val_lst 0 val_lst
		regsub -all {\-\-\-} $val_lst 0 val_lst
		screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
		sadd ${base}_SlogTq_${q_str} __tmp ${base}_SlogTq_${q_str}
		}
	}

	catch {delete __tmp __tmp2}
	close $file
    }
}


# pf::load_tsallis --
# usage pf::load_tsallis pfId str real
#
#   Load a Tsallis partition functions for a given value
#   of tsallis parameter.
#
# Parameters :
#   pfId   - The pf id.
#   string - File name.
#   real   - value of tsallis parameter
#
# Return value :
#   None.

proc pf::load_tsallis {pfid name qtsa args} {
    # Command to rewrite to handle file format problems.

    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    # Options analysis.

    set isForce no

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -force {
		set isForce yes
		set args [lreplace $args 0 0]
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

    set base $state(baseSigName)
    set qtsa_str [get_q_str $qtsa]

    set file [open $name r]
    if {$state(n_study) == 0} { # Create new values.
	gets $file l
	if {[string compare $l "Tsallis Partition function"] != 0} {
	    return -code error "wrong file format"
	}
	set state(a_min)        [lindex [gets $file] end]
	set state(n_octave)     [lindex [gets $file] end]
	set state(n_voice)      [lindex [gets $file] end]
	set state(first_octave) [lindex [gets $file] end]
	set state(last_octave)  [lindex [gets $file] end]
	set state(first_voice)  [lindex [gets $file] end]
	set state(last_voice)   [lindex [gets $file] end]
	set state(size)         [lindex [gets $file] end]
	set state(n_study)      [lindex [gets $file] end]
	set state(dimension)    [lindex [gets $file] end]
	set state(study_method) [lrange [gets $file] 2 end]
	set state(q_lst)        [lrange [gets $file] 6 end]
	set qtsafile            [lindex [gets $file] end]
	if {$qtsa != $qtsafile} {
	    return -code error "given tsallis parameter is different from the one inside file"
	}
	

	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    gets $file
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_SKqqtsa_${q_str}_${qtsa_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_SKqqtsalogT_${q_str}_${qtsa_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate ${base}_SLqqtsa_${q_str}_${qtsa_str} 0 [expr 1.0/$state(n_voice)] $val_lst
	}

	gets $file
	gets $file comments
	while {[gets $file str] != -1} {
	    set state(comments) "$comments\n$str"
	}
	close $file
    } else { # Add the new values.
	gets $file
	if {$state(a_min)    != [lindex [gets $file] end]} {return -code error "Bad value"}
	if {$state(n_octave) != [lindex [gets $file] end]} {return -code error "Bad value"}
	if {$state(n_voice)  != [lindex [gets $file] end]} {return -code error "Bad value"}

	# !!!! ATTENTION -> METTRE A JOUR CES VALEURS !!!!!!!!!!!
	set new_first_octave [lindex [gets $file] end]
	set new_last_octave  [lindex [gets $file] end]
	set new_first_voice  [lindex [gets $file] end]
	set new_last_voice   [lindex [gets $file] end]
	
	if {$state(size) != [lindex [gets $file] end]} {return -code error "Bad value"}
	
	incr state(n_study) [lindex [gets $file] end]
	
	if {[string compare $state(dimension) [lindex [gets $file] end]] != 0} {return -code error "Bad value"}
	if {$isForce == "no"} {
	    if {[string compare $state(study_method) [lrange [gets $file] 2 end]] != 0} {return -code error "Bad value for method"}
	} else {
	    gets $file
	}
	if {[string compare $state(q_lst) [lrange [gets $file] 6 end]] != 0} {return -code error "Bad value for q list"}
	if {[string compare $qtsa [lindex [gets $file] end]] != 0} {return -code error "Bad value for qTsallis"}

	foreach q $state(q_lst) {
	    set q_str [get_q_str $q]
	    gets $file

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    sadd ${base}_SKqqtsa_${q_str}_${qtsa_str} __tmp ${base}_SKqqtsa_${q_str}_${qtsa_str}
	    
	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    sadd ${base}_SKqqtsalogT_${q_str}_${qtsa_str} __tmp ${base}_SKqqtsalogT_${q_str}_${qtsa_str}

	    set val_lst [lrange [gets $file] 2 end]
	    regsub -all {\+\+} $val_lst 0 val_lst
	    regsub -all {\-\-\-} $val_lst 0 val_lst
	    screate __tmp 0 [expr 1.0/$state(n_voice)] $val_lst
	    sadd ${base}_SLqqtsa_${q_str}_${qtsa_str} __tmp ${base}_SLqqtsa_${q_str}_${qtsa_str}
	}
	catch {delete __tmp}
	close $file
    }
}


# pf::localfit --
# usage : pf::localfit pfId str [-prefscales real real] [-qlist list]
#
#   Compute different local linear regression of the partition functions
# ``name'' of order q. Take windows of size 1/2, 1, 1.5 and 2 dyades.
# create the signals :
#    best: the best slope.
#    mean: the mean slope.
#    min : the min slope
#    max : the max slope
#    [pref: the prefered slope] 
#
# Parameters :
#   pfId     - The pf id.
#   str      - Name of the partition functions (i.e. tau, h or D)
#
# Options :
#   -qlist : Display only some values of q.
#   list  - List of the q values
#   -prefscales : Defines prefered bounds for scales.
#     real - Prefered value of the minimum scale 
#     real - Prefered value of the maximum scale
#
# Return value :
#   The list of the name of the 5 created signals.

proc pf::localfit {pfid name args} { 
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    # Options analysis.

    set prefamin ""
    set prefamax ""
    set q_lst $state(q_lst)

    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -qlist {
		set q_lst [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -prefscales {
		set prefamin [lindex $args 1]
		set prefamax [lindex $args 2]
		set args [lreplace $args 0 2]
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

    set basename $state(baseSigName)

    catch {unset fit_lst}
    catch {unset meanfit_lst}
    catch {unset bestfit_lst}
    catch {unset amin_bestfit_lst}
    catch {unset amax_bestfit_lst}
    catch {unset amax_fitmax_lst}
    catch {unset amin_fitmax_lst}
    catch {unset amax_fitmin_lst}
    catch {unset amin_fitmin_lst}
    if {$prefamin != "" && $prefamax != ""} {
	catch {unset preffit_lst}
    }
    foreach q $q_lst {
	set chi 100.0
	set ss 0.0
	set meantot 0.0
	set q_str [get_q_str $q]
	set num [ssize ${basename}_${name}$q_str]
	set x0 [sgetx0 ${basename}_${name}$q_str]
	set dx [sgetdx ${basename}_${name}$q_str]
	set WindLst {5 10 15 20}
	if {$prefamin != "" && $prefamax != ""} {
	    set newwind [expr $prefamax-$prefamin]
	    set newwind [expr $newwind/$dx]
	    set testwind 0
	    foreach wind $WindLst {
		if {$newwind == $wind} {
		    set testwind 1
		}
	    }
	    if {$testwind == 0} {
		lappend WindLst $newwind
	    }
	}
	foreach wind $WindLst {
	    catch {unset fit_lst}
	    catch {unset x_lst}
	    if {$wind == 20} {
		catch {unset aminfit_lst}
		catch {unset amaxfit_lst}
	    }
	    set numb [expr $num-$wind]
	    for {set i 0} {$i <= $numb} {incr i} {
		set amin [expr $x0+$dx*$i]
		set amax [expr $amin+$wind*$dx]
		set posx [expr ($amin+$amax)/2.0]
		set fit [sfit ${basename}_${name}$q_str $amin $amax]
		set a [lindex $fit 0]
		if {![string compare $a nan]} {
		    set a 0
		}
		lappend fit_lst $a
		lappend x_lst $posx
		if {$amin == $prefamin && $amax == $prefamax} {
		    set preffit $a
		}
		if {$wind == 20} {
		    set chi2 [lindex $fit 4]
		    if {$chi2 <= $chi} {
			set chi $chi2
			set besta $a
			set bestamax $amax
			set bestamin $amin
		    }
		    lappend aminfit_lst $amin
		    lappend amaxfit_lst $amax
		}
	    }
	    if {$wind == 20} {
		lappend bestfit_lst $besta
		lappend amax_bestfit_lst $bestamax
		lappend amin_bestfit_lst $bestamin
		screate amin_temp [lindex $x_lst 0] $dx $aminfit_lst
		screate amax_temp [lindex $x_lst 0] $dx $amaxfit_lst
	    }
	    screate ${basename}_${name}_wind${wind} [lindex $x_lst 0] $dx $fit_lst 
	    set meanvar [sstats ${basename}_${name}_wind${wind} 2]
	    set size [ssize ${basename}_${name}_wind${wind}]
	    set mean [lindex $meanvar 0]
	    set meantot [expr $meantot+$size*$mean]
	    set ss [expr $ss+$size]
	}
	set meantot [expr $meantot/$ss]
	set fit [sgetextr ${basename}_${name}_wind20]
	set fitmin [lindex $fit 0]
	set fitmax [lindex $fit 1]
	set iminfit [lindex $fit 2]
	set imaxfit [lindex $fit 3]
	lappend meanfit_lst $meantot
	lappend minfit_lst $fitmin
	lappend maxfit_lst $fitmax	
	if {$prefamin != "" && $prefamax != ""} {
	    lappend preffit_lst $preffit
	}
	set temp [sget amin_temp $iminfit]
	set amin_fitmin [lindex $temp 0]
	set temp [sget amax_temp $iminfit]
	set amax_fitmin [lindex $temp 0]
	set temp [sget amin_temp $imaxfit]
	set amin_fitmax [lindex $temp 0]
	set temp [sget amax_temp $imaxfit]
	set amax_fitmax [lindex $temp 0]
	lappend amin_fitmin_lst $amin_fitmin
	lappend amax_fitmin_lst $amax_fitmin
	lappend amin_fitmax_lst $amin_fitmax
	lappend amax_fitmax_lst $amax_fitmax
    }
    screate ${basename}_${name}best [lindex $q_lst 0] 1 $bestfit_lst -xy $q_lst
    screate ${basename}_${name}bestamax [lindex $q_lst 0] 1 $amax_bestfit_lst -xy $q_lst
    screate ${basename}_${name}bestamin [lindex $q_lst 0] 1 $amin_bestfit_lst -xy $q_lst

    screate ${basename}_${name}mean [lindex $q_lst 0] 1 $meanfit_lst -xy $q_lst
    screate ${basename}_${name}max [lindex $q_lst 0] 1 $maxfit_lst -xy $q_lst
    screate ${basename}_${name}maxamax [lindex $q_lst 0] 1 $amax_fitmax_lst -xy $q_lst
    screate ${basename}_${name}maxamin [lindex $q_lst 0] 1 $amin_fitmax_lst -xy $q_lst

    screate ${basename}_${name}min [lindex $q_lst 0] 1 $minfit_lst -xy $q_lst
    screate ${basename}_${name}minamax [lindex $q_lst 0] 1 $amax_fitmin_lst -xy $q_lst
    screate ${basename}_${name}minamin [lindex $q_lst 0] 1 $amin_fitmin_lst -xy $q_lst

    if {$prefamin != "" && $prefamax != ""} {
	screate ${basename}_${name}pref [lindex $q_lst 0] 1 $preffit_lst -xy $q_lst
    }

    set state(localfit,$name,q_lst) $q_lst
    set state(localfit,$name,prefamin) $prefamin
    set state(localfit,$name,prefamax) $prefamax

    return
}


# pf::lfDisp --
# usage : pf::lfDisp pfId str
#
#   Display the result of the "localfit" command.
#
# Parameters :
#   pfId - The pf id.
#   name - h, tau or D.
#
# Return value :
#   None.

proc pf::lfDisp {pfid name} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-localfitDone $name
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set prefamin $state(localfit,$name,prefamin)
    set prefamax $state(localfit,$name,prefamax)

    set basename $state(baseSigName)

    set completeLst {}
    if {$prefamin != "" && $prefamax != ""} {
	set sigLst {}
	foreach val {best pref mean max min} {
	    set sigLst [lappend sigLst ${basename}_${name}$val]
	}
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst2 {}
	foreach val {bestamax bestamin maxamax maxamin minamax minamin} {
	    set sigLst2 [lappend sigLst2 ${basename}_${name}$val]
	}
	set completeLst [lappend completeLst ${sigLst2}]
	set code [catch {mdisp 1 2 ${completeLst}} result]
	if {$code != 0} {
	    error $result $result
	}
	${result}gr0000 setColorsByList {red violet cyan green blue}
	set itemList {}
	foreach value {best pref mean max min} {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval ${result}gr0000 setLabelsItemsByList $itemList
	${result}gr0000 set_label {black "fit, "} allSigLabel
    } else {
	set sigLst {}
	foreach val {best mean max min} {
	    set sigLst [lappend sigLst ${basename}_${name}$val]
	}
	set completeLst [lappend completeLst ${sigLst}]
	set sigLst2 {}
	foreach val {bestamax bestamin maxamax maxamin minamax minamin} {
	    set sigLst2 [lappend sigLst2 ${basename}_${name}$val]
	}
	set completeLst [lappend completeLst ${sigLst2}]
	set code [catch {mdisp 1 2 ${completeLst}} result]
	if {$code != 0} {
	    error $result $result
	}
	${result}gr0000 setColorsByList {red cyan green blue}
	set itemList {}
	foreach value {best mean max min} {
	    set itemList [lappend itemlist [list %c $value]]
	}
	eval ${result}gr0000 setLabelsItemsByList $itemList
	${result}gr0000 set_label {black "fit, "} allSigLabel
    }
    ${result}gr0001 setColorsByList {red red green green blue blue}
    set itemList1 {}
    foreach value {best .. max .. min ..} {
	set itemList1 [lappend itemlist1 [list %c $value]]
    }
    eval ${result}gr0001 setLabelsItemsByList $itemList1
    ${result}gr0001 set_label {black "Range for, "} allSigLabel

    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    ${result} gr init_disp
    if {$prefamin != "" && $prefamax != ""} {
	${result} setLabel "Local fit for ${name}(q)  -  pref ($prefamin - $prefamax)"
    } else {
	${result} setLabel "Local fit for ${name}(q)"
    }

    return $result
}


# pf::localslope --
# usage : pf::localslope str str real
#
#   Compute the local linear regression of the partition functions ``name''
# of order q. Take windows of size 1/2, 1, 1.5 and 2 dyades.
#
# Parameters :
#   pfId - The pf id.
#   str  - Name of the partition functions (i.e. tau, h or D).
#   real - The considered q value.
#
# Return value :
#   The list of the name of the 4 created signals.

proc pf::localslope {pfid name q} { 
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set basename $state(baseSigName)

    set q_str [get_q_str $q]
    set num [ssize ${basename}_${name}$q_str]
    set x0 [sgetx0 ${basename}_${name}$q_str]
    set dx [sgetdx ${basename}_${name}$q_str]
    set WindLst {5 10 15 20}

    set sigLst {}
    foreach wind $WindLst {
	catch {unset fit_lst}
	catch {unset x_lst}
	
	set numb [expr $num-$wind]
	for {set i 0} {$i <= $numb} {incr i} {
	    set amin [expr $x0+$dx*$i]
	    set amax [expr $amin+$wind*$dx]
	    set posx [expr ($amin+$amax)/2.0]
	    set fit [sfit ${basename}_${name}$q_str $amin $amax]
	    set a [lindex $fit 0]
	    if {![string compare $a nan]} {
		set a 0
	    }
	    lappend fit_lst $a
	    lappend x_lst $posx
	}
	screate ${basename}_${name}_wind${wind}_$q_str [lindex $x_lst 0] $dx $fit_lst 
	set sigLst [lappend sigLst ${basename}_${name}_wind${wind}_$q_str]
    }

    lappend state(localslope,$name,q_lst) $q

    return $sigLst
}


# pf::lsDisp --
# usage : pf::lsDisp pfId str real
#
#   Display the result of the "localslope" command.
#
# Parameters :
#   pfId - The pf id.
#   name - h, tau or D.
#   real - The considered q value.
#
# Return value :
#   None.

proc pf::lsDisp {pfid name q} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-localslopeDone $name $q
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set basename $state(baseSigName)
    set q_str [get_q_str $q]
    set dx [sgetdx ${basename}_${name}$q_str]

    set WindLst {5 10 15 20}

    set sigLst {}
    foreach wind $WindLst {
	set sigLst [lappend sigLst ${basename}_${name}_wind${wind}_$q_str]
    }

    set completelst {}
    set completeLst [lappend completeLst ${sigLst}]
    set code [catch {mdisp 1 1 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }
    ${result}gr0000 setColorsByList {red cyan green blue}
    set itemList {}
    foreach value $WindLst {
	set value [format "%.5g" [expr $value*$dx]]
	set itemList [lappend itemlist [list %c $value]]
    }
    eval ${result}gr0000 setLabelsItemsByList $itemList
    ${result}gr0000 set_label {black "Window size, "} allSigLabel

    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    ${result} gr init_disp
    ${result} setLabel "Local slope for ${name}($q)"

    return $result
}


# pf::allloc --
#
#   This command is unstable. So if you want to use it, take a look to its
# definition.

proc pf::allloc {pfid name qliste} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set complete_Lst {}
    foreach qq $qliste {
        set sig_Lst {}
        set list [localslope $pfid $name $qq]
        foreach name2 $list {
            set sig_Lst [lappend sig_Lst $name2] 
        }
        set complete_Lst [lappend complete_Lst $sig_Lst]        
    }
    set code [catch {mdisp 3 3 ${complete_Lst}} result]
    if {$code != 0} {
        error $result $result
    }
    ${result} setColorsByList {red cyan green blue}
    set itemList {}    
    foreach value {0.5 1.0 1.5 2.0} {
        set itemList [lappend itemlist [list %c $value]]
    }
    eval ${result} setLabelsItemsByList $itemList
    set r 0
    set l 0
    foreach qq $qliste {
        set newL [format "%.2d" $l]
        set newR [format "%.2d" $r]
        ${result}gr${newR}${newL} set_label [list black "$name q=$qq W. size, "] allSigLabel
        incr l
        if {$l == 3} {
            set l 0
            incr r
        }
    }
    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    ${result} gr init_disp
    ${result} setLabel "Local slope for ${name}(q)"

    return $result
}


# pf::Dh --
# usage : pf::Dh pfId [real real]
#
#  Compute D(h) with D(q) and h(q).
#
# Parameters :
#   pfId     - The pf id.
#   [2 real] - q min and q max.
#
# Return value :
#   None.

proc pf::Dh {pfid {qMin ""} {qMax ""}} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist \
		-thdDone \
		-thdFitDone
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set name $state(baseSigName)

    set hq ${name}_hq
    set Dq ${name}_Dq
    #set hqlst [sgetlst $hq ]
    set hqlst {}
    set tmp1 [sgetlst $hq -x]
    set tmp2 [sgetlst $hq -y]
    foreach data1 $tmp1 data2 $tmp2 {
	lappend hqlst [list $data1 $data2]
    }
    #set Dqlst [sgetlst $Dq ]
    set Dqlst {}
    set tmp1 [sgetlst $Dq -x]
    set tmp2 [sgetlst $Dq -y]
    foreach data1 $tmp1 data2 $tmp2 {
	lappend Dqlst [list $data1 $data2]
    }
    set hlst ""
    set Dlst ""
    set theqlst ""
    foreach q $state(q_lst) {
	if {$q >= $qMin & $q <= $qMax} {
	    lappend theqlst $q
	}
    }

    #echo theqlst $theqlst

    # on peut remplacer dans la ligne suivante theqlst par state(q_lst)
    #puts "$theqlst"
    foreach hq $hqlst Dq $Dqlst q $theqlst {
	if {$qMin == "" || ($q >= $qMin && $q <= $qMax)} {
	    lappend hlst [lindex $hq 1]
	    lappend Dlst [lindex $Dq 1]
	}
    }

    echo dh $Dlst hlst $hlst

    screate ${name}_Dh 0 1 $Dlst -xy $hlst

    return
}


# pf::Dh_tsallis --
# usage : pf::Dh pfId real [real real]
#
#  Compute Dtsa(htsa) with Dtsa(q) and htsa(q) for a given value of
# Tsallis parameter.
#
# Parameters :
#   pfId     - The pf id.
#   real     - Tsallis parameter (must be one of qtsa_lst defined in
#              your file parameters.tcl)
#   [2 real] - q min and q max.
#
# Return value :
#   None.

proc pf::Dh_tsallis {pfid qtsa {qMin ""} {qMax ""}} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set code [catch {
	StateCheck $pfid \
		-dataExist
    } result]
    if {$code == 1} {
	return -code error $result
    }

    set name $state(baseSigName)
    set qtsa_str [get_q_str $qtsa]

    set hqtsa ${name}_hq_${qtsa_str}
    set Dqtsa ${name}_Dq_${qtsa_str}
    set hqtsalst [sgetlst $hqtsa]
    #echo $hqtsalst
    set Dqtsalst [sgetlst $Dqtsa]
    #echo $Dqtsalst
    set htsalst ""
    set Dtsalst ""
    set theqlst ""
    foreach q $state(q_lst) {
	if {$q >= $qMin & $q <= $qMax} {
	    lappend theqlst $q
	}
    }
    # on peut remplacer dans la ligne suivante theqlst par state(q_lst)
    foreach hqtsa $hqtsalst Dqtsa $Dqtsalst q $theqlst {
	if {$qMin == "" || ($q >= $qMin && $q <= $qMax)} {
	    lappend htsalst [lindex $hqtsa 1]
	    lappend Dtsalst [lindex $Dqtsa 1]
	}
    }
    #echo $htsalst
    #echo $Dtsalst
    screate ${name}_Dh_${qtsa_str} 0 1 $Dtsalst -xy $htsalst

    return
}


# pf::thdDisp --
# usage : pf::thdDisp pfId [list]
#
#  Display tau(q), h(q), D(q) and D(h).
#
# Parameters :
#   [list] - one or more of the following : tq, hq, Dq, Dh. Default is all.
#
# Options :
#   -tsallis real : Display logZtsa(q), htsa(q), Dtsa(q) and Dtsa(htsa)
#                   for some value of tsallis parameter.
# Return value :
#   None.

proc pf::thdDisp {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set isTsallis no
    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -tsallis {
		set qtsa [lindex $args 1]
		set args [lreplace $args 0 1]
		set isTsallis yes
		set qtsa_str [get_q_str $qtsa]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    set base $state(baseSigName)

    if {$isTsallis == "yes"} {
	set completeLst [list ${base}_tq_${qtsa_str} ${base}_hq_${qtsa_str} ${base}_Dq_${qtsa_str} ${base}_Dh_${qtsa_str}]
    } else {
	set completeLst [list ${base}_tq ${base}_hq ${base}_Dq ${base}_Dh]
    }

    set code [catch {mdisp 2 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }

    ${result} gr set_disp_mode one
    if {$state(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    if {$isTsallis == "yes"} {
	${result} setLabel "Tsallis : qtsallis = $qtsa; $state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	${result}gr0000 set_label {black "logZtsa(q)"}
	${result}gr0100 set_label {black "Dtsa(q)"}
	${result}gr0001 set_label {black "htsa(q)"}
	${result}gr0101 set_label {black "Dtsa(h)"}
    } else {
	${result} setLabel "$state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	${result}gr0000 set_label {black "tau(q)"}
	${result}gr0100 set_label {black "D(q)"}
	${result}gr0001 set_label {black "h(q)"}
	${result}gr0101 set_label {black "D(h)"}
    }
    ${result} gr init_disp
    eval ${result} print dh.eps
    
    return $result
}


# pf::thdDisp2 --
# usage : pf::thdDisp2 pfId1 pfId2 [list]
#
#  Display tau(q), h(q), D(q) and D(h) for 2 pf's on the same graph's.
#
# Parameters :
#   [list] - one or more of the following : tq, hq, Dq, Dh. Default is all.
#
# Options :
#   -tsallis real : Display logZtsa(q), htsa(q), Dtsa(q) and Dtsa(htsa)
#                   for some value of tsallis parameter.
# Return value :
#   None.

proc pf::thdDisp2 {pfid1 pfid2 args} {
    set pfid1 [CheckPfid $pfid1]
    if {$pfid1 == 0} {
	return -code error "wrong pf id1"
    }
    variable $pfid1
    upvar 0 $pfid1 state1
    
    set pfid2 [CheckPfid $pfid2]
    if {$pfid2 == 0} {
	return -code error "wrong pf id2"
    }
    variable $pfid2
    upvar 0 $pfid2 state2
    
    set isTsallis no
    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }
    
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    
    set completeLst {}
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_tq]
    set sigLst [lappend sigLst ${base2}_tq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_hq]
    set sigLst [lappend sigLst ${base2}_hq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dq]
    set sigLst [lappend sigLst ${base2}_Dq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dh]
    set sigLst [lappend sigLst ${base2}_Dh]
    set completeLst [lappend completeLst ${sigLst}]
    #set completeLst [list [list ${base1}_tq ${base2}_tq] ${base1}_hq ${base1}_Dq ${base1}_Dh]
    
    set code [catch {mdisp 2 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }

    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    
    ${result} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) $source\n$state1(study_method)" 3
    
    $result setColorsByList {red blue yellow brown slateblue}
	
    ${result}gr0000 set_label {black "tau(q)"}
    ${result}gr0100 set_label {black "D(q)"}
    ${result}gr0001 set_label {black "h(q)"}
    ${result}gr0101 set_label {black "D(h)"}
    
    ${result} gr init_disp
    eval ${result} print dh.eps
    
    return $result
}

# pf::thdDisp3 --
# usage : pf::thdDisp3 pfId1 pfId2 pfId3 [list]
#
#  Display tau(q), h(q), D(q) and D(h) for 3 pf's on the same graph's.
#
# Parameters :
#   [list] - one or more of the following : tq, hq, Dq, Dh. Default is all.
#
# Options :
#   -tsallis real : Display logZtsa(q), htsa(q), Dtsa(q) and Dtsa(htsa)
#                   for some value of tsallis parameter.
# Return value :
#   None.

proc pf::thdDisp3 {pfid1 pfid2 pfid3 args} {
    set pfid1 [CheckPfid $pfid1]
    if {$pfid1 == 0} {
	return -code error "wrong pf id1"
    }
    variable $pfid1
    upvar 0 $pfid1 state1
    
    set pfid2 [CheckPfid $pfid2]
    if {$pfid2 == 0} {
	return -code error "wrong pf id2"
    }
    variable $pfid2
    upvar 0 $pfid2 state2
    
    set pfid3 [CheckPfid $pfid3]
    if {$pfid3 == 0} {
	return -code error "wrong pf id3"
    }
    variable $pfid3
    upvar 0 $pfid3 state3
    
    set isTsallis no
    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }
    
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    set base3 $state3(baseSigName)
    
    set completeLst {}
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_tq]
    set sigLst [lappend sigLst ${base2}_tq]
    set sigLst [lappend sigLst ${base3}_tq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_hq]
    set sigLst [lappend sigLst ${base2}_hq]
    set sigLst [lappend sigLst ${base3}_hq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dq]
    set sigLst [lappend sigLst ${base2}_Dq]
    set sigLst [lappend sigLst ${base3}_Dq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dh]
    set sigLst [lappend sigLst ${base2}_Dh]
    set sigLst [lappend sigLst ${base3}_Dh]
    set completeLst [lappend completeLst ${sigLst}]
    #set completeLst [list [list ${base1}_tq ${base2}_tq] ${base1}_hq ${base1}_Dq ${base1}_Dh]
    
    set code [catch {mdisp 2 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }

    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    
    ${result} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) $source\n$state1(study_method)" 3
    
    $result setColorsByList {red blue black brown slateblue}
	
    ${result}gr0000 set_label {black "tau(q)"}
    ${result}gr0100 set_label {black "D(q)"}
    ${result}gr0001 set_label {black "h(q)"}
    ${result}gr0101 set_label {black "D(h)"}
    
    ${result} gr init_disp
    eval ${result} print dh.eps
    
    return $result
}

# pf::thdDisp4 --
# usage : pf::thdDisp4 pfId1 pfId2 pfId3 pfId4 [list]
#
#  Display tau(q), h(q), D(q) and D(h) for 4 pf's on the same graph's.
#
# Parameters :
#   [list] - one or more of the following : tq, hq, Dq, Dh. Default is all.
#
# Options :
#   -tsallis real : Display logZtsa(q), htsa(q), Dtsa(q) and Dtsa(htsa)
#                   for some value of tsallis parameter.
# Return value :
#   None.

proc pf::thdDisp4 {pfid1 pfid2 pfid3 pfid4 args} {
    set pfid1 [CheckPfid $pfid1]
    if {$pfid1 == 0} {
	return -code error "wrong pf id1"
    }
    variable $pfid1
    upvar 0 $pfid1 state1
    
    set pfid2 [CheckPfid $pfid2]
    if {$pfid2 == 0} {
	return -code error "wrong pf id2"
    }
    variable $pfid2
    upvar 0 $pfid2 state2
    
    set pfid3 [CheckPfid $pfid3]
    if {$pfid3 == 0} {
	return -code error "wrong pf id3"
    }
    variable $pfid3
    upvar 0 $pfid3 state3
    
    set pfid4 [CheckPfid $pfid4]
    if {$pfid4 == 0} {
	return -code error "wrong pf id4"
    }
    variable $pfid4
    upvar 0 $pfid4 state4
    
    set isTsallis no
    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }
    
    set base1 $state1(baseSigName)
    set base2 $state2(baseSigName)
    set base3 $state3(baseSigName)
    set base4 $state4(baseSigName)
    
    set completeLst {}
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_tq]
    set sigLst [lappend sigLst ${base2}_tq]
    set sigLst [lappend sigLst ${base3}_tq]
    set sigLst [lappend sigLst ${base4}_tq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_hq]
    set sigLst [lappend sigLst ${base2}_hq]
    set sigLst [lappend sigLst ${base3}_hq]
    set sigLst [lappend sigLst ${base4}_hq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dq]
    set sigLst [lappend sigLst ${base2}_Dq]
    set sigLst [lappend sigLst ${base3}_Dq]
    set sigLst [lappend sigLst ${base4}_Dq]
    set completeLst [lappend completeLst ${sigLst}]
    set sigLst {}
    set sigLst [lappend sigLst ${base1}_Dh]
    set sigLst [lappend sigLst ${base2}_Dh]
    set sigLst [lappend sigLst ${base3}_Dh]
    set sigLst [lappend sigLst ${base4}_Dh]
    set completeLst [lappend completeLst ${sigLst}]
    #set completeLst [list [list ${base1}_tq ${base2}_tq] ${base1}_hq ${base1}_Dq ${base1}_Dh]
    
    set code [catch {mdisp 2 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }

    ${result} switch_allgraph_flag
    ${result} gr set_disp_mode all
    if {$state1(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    
    ${result} setLabel "$state1(comments)\n($state1(a_min), $state1(n_octave), \
	    $state1(n_voice)) ; $state1(n_study) $source\n$state1(study_method)" 3

    #set english_red "0.8300 0.2400 0.1000"

    $result setColorsByList {red blue magenta slateblue}
	
    ${result}gr0000 set_label {black "tau(q)"}
    ${result}gr0100 set_label {black "D(q)"}
    ${result}gr0001 set_label {black "h(q)"}
    ${result}gr0101 set_label {black "D(h)"}
    
    ${result} gr init_disp
    eval ${result} print dh.eps
    
    return $result
}


# pf::taudDisp --
# usage : pf::taudDisp pfId [list]
#
#  Display tau(q) and D(h).
#
# Parameters :
#   [list] - one or more of the following : tq and Dh. Default is all.
#
# Options :
#   -tsallis real : Display logZtsa(q), htsa(q), Dtsa(q) and Dtsa(htsa)
#                   for some value of tsallis parameter.
# Return value :
#   None.

proc pf::taudDisp {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set isTsallis no
    # Arguments analysis
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -tsallis {
		set qtsa [lindex $args 1]
		set args [lreplace $args 0 1]
		set isTsallis yes
		set qtsa_str [get_q_str $qtsa]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    set base $state(baseSigName)

    if {$isTsallis == "yes"} {
	set completeLst [list ${base}_tq_${qtsa_str} ${base}_Dh_${qtsa_str}]
    } else {
	set completeLst [list ${base}_tq ${base}_Dh]
    }

    set code [catch {mdisp 1 2 ${completeLst}} result]
    if {$code != 0} {
	error $result $result
    }

    ${result} gr set_disp_mode one
    if {$state(dimension) == "2D"} {
	set source "image(s)"
    } else {
	set source "signal(s)"
    }
    if {$isTsallis == "yes"} {
	${result} setLabel "Tsallis : qtsallis = $qtsa; $state(comments)\n($state(a_min), $state(n_octave), \
		$state(n_voice)) ; $state(n_study) $source\n$state(study_method)" 3
	
	${result}gr0000 set_label2 {black "logZtsa(q)"}
	${result}gr0001 set_label2 {black "Dtsa(h)"}
    } else {
	${result} setLabel ""
	
	${result}gr0000 set_label {black "tau(q)"}
	${result}gr0001 set_label {black "D(h)"}
	
    }
    ${result} gr init_disp
    eval ${result} print dh.eps

    return $result
}


# pf::GetBaseName - PRIVATE
# usage : pf::GetBaseName pfId
#
#   Get the signals base name of a pf id.
#
# Parameters :
#   pfId - the pf id.
#
# Return value :
#   The base name.

proc pf::GetBaseName {pfid} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    return $state(baseSigName)
}


# pf::getParam --
# usage : pf::getParam pfId str
#
#   Get the value of a parameter of a pf.
#
# Parameters :
#   pfId   - the pf id.
#   string - the parameter name.
#
# Return value :
#   The value.

proc pf::getParam {pfid pName} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    if {[info exists state($pName)] != 1} {
	return -code error "wrong parameter name"
    }

    return $state($pName)
}


# pf::setcomments --
# usage : pf::setcomments pfId str
#
#   Get the value of a parameter of a pf.
#
# Parameters :
#   pfId   - the pf id.
#   string - comments (list)
#
# Return value :
#   The value.

proc pf::setcomments {pfid comments} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set state(comments)	$comments
    return
}


# pf::tauNorm --
# usage : pf::tauNorm pfId
#
#   Add a constant to tau(q) to obtain tau(0) = -2.
#
# Parameters :
#   pfId   - the pf id.
#
# Return value :
#   The value of the constant.

proc pf::tauNorm {pfid tauName} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
	return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set tauSig [pf GetBaseName $pfid]_$tauName

    sigloop $tauSig {
	set tau_0 [expr { $y } ]
	if {$x == 0} {
	    break
	}
	if {$x > 0} {
	    return -code error "no value for tau(0)"
	}
    }

    set value [expr { -2 - $tau_0 }]

    s2fs $tauSig $tauSig x y+$value

    return $value
}


# pf::outascii --
# usage : pf::outascii pfId
#
#   Save on disk using ascii file format the partition function 
#   regression fits output signals tau(q), h(q), D(q) 
#   and the multifractal spectrum signal D(h)
#
# Parameters :
#   pfId   - the pf id.
#
# Return value :
#   None
#
# AK 2004_03_20
#
proc pf::outascii {pfid args} {
    set pfid [Check_Pfid $pfid]
    if {$pfid == 0} {
        return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set base $state(baseSigName)
    ssave ${base}_tq -ascii -noheader
    ssave ${base}_hq -ascii -noheader 
    ssave ${base}_Dq -ascii -noheader
    ssave ${base}_Dh -ascii -noheader
}

# pf::outascii_Z_h --
# usage : pf::outascii_Z_h pfId
#
#   Save on disk using ascii file format the partition functions 
#   in Z and h for all values of q available.
#
# Parameters :
#   pfId   - the pf id.
#
# Return value :
#   None
#
# AK 2004_03_27
#
proc pf::outascii_Z_h {pfid args} {
    set pfid [CheckPfid $pfid]
    if {$pfid == 0} {
        return -code error "wrong pf id"
    }
    variable $pfid
    upvar 0 $pfid state

    set base $state(baseSigName)

    foreach q $state(q_lst) {
        set q_str [get_q_str $q]
        ssave ${base}_tau${q_str} Z_$q -ascii -noheader
        ssave ${base}_h${q_str} h_$q -ascii -noheader
    }
}
