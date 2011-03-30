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


    # list of identifier for a multi images study
    set image_list {bro1 bro2 bro3 bro4}
    
    set type xsm_fftw${useFftw}_nmaxsup${useNMaxSup}

    # load partition function pre-computations
    foreach iF $image_list {
	pf load ::pf::1  pf/pf_${type}_${iF}_${wave}
    }

    # compute partition function
    pf thd ::pf::1

    # perform linear regression fits to recover tau(q), h(q) and D(q)
    pf thdFit ::pf::1 $fitmin $fitmax -qlst $qmin $qmax

    # compute D(h) singularity spectrum
    pf Dh ::pf::1 $qmin $qmax

    # graphical output
    pf setcomments ::pf::1 "$type $wave fit : $fitmin $fitmax \n"
    pf thdDisp ::pf::1 {Dh}

    # sortie graphique des fn de parttion
    pf setcomments ::pf::1 "$type $wave" 
    pf disp ::pf::1 -qlist {-2 -1 -0.5 0 0.5 1 1.4 2}

}






