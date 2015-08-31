# imTools.tcl --
#
#       This file implements the Tcl code image tools.
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: imTools.tcl,v 1.1 1999/07/09 15:14:41 decoster Exp $
#

package provide imTools


# imoment --
# usage: imoment Image int
#
#   Compute a given moment of an image.
#
# Parameters:
#   Image   - Image to treat.
#   integer - Order of the moment.
#
# Return value:
#   The moment.

proc imoment {image order} {
    mylassign {type lx ly min max} [iinfo $image]
    set size [expr { $lx*$ly }]

    return [expr { [ifct $image pow(x,$order)]/$size }]
}


# istats --
# usage: istats Image int
#
#   Compute successive moments of an image.
#
# Parameters:
#   Image   - Image to treat.
#   integer - Order of the upper moment. Must be greater or equal to 2.
#
# Return value:
#   List of the variance and of the moments.

proc istats {image order} {
    if {$order < 2} {
	return -code error ""
    }

    for {set k 1} {$k <= $order} {incr k} {
	set res [imoment $image $k]
        lappend resLst $res
    }

    set variance [expr { sqrt([lindex $resLst 1]-pow([lindex $resLst 0],2)) }]

    return [concat $variance $resLst]
}


# correlation --
# usage: correlation image image 
#
#   Compute the correlations between 2 images. By default the mean of
# each image is substracted before treatment. Both 2d and 1d
# correlations can be computed. 1d correlations are an angular mean of
# the 1d correlations.
#
# Parameters:
#   2 Images - Images to treat.
#
# Options:
#   -2d: Option to request the 2d correlations.
#      string - Name of the 2d correlations image.
#   -1d: Specify the name of the 1d correlations signal.
#      string - The Name. Default is "c_${im1}_${im2}", where im1 and
#               im2 are the names of the images to treat.
#   -w1: Specify the name of a precomputed image that contains the
#      first image minus its mean.
#   -w2: Specify the name of a precomputed image that contains the
#      second image minus its mean.
#   -nomean: Do not substract the mean of each images before
#      treatment.
#
# Return value:
#   Name of the 1d correlations signal.

proc correlation {im1 im2 args} {
    set sigName c_${im1}_${im2}
    set imName __imRes
    set w1Name __w1
    set w2Name __w2
    set isNomean no

    # Arguments analysis

    set oldArgs $args
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -2d {
		set imName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -1d {
		set sigName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -w1 {
		set w1Name [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -w2 {
		set w2Name [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -nomean {
		set isNomean yes
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

    if {$isNomean == "no"} {
	if {[string compare $w1Name "__w1"] == 0 \
		|| [catch [list ginfo $w1Name]] == 1} {
	    set mean [imoment $im1 1]
	    icomb $im1 $im1 x-$mean $w1Name
	}

	if {[string compare $w2Name "__w2"] == 0 \
		|| [catch [list ginfo $w2Name]] == 1} {
	    set mean [imoment $im2 1]
	    icomb $im2 $im2 x-$mean $w2Name
	}
    } else {
	set w1Name $im1
	set w2Name $im2
    }

    mylassign {type lx ly min max} [iinfo $im1]
    set size [expr { [im_size $im1]/2 }]
    
    set nbOfPoints [expr { $lx*$ly }]

    iinvert $w1Name __tmp
    cv2dn __tmp $w2Name $imName -ft -0p
    icomb $imName $imName x/$nbOfPoints $imName
    iangularmean $imName $sigName $size

    catch {delete __tmp}
    catch {delete __imRes}
    catch {delete __w1}
    catch {delete __w2}

    return $sigName
}

# iexists --
# usage: iexists string
#
#   Check the existence of an image.
#
# Parameters:
#   string - Name of the image to check.
#
# Return value:
#   1 if the image exists, 0 otherwise.

proc iexists {im} {
    set code [catch [list iinfo $im]]

    if {$code == 0} {
	return 1
    } else {
	return 0
    }
}

