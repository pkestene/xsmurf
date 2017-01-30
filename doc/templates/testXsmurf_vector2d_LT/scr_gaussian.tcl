
puts [pid]

set theScr {

    # initialize all parameters for the WTMM study
    init -filename parameters_gaussian.tcl
    
    set logCmd dputs
    
    # set number of Images of this study
    set numImages 2

    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}
    dputs " wtmm $wavelet"

    # loop over vector fields
    for {set i 0} {$i<$numImages} {incr i} {
	
	set imaIdf1 fBm_vx$i
	set imaIdf2 fBm_vy$i

	logMsg "Vector field: $imaIdf1 $imaIdf2 "
	# create data to be analyzed
	# 2D Fractional Brownian vector field with default holder value
	# defined in parameters_gaussian.tcl
	#ibro2Dfield ${imaIdf1} ${imaIdf2} $size -h $H
	#isave ${imaIdf1} ${baseDir}/${imaIdf1}
	#isave ${imaIdf2} ${baseDir}/${imaIdf2}
	exec python compute_2dfBm_divfree.py -s $size -i $i
	iload ${imaIdf1}.xsm ${imaIdf1}
	iload ${imaIdf2}.xsm ${imaIdf2}
	
	file mkdir ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}

	# main WTMM command
	# it is essentially a loop over scales
	# for each scale, it computes the WTMM edges and then execute
	# the code between the curly brackets
	wtmmg2d_vector ${imaIdf1} ${imaIdf2} {
	    
	    esave max$scaleIdF ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/max$scaleIdF
		    
	    delete mod$scaleIdF
	    delete arg$scaleIdF

	    if {$isLT} {
		esave maxL$scaleIdF ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/maxL$scaleIdF
		esave maxT$scaleIdF ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/maxT$scaleIdF
	    }

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

	# pf method
	set pf_method "Gradient max"
	set pf_method_str "grad_max"
	#set pf_method "Gradient lines"
	#set pf_method_str "grad_lines"
	
	# pre-computations (intermediate result) of the partition functions
	file mkdir ${baseDir}/pf

	# N is for normal  / regular partition function computations
	set pf_suffix {N}
	set pf_filename ${baseDir}/pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wavelet}_${pf_method_str}_${pf_suffix}
	if {[file exists $pf_filename] == 0} {
	    dputs " Computing the pf..."
	    
	    set zepf [pf create]
	    pf  init $zepf $amin $noct $nvox $q_lst $size $pf_method {}
	    pf compute $zepf m
	    pf save $zepf ${pf_filename}
	    pf destroy $zepf
	    dputs " ok."
	} else {
	    dputs " Pf already computed."
	}

	# if isLT is activated then compute L/T partition functions using local maxima
	# of each coefficient
	if ${isLT} {

	    # 1. LONGITUDINAL
	    dputs " chain L coefficients..."
	    chain m $amin $noct $nvox \
		-filename ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/maxL \
		-boxratio 1 \
		-ecut [list $b1 $b1 $b2 $b2] \
		-nomsg

	    set pf_suffix L
	    set pf_filename ${baseDir}/pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wavelet}_${pf_method_str}_${pf_suffix}
	    if {[file exists ${pf_filename}] == 0} {
		dputs " Computing Longitudinal pf..."
		
		set zepf [pf create]
		pf  init $zepf $amin $noct $nvox $q_lst $size $pf_method {}
		pf compute $zepf m
		pf save $zepf ${pf_filename}
		pf destroy $zepf
		dputs " ok."
	    } else {
		dputs " Pf L already computed."
	    }

	    # 2. TRANSVERSAL
	    dputs " chain T coefficients..."
	    chain m $amin $noct $nvox \
		-filename ${baseDir}/${imaIdf1}_${imaIdf2}_${type}_max_${wavelet}/maxT \
		-boxratio 1 \
		-ecut [list $b1 $b1 $b2 $b2] \
		-nomsg
	    
	    set pf_suffix T
	    set pf_filename ${baseDir}/pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wavelet}_${pf_method_str}_${pf_suffix}
	    if {[file exists ${pf_filename}] == 0} {
		dputs " Computing Longitudinal pf..."
		
		set zepf [pf create]
		pf  init $zepf $amin $noct $nvox $q_lst $size $pf_method {}
		pf compute $zepf m
		pf save $zepf ${pf_filename}
		pf destroy $zepf
		dputs " ok."
	    } else {
		dputs " Pf T already computed."
	    }

	}
	
	dputs "pf computed!"
    }
    logMsg "End."
}

ist $theScr

exit
