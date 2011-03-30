
puts [pid]

set theScr {

    # initialize all parameters for the WTMM study
    init -filename parameters_mexican.tcl
    
    set logCmd dputs
    
    # set a list of image identifiers (4 scalar fields will be analyzed)
    set image_list {bro1 bro2 bro3 bro4}

    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}_gpu${useGPU}
    dputs " wtmm $wavelet"

    # loop over scalar fields
    foreach imaIdf $image_list {

	logMsg "Scalar field: $imaIdf "
	# create data to be analyzed
	# 2D Fractional Brownian scalar field with default holder value
	# defined in parameters_mexican.tcl
	ibro ${imaIdf} $size -h $H
	isave ${imaIdf} ${baseDir}/${imaIdf}

	file mkdir ${baseDir}/${imaIdf}_${type}_max_${wavelet}

	# main WTMM command
	# it is essentially a loop over scales
	# for each scale, it computes the WTMM edges and then execute
	# the code between the curly brackets
	wtmmg ${imaIdf} {
	    
	    esave max$scaleIdF ${baseDir}/${imaIdf}_${type}_max_${wavelet}/max$scaleIdF
		    
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
		-filename ${baseDir}/${imaIdf}_${type}_max_${wavelet}/max \
		-boxratio 1 \
		-ecut [list $b1 $b1 $b2 $b2] \
		-nomsg
	

	# pre-computations (intermediate result) of the partition functions
	file mkdir ${baseDir}/pf
	if {[file exists ${baseDir}/pf/pf_${type}_${imaIdf}_${wavelet}] == 0} {
	    dputs " Computing the pf..."
	    
	    set zepf [pf create]
	    pf  init $zepf $amin $noct $nvox $q_lst $size "Gradient max" {}
	    pf compute $zepf m
	    pf save $zepf ${baseDir}/pf/pf_${type}_${imaIdf}_${wavelet}
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
