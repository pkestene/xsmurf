# wt.tcl --
#
#       This file implements the Tcl code for wavelet transform computing using
# gfft FFT algorithm (faster than NR one).
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: wt.tcl,v 1.4 1998/05/11 13:07:50 decoster Exp $
#

# iswt --
# usage : iswt image float str str
#
#  Compute the wavelet transform of an image at a given scale.
#
# Parameters :
#   image  - image to treat.
#   float  - scale of the wavelet transform.
#   string - expression of the real part of the wavelet fourier transform.
#   string - expression of the imaginary part of the wavelet fourier transform.
#
# Return value :
#   Name of the wavelet transform image.

proc iswt {image scale fct_r fct_i} {
    igfft $image wt$image
    iconvol wt$image gah $scale -new $fct_r $fct_i
    igfft wt$image wt$image -reverse
    return wt$image
}

# iwt --
# usage : iwt image float int int [int]
# 
#  Compute the wavelet transform of an image. The scales follow a log
# progression.
# 
# Parameters :
#   image   - image to treat.
#   float   - first scale of the WT.
#   integer - number of octaves.
#   integer - number of voices for each octave.
#   integer - (optional) If this flag is set to 0 (default), each scale is
#             saved on disk and delete from memory.
#
# Return Value :
#   none.

proc iwt {image a_min n_oct n_vox fct_r fct_i {flag 0}} {
    igfft   $image __ft
    isave   $image __$image
    delete $image
    isave   __ft
    delete  __ft
     
    set num 0
    
    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    set scale [expr $a_min*pow(2,$oct+($vox/double($n_vox)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    iload   __ft wt$image$new_num
	    iconvol wt$image$new_num gah $scale -new $fct_r $fct_i
	    igfft   wt$image$new_num wt$image$new_num -reverse
	    if {$flag == 0} {
		isave   wt$image$new_num
		delete  wt$image$new_num
	    }
	}
    }
    iload __$image $image
    exec rm __$image
    exec rm __ft
    return
}

proc __conv_dx {scale} {
    iload   __ft dx
    iconvol dx gah $scale -new 0 x*exp(-x*x-y*y)
    igfft   dx dx -reverse
}

proc __conv_dy {scale} {
    iload   __ft dy
    iconvol dy gah $scale -new 0 y*exp(-x*x-y*y)
    igfft   dy dy -reverse
}

proc __conv_dxx {scale} {
    iload   __ft dxx
    iconvol dxx gah $scale -new -x*x*exp(-x*x-y*y) 0
    igfft   dxx dxx -reverse
}

proc __conv_dxy {scale} {
    iload   __ft dxy
    iconvol dxy gah $scale -new -y*x*exp(-x*x-y*y) 0
    igfft   dxy dxy -reverse
}

proc __conv_dyy {scale} {
    iload   __ft dyy
    iconvol dyy gah $scale -new -y*y*exp(-x*x-y*y) 0
    igfft   dyy dyy -reverse
}

proc __conv_dxxx {scale} {
    iload   __ft dxxx
    iconvol dxxx gah $scale -new 0 -x*x*x*exp(-x*x-y*y)
    igfft   dxxx dxxx -reverse
}

proc __conv_dxxy {scale} {
    iload   __ft dxxy
    iconvol dxxy gah $scale -new 0 -x*x*y*exp(-x*x-y*y)
    igfft   dxxy dxxy -reverse
}

proc __conv_dxyy {scale} {
    iload   __ft dxyy
    iconvol dxyy gah $scale -new 0 -x*y*y*exp(-x*x-y*y)
    igfft   dxyy dxyy -reverse
}

proc __conv_dyyy {scale} {
    iload   __ft dyyy
    iconvol dyyy gah $scale -new 0 -y*y*y*exp(-x*x-y*y)
    igfft   dyyy dyyy -reverse
}

proc __conv_all_derivatives {scale} {
    __conv_dx   $scale
    __conv_dy   $scale
    __conv_dxx  $scale
    __conv_dxy  $scale
    __conv_dyy  $scale
    __conv_dxxx $scale
    __conv_dxxy $scale
    __conv_dxyy $scale
    __conv_dyyy $scale
}

# iwtmm --
# usage : iwtmm image float int int [-clean] [-isave] [-esave] [-thresh]
# 
#  Compute the wavelet transform of an image and compute its lines of maxima.
# The scales follow a log progression.
# 
# Parameters :
#   image   - image to treat.
#   float   - first scale of the WT.
#   integer - number of octaves.
#   integer - number of voices for each octave.
#
# Options :
#   -clean  : Delete everything after computing. 
#   -isave  : For each scale the modulus and the argument images are saved.
#   -esave  : For each scale the image of the line of maxima is saved.
#   -thresh : A thresh is done on each image of line of maxima. The value of
# the thresh is 10e-6.
#
# Return Value :
#   none.

proc iwtmm {image a_min n_oct n_vox args} {
    # get flags
    set is_clean 0
    set is_isave 0
    set is_esave 0
    set is_thresh 0
    foreach flag $args {
	if {![string compare $flag -clean]} {
	    set is_clean 1
	}
	if {![string compare $flag -isave]} {
	    set is_isave 1
	}
	if {![string compare $flag -esave]} {
	    set is_esave 1
	}
	if {![string compare $flag -thresh]} {
	    set is_thresh 1
	}
    }
    
    igfft   $image __ft
    isave   $image __$image
    delete  $image
    isave   __ft
    delete  __ft
     
    set num 0
    
    for { set oct 0;set num 0}\
	    { $oct < $n_oct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $n_vox } \
		{ incr vox ; incr num} {
	    set scale [expr $a_min*pow(2,$oct+($vox/double($n_vox)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    __conv_all_derivatives $scale
	    garg dx dy ${image}arg$new_num
	    gmod dx dy ${image}mod$new_num
	    gkapap ${image}kapap${new_num} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
	    delete dxxx dxxy dxyy dyyy
	    gkapa ${image}kapa${new_num} dx dxx dy dyy dxy
	    delete dx dy dxx dxy dyy
	    follow ${image}kapa${new_num} ${image}kapap${new_num} \
                    ${image}mod${new_num} ${image}arg${new_num} \
		    ${image}max$new_num $scale
	    if {$is_thresh} {
		ekeep ${image}max$new_num ${image}max$new_num 0.000001
	    }
	    if {$is_isave} {
		isave ${image}arg${new_num}
		isave ${image}mod${new_num}
		isave ${image}kapa${new_num}
		isave ${image}kapap${new_num}
	    }
	    if {$is_esave} {
		esave ${image}max$new_num ${image}max$new_num
	    }
	    if {$is_clean} {
		delete ${image}max$new_num
		delete ${image}arg${new_num}
		delete ${image}mod${new_num}
		delete ${image}kapa${new_num}
		delete ${image}kapap${new_num}
	    }
	}
    }
    iload __$image $image
    exec rm __$image
    exec rm __ft

    return
}

# iwtmml --
# usage : iwtmm image float int float [-clean] [-isave] [-esave] [-thresh]
# 
#  Compute the wavelet transform of an image and compute its lines of maxima.
# The scales follow a linear progression.
# 
# Parameters :
#   image   - image to treat.
#   float   - first scale of the WT.
#   integer - number of scales.
#   float   - step between each scale.
#
# Options :
#   -clean  : Delete everything after computing. 
#   -isave  : For each scale the modulus and the argument images are saved.
#   -esave  : For each scale the image of the line of maxima is saved.
#   -thresh : A thresh is done on each image of line of maxima. The value of
# the thresh is 10e-6.
#
# Return Value :
#   none.

proc iwtmml {image a_min nb step args} {
    # get flags
    set is_clean 0
    set is_isave 0
    set is_esave 0
    set is_thresh 0
    foreach flag $args {
	if {![string compare $flag -clean]} {
	    set is_clean 1
	}
	if {![string compare $flag -isave]} {
	    set is_isave 1
	}
	if {![string compare $flag -esave]} {
	    set is_esave 1
	}
	if {![string compare $flag -thresh]} {
	    set is_thresh 1
	}
    }
    
    igfft   $image __ft
    isave   $image __$image
    delete  $image
    isave   __ft
    delete  __ft
    
    for {set num 0}\
	    { $num < $nb} \
	    { incr num } {
	set scale [expr $a_min+$step*$num]
	dputs "  scale $scale ( $num )"
	set new_num [format "%.3d" $num]
	
	__conv_all_derivatives $scale
	garg dx dy ${image}arg$new_num
	gmod dx dy ${image}mod$new_num
	gkapap ${image}kapap${new_num} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
	delete dxxx dxxy dxyy dyyy
	gkapa ${image}kapa${new_num} dx dxx dy dyy dxy
	delete dx dy dxx dxy dyy
	extrema ${image}max$new_num \
		${image}mod${new_num} \
		${image}arg${new_num} \
		$scale \
		-contour \
		${image}kapa${new_num} \
		${image}kapap${new_num} 1
	if {$is_thresh} {
	    ekeep ${image}max$new_num ${image}max$new_num 0.000001
	}
	if {$is_isave} {
	    isave ${image}arg${new_num}
	    isave ${image}mod${new_num}
	    isave ${image}kapa${new_num}
	    isave ${image}kapap${new_num}
	}
	if {$is_esave} {
	    esave ${image}max$new_num ${image}max$new_num
	}
	if {$is_clean} {
	    delete ${image}max$new_num
	    delete ${image}arg${new_num}
	    delete ${image}mod${new_num}
	    delete ${image}kapa${new_num}
	    delete ${image}kapap${new_num}
	}
    }

    iload __$image $image
    exec rm __$image
    exec rm __ft
}
