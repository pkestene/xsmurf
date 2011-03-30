
puts [pid]

set theScr {

    # initialize all parameters for the WTMM study
    init -filename parameters_gaussian.tcl
    
    file mkdir ${baseDir}/pf
    
    set logCmd dputs
    
    # set a list of image identifiers (1 vector field will be analyzed)
    #set image_list_x {ima1x}
    #set image_list_y {ima1y}
    #set image_list_z {ima1z}
    #set image_list_x {fbm3d_64_x}
    #set image_list_y {fbm3d_64_y}
    #set image_list_z {fbm3d_64_z}
    set image_list_x {fbm3d_128_x}
    set image_list_y {fbm3d_128_y}
    set image_list_z {fbm3d_128_z}

    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}
    dputs " wtmm $wavelet"

    # compute bordersize
    #set cut1 [GetBorderSize]
    set cut1 16
    set cut2 [expr { $size - $cut1 }]
    dputs "cut1 $cut1"
    dputs "cut2 $cut2"

    # loop over the 3d vector fields
    foreach imaIdfx $image_list_x imaIdfy $image_list_y imaIdfz $image_list_z {

	logMsg "Vector field: $imaIdfx $imaIdfy $imaIdfz "
	# create data to be analyzed
	# 3D test vector field with default
	# defined in parameters_gaussian.tcl
	#itest3Dvector ${imaIdfx} ${imaIdfy} ${imaIdfz} $size $H -sigma 8. -coef -1.0 1.0
	ibro3D ${imaIdfx} $size -h $H
	ibro3D ${imaIdfy} $size -h $H
	ibro3D ${imaIdfz} $size -h $H
	i3Dsave ${imaIdfx} ${baseDir}/${imaIdfx}
	i3Dsave ${imaIdfy} ${baseDir}/${imaIdfy}
	i3Dsave ${imaIdfz} ${baseDir}/${imaIdfz}

	file mkdir ${baseDir}/${imaIdfx}_${imaIdfy}_${imaIdfz}_${type}_max_${wavelet}

	# main WTMM command
	# it is essentially a loop over scales
	# for each scale, it computes the WTMM edges and then execute
	# the code between the curly brackets
	wtmmg3d_vector ${imaIdfx} ${imaIdfy} ${imaIdfz} {
	    
	    esave3Dsmall max$scaleIdF ${baseDir}/${imaIdfx}_${imaIdfy}_${imaIdfz}_${type}_max_${wavelet}/max$scaleIdF
	    esave3Dsmall mmax$scaleIdF ${baseDir}/${imaIdfx}_${imaIdfy}_${imaIdfz}_${type}_max_${wavelet}/mmax$scaleIdF
		    
	    delete mod$scaleIdF
	}
	dputs "End wtmm."
		
	#init pf computation
	pf {
            if ![info exists pf(pfid)] {
                set pf(pfid) 0
                create
            } elseif { $pf(pfid) > 0} {
                set pf(pfid) 1
                clear ::pf::1
            }
	}
	#set zepf [pf create]
	pf init ::pf::1 $amin $noct $nvox $q_lst $size "Gradient max" {}
	
	#init qvalue_list for pf computations
	foreach q $q_lst {
	    set val_STq_liste($q) {}
	    set val_STqlogT_liste($q) {}
	}
	

	# init chaining at the lowest scale
	ecut3Dsmall mmax000 e000 $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
	ecut3Dsmall mmax001 e001 $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
	vchain3Dsmall e000 e001 -first -simil $similitude
	foreach q $q_lst {
	    #logMsg "  $q"
	    set val_on_mod [efct3Dsmall e000 (abs(x))^y $q -vc]
	    lappend val_STq_liste($q) $val_on_mod
	    set val_on_mod [efct3Dsmall e000 log(abs(x))*(abs(x))^y $q -vc]
	    lappend val_STqlogT_liste($q) $val_on_mod
	    
	    set val_on_mod [efct3Dsmall e001 (abs(x))^y $q -vc]
	    lappend val_STq_liste($q) $val_on_mod
	    set val_on_mod [efct3Dsmall e001 log(abs(x))*(abs(x))^y $q -vc]
	    lappend val_STqlogT_liste($q) $val_on_mod
	}    
	
	delete mmax000 e000

	#iterate chaining through higher scales
	for {set scale 2} {$scale<$noct*$nvox} {incr scale} {
	    set scaleF   [format "%.3d" $scale]
	    set scaleFm1 [format "%.3d" [expr $scale-1]]

	    logMsg "   scale $scaleF"
	    ecut3Dsmall mmax$scaleF e$scaleF $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
	    vchain3Dsmall e$scaleFm1 e$scaleF  -simil $similitude

	    delete e$scaleFm1 mmax$scaleF

	    foreach q $q_lst {
		#logMsg "  $q"
		set val_on_mod [efct3Dsmall e$scaleF (abs(x))^y $q -vc]
		lappend val_STq_liste($q) $val_on_mod
		set val_on_mod [efct3Dsmall e$scaleF log(abs(x))*(abs(x))^y $q -vc]
		lappend val_STqlogT_liste($q) $val_on_mod
	    }
	}
	delete e$scaleF mmax$scaleF

	# partition function computation
	dputs "Now compute pf..."
	set base _pf1
	foreach q $q_lst {
	    set q_str [get_q_str $q]
	    #dputs "$q"
	    
	    #make the main signals
	    set fctName STq
	    set sigName ${base}_${fctName}_$q_str
	    screate $sigName 0 [expr 1.0/$nvox] $val_STq_liste($q)
	    
	    
	    set logFctName logSTq
	    set sigName ${base}_${logFctName}_$q_str
	    set  val_STq_liste2($q) ""
	    foreach val $val_STq_liste($q) {
		lappend val_STq_liste2($q) [expr ($val?log($val):0)]
	    }
	    screate $sigName 0 [expr 1.0/$nvox] $val_STq_liste2($q)
	    
	    set fctName STqlogT
	    set sigName ${base}_${fctName}_$q_str
	    screate $sigName 0 [expr 1.0/$nvox] $val_STqlogT_liste($q)
    
	}
	
	# save the results of OUR computations !!!
	dputs "pf save..."
	pf save ::pf::1 ${baseDir}/pf/pf_${type}_${imaIdfx}_${imaIdfy}_${imaIdfz}_${wavelet} -nstudy
	#pf destroy ::pf::1
    
	dputs "pf computed!"
    }
    logMsg "End."
}

ist $theScr

exit
