#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#

#
#
proc conv_w {wave scale} {
    iload   im.1    IMAGE
    iconvol IMAGE $wave $scale
    fft     IMAGE -reverse
    ffttoi  IMAGE $wave
}

#
#
proc conv_all {scale} {
    conv_w dx $scale
    conv_w dy $scale
    garg dx dy arg
    gmod dx dy mod
    conv_w dxx $scale
    conv_w dyy $scale
    conv_w dxy $scale
    conv_w dxxx $scale
    conv_w dyyy $scale
    conv_w dxxy $scale
    conv_w dxyy $scale
}

# Wavelet transform of an image. Only maxima lines are kept and saved to disk.
# Multi scale.
#
proc mmto {image amin noct nvoix} {
    itofft  $image 	IMAGE
    isave   $image im.0
    delete $image
    fft     IMAGE
    isave   IMAGE       im.1
     
    set num 0
    
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set voix 0} \
		{ $voix < $nvoix } \
		{ incr voix ; incr num} {
	    set scale [expr $amin*pow(2,$oct+($voix/double($nvoix)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    conv_all $scale
	    gkapap kapap${image}${new_num} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
	    delete dxxx dxxy dxyy dyyy
	    gkapa kapa${image}${new_num} dx dxx dy dyy dxy
	    delete dx dy dxx dxy dyy
	    follow kapa${image}${new_num} kapap${image}${new_num} \
		    mod arg ${image}max$new_num $scale
	    esave ${image}max$new_num
	    delete arg mod kapa${image}${new_num} kapap${image}${new_num}
	}
    }
    delete IMAGE
    iload im.0 $image
    exec rm im.0
    exec rm im.1
}

# Wavelet transform of an image. Only maxima lines are kept and saved to disk.
# Single scale.
#
proc smmto {image scale} {
    itofft  $image IMAGE
    isave   $image im.0
    delete $image
    fft     IMAGE
    isave   IMAGE im.1

    nputs "scale $scale"
    conv_all $scale
    gkapap kapap dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
    delete dxxx dxxy dxyy dyyy
    gkapa kapa dx dxx dy dyy dxy
    delete dx dy dxx dxy dyy
    follow kapa kapap mod arg ${image}max $scale
    esave ${image}max
    delete arg mod kapa kapap
	    
    delete IMAGE
    iload im.0 $image
    exec rm im.0
    exec rm im.1
}

# Only display the value of the scales for a set of value of amin, noct and
# nvoix.
#
proc silent_mmto {amin noct nvoix} {
    set num 0
    
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set voix 0} \
		{ $voix < $nvoix } \
		{ incr voix ; incr num} {
	    set scale [expr $amin*pow(2,$oct+($voix/double($nvoix)))]
	    set scale [expr $scale*(6/0.86)]
	    puts "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	}
    }
}

# From a WTMM, remove all maxima which modulus is under a given value.
#
proc gekeep {max amin noct nvoix {value 0.00001}} {
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set voix 0} \
		{ $voix < $nvoix } \
		{ incr voix ; incr num} {
	    set scale [expr $amin*pow(2,$oct+($voix/double($nvoix)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    ekeep ${max}$new_num ${max}$new_num $value
	}
    }
}

# line local maxima. A REFAIRE !!!!!
#
proc llm {image amin noct nvoix {box_mult 1}} {
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set voix 0} \
		{ $voix < $nvoix } \
		{ incr voix ; incr num} {
	    set scale [expr $amin*pow(2,$oct+($voix/double($nvoix)))]
	    set scale [expr $scale*(6/0.86)]
	    dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    set num0 [expr $num-1]
	    set new_num0 [format "%.3d" $num0]
	    
	    eload ${image}$new_num l$new_num
	    rm_ext l$new_num l$new_num 150 873 150 873
	    hsearch l$new_num
	    #rm_by_size l$new_num -min [expr int($scale/3)]
	    eupdate l$new_num l$new_num
	    ssm l$new_num
	    if {$num == 1} {
		set box_size [expr int(log($scale)/log(2))]
		vchain l$new_num0 l$new_num $box_size 0.9 -first
	    } else {
		if {$num != 0} {
		    set box_size [expr int(log($scale)*$box_mult/log(2))]
		    puts $box_size
		    vchain l$new_num0 l$new_num $box_size 0.9
		    #l2m l$new_num0 m$new_num0
		    #delete l$new_num0
		    #esave m$new_num0
		    #delete m$new_num0
		}
	    }
	}
    }
    #l2m l000 m000
    #delete l000
    #esave m000
    #delete m000
    #l2m l$new_num m$new_num
    #delete l$new_num
    #esave m$new_num
    #delete m$new_num
    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set voix 0} \
		{ $voix < $nvoix } \
		{ incr voix ; incr num} {
	    set scale [expr $amin*pow(2,$oct+($voix/double($nvoix)))]
	    set scale [expr $scale*(6/0.86)]
	    #dputs "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    set num0 [expr $num-1]
	    set new_num0 [format "%.3d" $num0]
	    
	    #eload l$new_num
	    #el2shisto l$new_num h$new_num 32 -x 0 100
	    #l2m l$new_num m$new_num
	    #delete l$new_num
	    #esave m$new_num
	}
    }
}
