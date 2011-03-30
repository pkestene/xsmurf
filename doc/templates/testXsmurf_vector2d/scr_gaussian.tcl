
puts [pid]

set theScr {

    # initialize all parameters for the WTMM study
    init -filename parameters_gaussian.tcl
    
    set logCmd dputs
    
    # set a list of image identifiers (4 vector fields will be analyzed)
    set image_list_x {brox1 brox2 brox3 brox4}
    set image_list_y {broy1 broy2 broy3 broy4}

    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}
    dputs " wtmm $wavelet"

    # loop over vector fields
    foreach imaIdf1 $image_list_x imaIdf2 $image_list_y {

	logMsg "Vector field: $imaIdf1 $imaIdf2 "
	# create data to be analyzed
	# 2D Fractional Brownian vector field with default holder value
	# defined in parameters_gaussian.tcl
	ibro2Dfield ${imaIdf1} ${imaIdf2} $size -h $H
	isave ${imaIdf1} ${baseDir}/${imaIdf1}
	isave ${imaIdf2} ${baseDir}/${imaIdf2}

	file mkdir ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}

	# main WTMM command
	# it is essentially a loop over scales
	# for each scale, it computes the WTMM edges and then execute
	# the code between the curly brackets
	wtmmg2d_vector ${imaIdf1} ${imaIdf2} {
	    
	    esave max$scaleIdF ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/max$scaleIdF
		    
	    delete mod$scaleIdF
	    delete arg$scaleIdF
	}
	dputs "End wtmm."
	
	# compute bordersize
	set b1 [GetBorderSize]
	set b2 [expr { $size - $b1 }]
	dputs "b1 $b1"
	dputs "b2 $b2"
	
	# perform the chaining procedure over scales (computing WTMMM)
	dputs " chain..."
	chain m $amin $noct $nvox \
		-filename ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/max \
		-boxratio 1 \
		-ecut [list $b1 $b1 $b2 $b2] \
		-nomsg
	

	# pre-computations (intermediate result) of the partition functions
	file mkdir ${baseDir}/pf
	if {[file exists ${baseDir}/pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wavelet}] == 0} {
	    dputs " Computing the pf..."
	    
	    set zepf [pf create]
	    pf  init $zepf $amin $noct $nvox $q_lst $size "Gradient max" {}
	    pf compute $zepf m
	    pf save $zepf ${baseDir}/pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wavelet}
	    pf destroy $zepf
	    dputs " ok."
	} else {
	    dputs " Pf already computed."
	}
    
    dputs "pf computed!"
    }
    logMsg "End."
}

ist $theScr

exit
