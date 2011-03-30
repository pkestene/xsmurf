
puts [pid]

set theScr {
    set logCmd dputs
    
    set cut1 [GetBorderSize]
    set cut2 [expr $size-$cut1]

    foreach id ${image_list} {

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
	
	eload3Dsmall ${extDir}_${id}/mmax000 ee000
	ecut3Dsmall ee000 e000 $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
	eload3Dsmall ${extDir}_${id}/mmax001 ee001
	ecut3Dsmall ee001 e001 $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
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
	
	delete ee000 e000

	#load data and make FFT
	for {set scale 2} {$scale<$noct*$nvox} {incr scale} {
	    set scaleF   [format "%.3d" $scale]
	    set scaleFm1 [format "%.3d" [expr $scale-1]]

	    logMsg "   scale $scaleF"
	    eload3Dsmall ${extDir}_${id}/mmax$scaleF ee$scaleF
	    ecut3Dsmall ee$scaleF e$scaleF $cut1 $cut1 $cut1 $cut2 $cut2 $cut2
	    vchain3Dsmall e$scaleFm1 e$scaleF  -simil $similitude

	    delete e$scaleFm1 ee$scaleF

	    foreach q $q_lst {
		#logMsg "  $q"
		set val_on_mod [efct3Dsmall e$scaleF (abs(x))^y $q -vc]
		lappend val_STq_liste($q) $val_on_mod
		set val_on_mod [efct3Dsmall e$scaleF log(abs(x))*(abs(x))^y $q -vc]
		lappend val_STqlogT_liste($q) $val_on_mod
	    }
	}
	delete e$scaleF ee$scaleF

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
	pf save ::pf::1 ${baseDir}/pf/pf_${type}_${imaIdf}_${wavelet} -nstudy
	#pf destroy ::pf::1
    
	logMsg "End $id"   
    }
}
    
ist $theScr

exit


