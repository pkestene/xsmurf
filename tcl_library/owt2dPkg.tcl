# owt2dPkg.tcl --
#
#       This file implements the Tcl code for orthogonal decomposition.
#
#   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: owt2dPkg.tcl,v 1.4 1999/06/25 20:25:44 decoster Exp $
#

package provide owt2d 0.0

package require lwPkg 0.0

# o2load --
# usage: o2load string int
#
#   Load all the images corresponding to a orthogonal decomposition.
#
# Arguments:
#   string  - Base name of the files.
#   integer - Number of octaves used for the decomposition.
#
# Return value:
#   None.

proc o2load {name nOct} {
    for {set i 1} {$i <= $nOct} {incr i} {
	iload ${name}_x_$i
	iload ${name}_y_$i
	iload ${name}_xy_$i
    }
    return
}


# o22im --
# usage: o22im string int [-load]
#
#   Create ane image that contains all the images corresponding to a orthogonal
# decomposition.
#
# Arguments:
#   string  - Base name of the files. This will be used for the name of the new
#             image.
#   integer - Number of octaves used for the decomposition.
#
# Options:
#   -load : Load all the images before inserting.
#
# Return value:
#   Name of the image.

proc o22im {name nOct args} {
    set isLoad 0
    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    -load {
		set isLoad 1
		set args [lreplace $args 0 0]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    if {$isLoad == 1} {
	o2load $name $nOct
    }

    set resName ${name}_o2
    set size [expr { 2*[lindex [iinfo ${name}_x_1] 1] }]
    inull $resName $size
    for {set i 1} {$i <= $nOct} {incr i} {
	set size [expr { [lindex [iinfo ${name}_x_$i] 1] }]
	iinsert $resName ${name}_x_$i $size 0
	iinsert $resName ${name}_y_$i 0 $size
	iinsert $resName ${name}_xy_$i $size $size
    }

    return $resName
}


# o2dec --
# usage: o2dec string integer [-resname string]
#
#   Ask last wave ( (c) E. Bacry ) to compute an orthogonal decomposition on an
# image. The image must exist on disk.
#
# Arguments:
#   string  - Image file name..
#   integer - Number of octaves used for the decomposition.
#
# Options:
#   -resname :
#      string - Base name of the resulting files.
#
# Return value:
#   List of the orthogonal decomposition files.

proc o2dec {name nOct args} {
    set wavelet Daub8
    set resName $name

    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    -resname {
		set resName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    if {[file isfile $name] == 0} {
	return -code error "can't read \"$name\": no such file"
    }
    if {[file isdirectory [file dirname $resName]] == 0} {
	return -code error "can't write in \"[file dirname $resName]\": no such directory"
    }

    scr4lw {
	a2
	iread 0 @$name@ -c
	owt2f @$wavelet@
	owt2d @$nOct@
	for {set i 1} {$i <= @$nOct@} {incr i} {
	    set numx [= ($i)*10+1]
	    set numy [= ($i)*10+2]
	    set numxy [= ($i)*10+3]
	    iwrite $numx @$resName@_x_$i -c
	    iwrite $numy @$resName@_y_$i -c
	    iwrite $numxy @$resName@_xy_$i -c
	}
    }

    set fileLst {}
    for {set i 1} {$i <= $nOct} {incr i} {
	set zeName ${resName}_x_$i
	if {[file exists $zeName] == 0} {
	    return -code error "an error has occured during last wave script execution: file \"$zeName\" doesn't exists"
	}
	lappend fileLst $zeName

	set zeName ${resName}_y_$i
	if {[file exists $zeName] == 0} {
	    return -code error "an error has occured during last wave script execution: file \"$zeName\" doesn't exists"
	}
	lappend fileLst $zeName

	set zeName ${resName}_xy_$i
	if {[file exists $zeName] == 0} {
	    return -code error "an error has occured during last wave script execution: file \"$zeName\" doesn't exists"
	}
	lappend fileLst $zeName
    }

    return $fileLst
}


# o2lnc --
# usage: o2lnc string integer float float float float float
#
#   Ask last wave ( (c) E. Bacry ) to create a log-normal cascade using
# orthogonal wavelet base. For now the best help you can find is in reading the
# source code of the algorithm in LastWave.
#
# Arguments:
#   string  - Result image file name 
#             (saved in current directory in PGM file format) ...
#   integer - Number of octaves used for the cascade.
#   float   - Constant for the scale function (at the highest scale).
#   float   - Constant for the X wavelet function (at the highest scale).
#   float   - Constant for the Y wavelet function (at the highest scale).
#   float   - Constant for the XY wavelet function (at the highest scale).
#   float   - Squared sigma of the log-normal process.
#   float   - Mean of the log-normal process.
#   float   - dTheta1: variation of theta1. theta1 in [-dTheta1/2,dTheta1/2].
#             Theta1 is the angle between the XY wavelet axe and the
#             (X wavelet, Y wavelet) plan. Very special behavior if dTheta1
#             is < 0: we don't want to fill the X and Y components, and the XY
#             components is randomly positive or negative.
#
#  example of parameters :
#    o2lnc res.pgm 10 0 1 1 0.59460355750 0.005 -0.35157359 1.482
#    see file : /user41/decoster/soutenance/figures/04/targets.tcl !!!
#
# Return value:
#   Result file name.

proc o2lnc {resName nOct c0 dx dy dxy sigma2 mean dTheta1 args} {
    set wavelet Daub8
    set isOnlyx 0

    if {[string compare $args ""] != 0} {
	switch -glob -- [lindex $args 0] {
	    -x {
		set isOnlyx 1
		set args [lreplace $args 0 0]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    scr4lw {
	a2
	owt2f Daub8
	set seed [= int(urand*100000)]
	image new kk
	#owcantrn 0 10 0 1 0 0 0.044 -.4 0.0 $seed
	owcantrn 0 kk @$nOct@ @$c0@ @$dx@ @$dy@ @$dxy@ @$sigma2@ @$mean@ 0.0 $seed @$dTheta1@ @$isOnlyx@
	iwrite 0 @$resName@  -h
    }
    
    #example 
    #owcantrn 0 8 0 1 1 0.59460355750 0.005 -0.35157359 0.0 8703 1.482 0
    #owcantrn 0a2 1a2 8 0 1 1 0.59460355750 0.005 -0.35157359 0.0 8703 1.482 0

    return $resName
}


# o2lpc --
# usage: o2lnc string integer float float float float float float
#
#   Ask last wave ( (c) E. Bacry ) to create a log-Poisson cascade using
# orthogonal wavelet base.
#
# Arguments:
#   string  - Result image file name..
#   integer - Number of octaves used for the cascade.
#   float   - Constant for the scale function (at the highest scale).
#   float   - Constant for the X wavelet function (at the highest scale).
#   float   - Constant for the Y wavelet function (at the highest scale).
#   float   - Constant for the XY wavelet function (at the highest scale).
#   float   - Beta.
#   float   - Gamma.
#   float   - Lambda.
#   float - Intervall of delta theta.
#
# Return value:
#   Result name.

proc o2lpc {resName nOct c0 dx dy dxy beta gamma lambda theta args} {
    set wavelet Daub8

    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    scr4lw {
	a2
	owt2f Daub8
	set seed [= int(urand*100000)]
	owcantrp 0 @$nOct@ @$c0@ @$dx@ @$dy@ @$dxy@ @$beta@ @$gamma@ @$lambda@ $seed @$theta@
	iwrite 0 @$resName@ -x
    }

    return $resName
}

