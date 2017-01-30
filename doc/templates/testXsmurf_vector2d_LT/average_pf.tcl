proc imStudy::average {{wave gaussian} {fitmin 2} {fitmax 4} } {

    variable baseDir
    variable size
    variable useNMaxSup
    variable useFftw

    # we initialize qmin and qmax for the computation of D(q) and h(q)
    set qmin -3
    set qmax 5

    # initialize the pf partition function data structure
    pf {
	if ![info exists pf(pfid)] {
	    set pf(pfid) 0
	    create
	} elseif { $pf(pfid) > 0} {
	    set pf(pfid) 1
	    clear ::pf::1
	}
    }

    # number of images
    set numImages 2
    
    # base name of pf files
    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}

    # Which type of partition functions are we using ?
    # gradient on maxima or gradient on lines ?
    set pf_method_str "grad_max"
    #set pf_method_str "grad_lines"

    # suffix can be N (normal/regular) L (longitudinal) or T (transversal)
    set pf_suffix {N}
    
    # load partition function pre-computations
    for {set i 0} {$i<$numImages} {incr i} {
	
	set imaIdf1 fBm_vx$i
	set imaIdf2 fBm_vy$i
	
	set pf_filename pf/pf_${type}_${imaIdf1}_${imaIdf2}_${wave}_${pf_method_str}_${pf_suffix}
	pf load ::pf::1 $pf_filename
    }

    # compute partition function
    pf thd ::pf::1

    # perform linear regression fits to recover tau(q), h(q) and D(q)
    pf thdFit ::pf::1 $fitmin $fitmax -qlst $qmin $qmax

    # compute D(h) singularity spectrum
    pf Dh ::pf::1 $qmin $qmax

    # graphical output
    pf setcomments ::pf::1 "$type $wave fit : $fitmin $fitmax pf_type:$pf_suffix pf_method:$pf_method_str\n"
    pf thdDisp ::pf::1 {Dh}

    # sortie graphique des fn de parttion
    pf setcomments ::pf::1 "$type $wave" 
    pf disp ::pf::1 -qlist {-2 -1 -0.5 0 0.5 1 1.4 2}

}






