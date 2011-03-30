#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#

# Wavelet transform at one scale.
#
proc sto {image scale wave} {
    itofft  $image IMAGE
    isave   $image im.0
    delete $image
    fft     IMAGE
    isave   IMAGE im.1

    nputs "scale $scale"
    conv_w $wave $scale

    delete IMAGE
    iload im.0 $image
    exec rm im.0
    exec rm im.1
}

# Wavelet transform at several scales.
#
proc to {image amin noct nvoix wave} {
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
	    puts "scale $scale ( $num )"
	    set new_num [format "%.3d" $num]
	    
	    conv_w $wave $scale
	    isave $wave i_$new_num
	    #icopy $wave i_$new_num
	}
    }
    delete IMAGE
    iload im.0 $image
    exec rm im.0
    exec rm im.1
}

# Wavelet transform at several scales.
#
proc to_lin {image amin nb step wave} {
    itofft  $image 	IMAGE
    isave   $image im.0
    delete $image
    fft     IMAGE
    isave   IMAGE       im.1

    for {set num 0}\
	    { $num < $nb} \
	    { incr num } {
	set scale [expr $amin+$step*$num]
	puts "scale $scale ( $num )"
	set new_num [format "%.3d" $num]
	
	conv_w $wave $scale
	isave $wave ${image}_$new_num
    }

    delete $wave
    delete IMAGE
    iload im.0 $image
    exec rm im.0
    exec rm im.1
}