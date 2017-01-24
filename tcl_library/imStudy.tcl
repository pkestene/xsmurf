# imStudy.tcl --
#
#       This file implements the Tcl code to compute image study and to
# store its results and its "state".
#
#   Copyright (c) 1998-1999 Nicolas Decoster.
#   Copyright (c) 1998-1999 Centre de Recherche Paul Pascal, Bordeaux, France.
#
#   Copyright (c) 1999-2007 Pierre Kestener.
#   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
#   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
#
#   this file contains codes to implement 2D/3D and scalar/vector WTMM studies.
#
#

# 
# Warning : The names convention for histograms has changed :
#  h_mod		-> h_i_mod	: image modulus histogram.
#  h_arg		-> h_i_arg	: image argument histogram.
#  calendos_image	-> h_i_cal	: image gradient histogram.
#  h_max_mod   	-> h_l_mod	: contour lines modulus histogram.
#  h_max_arg   	-> h_l_arg	: contour lines argument histogram.
#  calendos_max	-> h_l_cal	: contour lines gradient histogram.
#  h_max_line_mod	-> h_m_mod	: maxima (on contour lines) modulus
#					  histogram.
#  h_max_line_arg	-> h_m_arg	: maxima (on contour lines) argument
#					  histogram.
#  calendos_max_line-> h_m_cal	: maxima (on contour lines) gradient
#					  histogram.

package require study
package require pf
package provide imStudy 0.0

namespace eval imStudy {
    variable studyId none

    # Description of the parameters of the study
    variable parDesc
    array set  parDesc {
	baseDir        {requested -dir "Base directory."}
	studyName	{"the study" -string "Name of the study (short)."}
	studyDesc	{"It is a study on something (gah)." -string "Long description of the study."}
	histDirName    {$baseDir/histograms -string "Histogram directory name."}
	histFileName   {$baseDir/min_max_hist -string "Histogram file name where to save hist min and max."}
	logHistFileName   {none -string "Logarithm histogram file name where to save log hist min and max."}
	spFileName	{{}	-string 	"Spectrum file name."}
	first_image    {0 -posint      "Index of the first image."}
	last_image     {0 -posint      "Index of the last image."}
	size           {32 -posint     "Size of the image(s)."}
	wavelet        {gaussian -string "Name of the wavelet."}
	amin           {1 -posfloat    "First scale."}
	noct           {2 -posint      "Number of octaves."}
	nvox           {2 -posint      "Number of vox per octave."}
	nbox_mod       {1024 -posint   "Size of the modulus histograms."}
	nbox_arg       {1024 -posint   "Size of the argument histograms."}
	nbox_gradx     {1024 -posint   "Size of the gradx histograms."}
	nbox_grady     {1024 -posint   "Size of the grady histograms."}
	calendos_size  {128 -posint    "Size of the 2d histograms."}
	useDiskSwap    {0 -boolean     "Use disk swap so as to compute big images. Speed loss..."}
	useCv2d        {0 -boolean     "Use convolution algorithms from the cv2d library."}
	useFftw        {0 -boolean     "Use convolution algorithms from the fftw library."}
	useNMaxSup     {0 -boolean     "Use a rapid non-maxima suppression method for comptuting WTMM points instead of the default computation intensive method that uses 2nd and 3rd derivative to find if a point is a maxima of gradient modulus in the direction of the gradient vector."}
	inMemory       {0 -boolean     "This parameter allows to swith computation mode: the Fourier transform image is kept in memory instead of being saved ina file on disk."}
	imagePath      {$baseDir/$imIdF/image -string "Path for the image file."}
	partSize       {1024 -int      "This is the size of the parts when disk swap is used."}
	isgaussian     {1 -boolean     "Gaussian computation."}
	ismexican      {0 -boolean     "Mexican computation."}
	issave         {0 -boolean     "Save between each image computation."}
	ishisto        {0 -boolean     "Computation of the histograms."}
	ismaxhisto     {0 -boolean     "Computation of the maxima histograms."}
	ishistoinit    {0 -boolean     "Initialization of the histograms."}
	iscontpart     {0 -boolean     "Computation of the continuous partition functions."}
	isthetapart    {0 -boolean     "Computation of the \"theta\" partition functions."}
	followVersion  {1 -int         "Version of the algo for follow command (does not work with disk swap). Must be 0 1 or 2. Other values are like 1."}
	border_percent {0.72 {{-intfloat 0 1}}    "To set the size of the border to cut."}
	similitude     {0.8 {{-intfloat 0 1}}     "Value to use with the vchain command."}
	q_lst          {{-3 -2 -1 0 1 2 3} \
		{{-list -float}} "List of the value of q for partition functions."}
	pos_q_lst      {{0 1 2 3 4 5 6 7} \
		{{-list -posfloat}} "List of the value of q for continuous partition functions."}
	images_lst     {none \
		{{-list -int}} "List of ID numbers (integer) of images used by foreachImage."}
	nThreads       {1 -posint    "Number of threads for fftw computations (only used when xsmurf is linked against the multi-threaded fftw lib.)."}
    }

    # Some constants.
    variable PI 3.1415926535897931
    variable mPI -3.1415926535897931
    variable pi 3.1416
    variable m_pi -3.1416

    # State variables
    variable border_size
    variable hist_min none
    variable hist_max none

    # Wavelets derivatives expression definition.
    variable gaussianDef
    array set gaussianDef {
	dx,r	0
	dx,i	x*exp(-x*x-y*y)
	dy,r	0
	dy,i	y*exp(-x*x-y*y)
	dxx,r	-x*x*exp(-x*x-y*y)
	dxx,i	0
	dxy,r	-y*x*exp(-x*x-y*y)
	dxy,i	0
	dyy,r	-y*y*exp(-x*x-y*y)
	dyy,i	0
	dxxx,r	0
	dxxx,i	-x*x*x*exp(-x*x-y*y)
	dxxy,r	0
	dxxy,i	-x*x*y*exp(-x*x-y*y)
	dxyy,r	0
	dxyy,i	-x*y*y*exp(-x*x-y*y)
	dyyy,r	0
	dyyy,i	-y*y*y*exp(-x*x-y*y)
    }

    variable mexicanDef
    array set mexicanDef {
	dx,r	0
	dx,i	x*(x*x+y*y)*exp(-x*x-y*y)
	dy,r	0
	dy,i	y*(x*x+y*y)*exp(-x*x-y*y)
	dxx,r	-x*x*(x*x+y*y)*exp(-x*x-y*y)
	dxx,i	0
	dxy,r	-y*x*(x*x+y*y)*exp(-x*x-y*y)
	dxy,i	0
	dyy,r	-y*y*(x*x+y*y)*exp(-x*x-y*y)
	dyy,i	0
	dxxx,r	0
	dxxx,i	-x*x*x*(x*x+y*y)*exp(-x*x-y*y)
	dxxy,r	0
	dxxy,i	-x*x*y*(x*x+y*y)*exp(-x*x-y*y)
	dxyy,r	0
	dxyy,i	-x*y*y*(x*x+y*y)*exp(-x*x-y*y)
	dyyy,r	0
	dyyy,i	-y*y*y*(x*x+y*y)*exp(-x*x-y*y)
    }

    # Wavelets derivatives expression definition for FFTW
    # x and y are swapped because fftw lib expects row-major ordered data
    variable gaussianDef_FFTW
    array set gaussianDef_FFTW {
	dx,r	0
	dx,i	y*exp(-x*x-y*y)
	dy,r	0
	dy,i	x*exp(-x*x-y*y)
	dxx,r	-y*y*exp(-x*x-y*y)
	dxx,i	0
	dxy,r	-y*x*exp(-x*x-y*y)
	dxy,i	0
	dyy,r	-x*x*exp(-x*x-y*y)
	dyy,i	0
	dxxx,r	0
	dxxx,i	-y*y*y*exp(-x*x-y*y)
	dxxy,r	0
	dxxy,i	-x*y*y*exp(-x*x-y*y)
	dxyy,r	0
	dxyy,i	-x*x*y*exp(-x*x-y*y)
	dyyy,r	0
	dyyy,i	-x*x*x*exp(-x*x-y*y)
    }

    variable mexicanDef_FFTW
    array set mexicanDef_FFTW {
	dx,r	0
	dx,i	y*(x*x+y*y)*exp(-x*x-y*y)
	dy,r	0
	dy,i	x*(x*x+y*y)*exp(-x*x-y*y)
	dxx,r	-y*y*(x*x+y*y)*exp(-x*x-y*y)
	dxx,i	0
	dxy,r	-y*x*(x*x+y*y)*exp(-x*x-y*y)
	dxy,i	0
	dyy,r	-x*x*(x*x+y*y)*exp(-x*x-y*y)
	dyy,i	0
	dxxx,r	0
	dxxx,i	-y*y*y*(x*x+y*y)*exp(-x*x-y*y)
	dxxy,r	0
	dxxy,i	-x*y*y*(x*x+y*y)*exp(-x*x-y*y)
	dxyy,r	0
	dxyy,i	-x*x*y*(x*x+y*y)*exp(-x*x-y*y)
	dyyy,r	0
	dyyy,i	-x*x*x*(x*x+y*y)*exp(-x*x-y*y)
    }
}


# ist --
# usage : ist args
#
#   Execute a script in the imStudy namespace.
#
# Parameters :
#   args - a list of arg.
#
# Return value :
#   Result of the execution.

proc ist args {
    if {[llength $args] != 0} {
	set cmd [concat namespace inscope imStudy $args]
	set code [catch {eval $cmd} result]
	if {$code != 0} {
	    return -code error $result
	} else {
	    return $result
	}
    }
}


# imStudy::help --
# usage : imStudy::help [proc]
#
#   Get help on imStudy pakage.
#
# Parameters :
#   [proc] - help on a proc.
#
# Return value :
#   Help message.

proc imStudy::help args {
    variable studyId
    variable parDesc

    if {[llength $args] == 0} {
	set resStr "Package imStudy :

  This package handle wtmm images study. Use the \"ist\" command to execute
scripts in the imStudy pakage.
  A study is handle with user-defined parameters stored in a file (see \"init\"
command). This parameters are the usual wtmm parameters (i.e. amin, noct, etc).
You can have an help on parameters with the command \"help parameters\" (or
\"ist help parameters\" from the global interpreter).
  With the same syntax you can acces to help on internal commands (i.e. \"help
init\", or \"ist help init\" from the global interpreter).

List of internal commands :
  init
  help
  paramValues
  scalesLoop
  imagesLoop
  foreachImage
  logMsg
  wtmmg
  wtmmg2d_vector
  wtmmg3d_scalar
  wtmmg3d_vector
  wtmmg_partial
  limitsG
  histInitG
  iHistOneScaleG
  lHistOneScaleG
  mHistOneScaleG
  convHistName
  hidisp
  hread

  This package defines some very usefull scripts for you (lucky guy). To eval
them from the global interpreter you can do this :
    ]ist \{eval \$theScriptName\}

or you can do this :
    ]ist \$\{imStudy::theScriptName\}

  For now there is no inline-help on what the scripts do. If you want to read a
little about them, you must check their \"comment like help message\" in the
file they are defined (i.e. the end of imStudy.tcl).

List of pre-defined scripts :
  initHistScr
  computeWtmmgOneImageScr
  pfInitScr
  computeMaxStatGOneImageScr
  pfEndScr
  completeScr
  maxStatScr"
        return $resStr
    }

    if {[string compare $args parameters] == 0} {
	set resStr "
  Here is the list of the parameters for a image study. If the word
\"requested\" appears in the \"default value\" column, this means the parameter
_must_ be specified in the parameters file (see \"init\" command).
"

	set pMaxLength 0
	set vMaxLength 0
	set searchId [array startsearch parDesc]
	while {[array anymore parDesc $searchId] == 1} {
	    set parameter [array nextelement parDesc $searchId]
	    set length [string length $parameter]
	    if {$length > $pMaxLength} {
		set pMaxLength $length
	    }
	    set value [lindex $parDesc($parameter) 0]
	    set length [string length $value]
	    if {$length > $vMaxLength} {
		set vMaxLength $length
	    }
	}
	# To be changed...
	if {$vMaxLength > 30} {
	    set vMaxLength 30
	}

	# Output...

	set resStr [format \
		"%s\n%-${pMaxLength}s %-${vMaxLength}s %s\n" \
		$resStr parameter "default value" "description"]
	array donesearch parDesc $searchId
	set searchId [array startsearch parDesc]
	while {[array anymore parDesc $searchId] == 1} {
	    set parameter [array nextelement parDesc $searchId]
	    set value [lindex $parDesc($parameter) 0]
	    set help [lindex $parDesc($parameter) 2]
	    set resStr [format \
		    "%s\n%-${pMaxLength}s %-${vMaxLength}s %s" \
		    $resStr $parameter $value $help]
	}
	array donesearch parDesc $searchId

	return $resStr
    }

    global auto_index

    set cmdName [lindex $args 0]
    set code [catch "set auto_index(::imStudy::$cmdName)" result]
    if {$code != 0} {
	return -code error "no command \"$cmdName\" in pakage imStudy"
    }
    set fileName [lindex $result 1]
    if {[string compare [lindex [file split $fileName] end] "imStudy.tcl"] != 0} {
	return -code error "no command \"$cmdName\" in pakage imStudy"
    }

    # Set the default message.
    set resStr "Sorry, no help for `$cmdName'."

    set fileId [open $fileName r]

    while {[gets $fileId line] != -1} {
	if {[string compare $line "# imStudy::$cmdName --"] == 0} {
	    # header line of help comment is found.
	    unset resStr
	    while {[gets $fileId line] != -1} {
		if {[regexp {^# *} $line]} {
		    # the line begins by "#" -> it is a help comment line.
		    # Remove the leading "#" ...
		    regsub {^# } $line "" line
		    regsub {^#} $line "" line
		    # ... and add to result
		    if {[info exist resStr] == 1} {
			set resStr [format "%s\n%s" $resStr "$line"]
		    } else {
			set resStr $line
		    }
		} else {
		    # No more leading "#" -> end of help comment.
		    break
		}
	    }
	    if {[info exist resStr] != 1} {
		# There was no line beginning with "#" after the
		# header line of the help comment.
		set resStr "Sorry, no help for `$cmdName'."
	    }		    
	    # Exit the loop on file lines.
	    break
	}
    }
    close $fileId

    set resStr [format "%s\n\n(file : %s)" $resStr $fileName]


    return $resStr
}


# imStudy::init --
# usage : imStudy::init [-filename str] [-force]
#
#   Init the parameters of the image study. The paramters of the study must be
# stored in a file whose default name is parameters.tcl.
#   The file must contain the base directory for the study. By default this
# procedure must be execute in the base directory. This is done to avoid
# pathname error. This can be escaped by the -force option.
#
# Parameters :
#   none.
#
# Options :
#   -filename : gah !
#      string - file name.
#   -force : escape the directory check.
#
# Return value :
#   None.

proc imStudy::init {args} {
    variable studyId
    variable parDesc

    # Default value of the proc variables.

    set fileName parameters.tcl
    set curDir [pwd]
    set isForce 0

    # Arguments analysis

    set oldArgs $args
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -filename {
		set fileName [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -force {
		set isForce 1
		set args [lreplace $args 0 0]
	    }
	    --  {
		set args [lreplace $args 0 0]
		break
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }

    # If there is a current im study, we end it.
    if {$studyId != "none"} {
	study::end $studyId
    }
    InitVar

    # Init the study.
    set cmd [concat study::init -desc parDesc $oldArgs]
    set code [catch $cmd result]
    if {$code != 0} {
	return -code error $result
    }
    set studyId $result

    # Create a variable for each parameter.
    foreach {key value} [array get $studyId] {
	variable $key
    }
    study::param2var $studyId

    if {$iscontpart} {
	# Temporary message. Remove it as soon as possible.
	puts "init : *** Sorry, no continuous partition functions for now... ***" 
	puts "init : *** set \"iscontpart\" to 0.                              ***" 
    }

    variable border_size
    set border_size [GetBorderSize]

    return
}


# imStudy::InitVar -- PRIVATE
# usage : imStudy::InitVar
#
#   Init all variables.
#
# Parameters :
#   None.
#
# Return value :
#   None.

proc imStudy::InitVar {} {
    # Usefull variables.

    variable scale	-1	;# Current scale.
    variable scaleId	-1	;# Current scale id.
    variable scaleIdF	0	;# Current formatted scale id (i.e. 000, 001, etc).
    variable oct	0	;# Current octave.
    variable octList    {}      ;# used in foreachscale
    variable vox	0	;# Current voice.
    variable voxList    {}      ;# used in foreachscale; must have same
    # size as octList


    variable imId	-1	;# Current image id.
    variable imIdF	0	;# Current formatted image id (i.e. 00, 01, etc).

    variable scalesLoopLock 0
    variable imagesLoopLock 0
    variable foreachImageLock 0
}


# imStudy::paramValues --
# usage : imStudy::paramValues [-channel channelId] [-helpmsg]
#
#   Puts the name and the value of all parameters on stdout.
#
# Parameters :
#
# Options :
#   -channel : use another channel.
#      channelId - the channel id.
#   -helpmsg : add an help for each parameter at the end of the line.
#
# Return value :
#   None.

proc imStudy::paramValues args {
    variable studyId
    variable parDesc

    set theArgs ""
    while {[string match -* $args]} {
	switch -glob -- [lindex $args 0] {
	    -helpmsg {
		lappend theArgs [lindex $args 0]
		lappend theArgs parDesc
		set args [lreplace $args 0 0]
	    }
	    default {
		lappend theArgs [lindex $args 0]
		set args [lreplace $args 0 0]
	    }
	}
    }

    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set cmd "study::param $studyId $theArgs"
    set code [catch $cmd result]
    if {$code != 0} {
	return -code error $result
    }

    return
}


# imStudy::scalesLoop --
# usage : imStudy::scalesLoop script
#
#   Loop on the scales of the study. Here is the list of modified variables :
# scale, scaleId, scaleIdF, oct and vox. The loop begins at the scale
# corresponding to the value of scaleId before the loop.
#
# Parameters :
#   script  - script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::scalesLoop {script} {
    variable amin
    variable noct
    variable nvox
    variable scale
    variable scaleId
    variable scaleIdF
    variable oct
    variable vox
    variable scalesLoopLock

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    if {$scalesLoopLock == 1} {
	return -code error "there's an other running scales loop"
    }
    set scalesLoopLock 1

    set beginId $scaleId

    for { set oct 0;set num 0}\
	    { $oct < $noct} \
	    { incr oct } {
	for {set vox 0} \
		{ $vox < $nvox } \
		{ incr vox ; incr num} {
	    if {$num >= $beginId} {
		set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
		set scale [expr $scale*(6/0.86)]
		set scaleId $num
		set scaleIdF [format "%.3d" $num]
		set code [catch {uplevel $script} result]
		switch $code {
		    1 {
			global errorInfo
			set scalesLoopLock 0
			#error "$result $errorInfo"
			error $result
		    }
		    2 {
			set scalesLoopLock 0
			return $result
		    }
		    3 {
			set scalesLoopLock 0
			return
		    }
		    4 {continue}
		    default {}
		}
	    }
	}
    }
    set scaleId -1
    set scalesLoopLock 0

    return $result
}


# imStudy::foreachscale --
# usage : imStudy::foreachscale octList voxList script
#
#   Loop on the scales of $scaleList. Here is the list of modified variables :
# scale, scaleId, scaleIdF, oct and vox.
#
# Parameters :
#   script    - script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::foreachscale {script} {
    variable amin
    variable noct
    variable octList
    variable nvox
    variable voxList
    variable scale
    variable scaleId
    variable scaleIdF
    variable oct
    variable vox
    variable scalesLoopLock

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    if {$scalesLoopLock == 1} {
	return -code error "there's an other running scales loop"
    }
    set scalesLoopLock 1

    #set beginId $scaleId

    foreach oct $octList vox $voxList {
	set scale [expr $amin*pow(2,$oct+($vox/double($nvox)))]
	set scale [expr $scale*(6/0.86)]
	set scaleId [expr $oct*$nvox+$vox]
	set scaleIdF [format "%.3d" $scaleId]
	set code [catch {uplevel $script} result]
	switch $code {
	    1 {
		global errorInfo
		set scalesLoopLock 0
		#error "$result $errorInfo"
		error $result
	    }
	    2 {
		set scalesLoopLock 0
		return $result
	    }
	    3 {
		set scalesLoopLock 0
		return
	    }
	    4 {continue}
	    default {}
	    
	}
    }

    set scaleId -1
    set scalesLoopLock 0
    
    return $result
}


# imStudy::setCurIm --
# usage : imStudy::setCurIm int
#
#   Set the current image id. Here is the list of modified variables : imId and
# imIdF. If the requested id is not valid (i.e. greater than max or lesser than
# min) the nearest id is taken.
#
# Parameters :
#   int - value of the id.
#
# Return value :
#   The imId.

proc imStudy::setCurIm {reqId} {
    variable first_image
    variable last_image
    variable imId
    variable imIdF

    if {$reqId < $first_image} {
	set reqId $first_image
    }
    if {$reqId > $last_image} {
	set reqId $last_image
    }
    set imId $reqId
    set imIdF [format "%.2d" $reqId]

    return $imId
}


# imStudy::imagesLoop --
# usage : imStudy::imagesLoop script
#
#   Loop on the images of the study. Here is the list of modified variables :
# imId and imIdF.  The loop begins at the image corresponding to the value of
# imId before the loop.
#
# Parameters :
#   script  - script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::imagesLoop {script} {
    variable first_image
    variable last_image
    variable imId
    variable imIdF
    variable imagesLoopLock

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    if {$imagesLoopLock == 1} {
	return -code error "there's an other running images loop"
    }
    set imagesLoopLock 1

    set beginId $imId

    for {set i $first_image} {$i <= $last_image} {incr i} {
	if {$i >= $beginId} {
	    set imId $i
	    set imIdF [format "%.2d" $i]
	    set code [catch {uplevel $script} result]
	    switch $code {
		1 {
		    global errorInfo
		    set imagesLoopLock 0
		    #error "$result $errorInfo"
		    error $result
		}
		2 {
		    set imagesLoopLock 0
		    return $result
		}
		3 {
		    set imagesLoopLock 0
		    return
		}
		4 {continue}
		default {}
	    }
	}
    }
    set imId -1
    set imagesLoopLock 0

    return $result
}


# imStudy::foreachImage --
# usage : imStudy::foreachImage script
#
#   Loop on the images of the study; id numbers must be specified in
# a variable images_lst.
# Here is the list of modified variables :
# imId and imIdF.  The loop begins at the image corresponding to the value of
# imId before the loop.
#
# Parameters :
#   script  - script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::foreachImage {script} {
    variable first_image
    variable last_image
    variable imId
    variable imIdF
    variable foreachImageLock
    variable images_lst

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    if {$foreachImageLock == 1} {
	return -code error "there's an other running images loop : foreachImage"
    }
    set foreachImageLock 1

    set beginId $imId

    foreach i $images_lst {
	if {$i >= $beginId} {
	    set imId $i
	    set imIdF [format "%.3d" $i]
	    set code [catch {uplevel $script} result]
	    switch $code {
		1 {
		    global errorInfo
		    set foreachImageLock 0
		    #error "$result $errorInfo"
		    error $result
		}
		2 {
		    set foreachImageLock 0
		    return $result
		}
		3 {
		    set foreachImageLock 0
		    return
		}
		4 {continue}
		default {}
	    }
	}
    }
    set imId -1
    set foreachImageLock 0

    return $result
}




# imStudy::ConvolOneScale -- PRIVATE
# usage : imStudy::ConvolOneScale str str real str str
#
#   Compute the convolution at one scale with a filter.
#
# Parameters :
#   string - Path for the image that contains the fourier transform of the
#            source image.
#   string - Name of the result.
#   real   - Scale of the convolution.
#   string - Expression for the real part of the fourier transform of the
#            filter.
#   string - Expression for the imaginary part of the fourier transform of the
#            filter.
#
# Return value :
#   None.

proc imStudy::ConvolOneScale {ftPath cvName scale expr1 expr2} {
    variable useDiskSwap
    variable useFftw
    variable inMemory
    variable nThreads

    if {$inMemory==1} {
	# Tourier image is in memory, we just need to copy it
	icopy $ftPath $cvName
    } else {
	# Fourier image is on disk, we need to load it
	iload $ftPath $cvName
    }
    if {$useFftw == 1} {
	#logMsg "ConvolOneScale with fftw..."
	#logMsg "ifftwfilter $cvName $scale $expr1 $expr2"
	ifftwfilter $cvName $scale $expr1 $expr2
	ifftw2d $cvName -threads $nThreads
	#logMsg "debug convol fftw..."
    } else {
	iconvol $cvName gah $scale -new $expr1 $expr2
	igfft   $cvName $cvName -reverse
    }

    if {$useDiskSwap == 1} {
	isave $cvName
	delete $cvName
    }
}


# imStudy::ConvolOneScale3D -- PRIVATE
# usage : imStudy::ConvolOneScale3D str str real str str
#
#   Compute the convolution at one scale with a filter (3D data).
#
# Parameters :
#   string - Path for the image that contains the fourier transform of the
#            source image.
#   string - Name of the result.
#   real   - Scale of the convolution.
#   string - Expression for the real part of the fourier transform of the
#            filter.
#   string - Expression for the imaginary part of the fourier transform of the
#            filter.
#
# Return value :
#   None.

proc imStudy::ConvolOneScale3D {ftPath cvName scale args} {
    #variable useDiskSwap
    variable wavelet
    variable useFftw
    variable nThreads

    i3Dload $ftPath $cvName
    
    switch -exact -- $wavelet {

	gaussian {
	    ifftw3dfilter $cvName $scale -gaussian $args 
	    ifftw3d $cvName -threads $nThreads	
	}
	mexican {
	    ifftw3dfilter $cvName $scale -mexican $args 
	    ifftw3d $cvName -threads $nThreads
	}

    }

#     if {$useDiskSwap == 1} {
# 	i3Dsave $cvName
# 	delete $cvName
#     }
}


# imStudy::LoadAndCut -- PRIVATE
# usage : imStudy::LoadAndCut list int int int
#
# blabla....
#
# Parameters :
#   list  - names of the images
#   2 int - coordinates of the cut
#   int   - size of the cut
#
# Return value :
#   None

proc imStudy::LoadAndCut {imLst x y n} {
    foreach name $imLst {
	iload $name
	iicut $name $name $x $y $n $n
    }

    return
}


proc imStudy::SeekLoad {imLst pos length} {
    foreach name $imLst {
	iload $name -seek $pos $length
    }

    return
}


# imStudy::LoadAndInsertAndSave -- PRIVATE
# usage : imStudy::LoadAndInsertAndSave list int int
#
# blabla....
#
# Parameters :
#   list  - names of the images
#   2 int  - coordinates of the insert
#
# Return value :
#   None

proc imStudy::LoadAndInsertAndSave {imLst x y} {
    foreach name $imLst {
	iload $name __LoadAndInsertAndSave_tmp
	iinsert __LoadAndInsertAndSave_tmp $name $x $y
	isave __LoadAndInsertAndSave_tmp $name
    }
    delete __LoadAndInsertAndSave_tmp

    return
}


proc imStudy::SeekSave {imLst pos length} {
    foreach name $imLst {
	isave $name -seek $pos $length
    }

    return
}


# imStudy::WtmmgCurrentScale -- PRIVATE
# usage : imStudy::WtmmgCurrentScale str args
#
#   Compute the wtmm at the current scale using the gradient lines method.
# This command computes the modulus and the argument for each point of the image
# and the contour lines.
#
# Parameters :
#   string - Path for the image that contains the fourier transform of the
#            source image.
#
# Options :
#   -vector2d [string] : compute the wtmm edges of a 2d vector field
#   -3d : interpreter input image as a Image3D data structure and compute
#         the wtmm edges (surfaces)
#   -vector3d [string string] : compute the wtmm edges of a 3d vector field
#   -lt : this option used together with -vector2d or -vector3d will generated
#         longitudinal / transversal information
#
# Return value :
#   List of names of the new (ext) images : modulus image, argument image and
# contour lines ext image.

proc imStudy::WtmmgCurrentScale {ftPath args} {
    variable size
    variable partSize
    variable scale
    variable scaleIdF
    variable wavelet
    variable gaussianDef
    variable mexicanDef
    variable gaussianDef_FFTW
    variable mexicanDef_FFTW
    variable useDiskSwap
    variable useCv2d
    variable useFftw
    variable useNMaxSup
    variable inMemory
    variable imageName
    variable imagePath
    variable cv2dPath
    variable baseDir

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # remember that if inMemory is equal to 1, ftPath is an image instead of 
    # just a path to an image

    # set default values for options
    set isVector2d 0
    set is3d 0
    set isVector3d 0
    set islt 0
    
    # Arguments analysis
    set oldArgs $args
    while {[string match -* $args]} {
	switch -exact -- [lindex $args 0] {
	    -vector2d {
		set isVector2d 1
		set ftPath2 [lindex $args 1]
		set args [lreplace $args 0 1]
	    }
	    -3d {
		set is3d 1
		set args [lreplace $args 0 0]
	    }
	    -vector3d {
		set isVector3d 1
		set ftPath2 [lindex $args 1]
		set ftPath3 [lindex $args 2]
		set args [lreplace $args 0 2]
	    }
	    -lt {
		set islt 1
		set args [lreplace $args 0 0]
	    }
	    default {
		return -code error "unknown option \"[lindex $args 0]\""
	    }
	}
    }
    
    if {$isVector2d == 1} {
	switch -exact -- $wavelet {
	    gaussian {
		ConvolOneScale $ftPath  dx1 $scale $gaussianDef_FFTW(dx,r) $gaussianDef_FFTW(dx,i)
		ConvolOneScale $ftPath2 dx2 $scale $gaussianDef_FFTW(dx,r) $gaussianDef_FFTW(dx,i)
		ConvolOneScale $ftPath  dy1 $scale $gaussianDef_FFTW(dy,r) $gaussianDef_FFTW(dy,i)
		ConvolOneScale $ftPath2 dy2 $scale $gaussianDef_FFTW(dy,r) $gaussianDef_FFTW(dy,i)
	    }
	    mexican {
		ConvolOneScale $ftPath  dx1 $scale $mexicanDef_FFTW(dx,r) $mexicanDef_FFTW(dx,i)
		ConvolOneScale $ftPath2 dx2 $scale $mexicanDef_FFTW(dx,r) $mexicanDef_FFTW(dx,i)
		ConvolOneScale $ftPath  dy1 $scale $mexicanDef_FFTW(dy,r) $mexicanDef_FFTW(dy,i)
		ConvolOneScale $ftPath2 dy2 $scale $mexicanDef_FFTW(dy,r) $mexicanDef_FFTW(dy,i)
	    }
	}
    } elseif {$isVector3d == 1} {
	# the wavelet type (gaussian or mexican) is tested inside
	# ConvolOneScale3D
	ConvolOneScale3D $ftPath dx1 $scale -x
	ConvolOneScale3D $ftPath dy1 $scale -y
	ConvolOneScale3D $ftPath dz1 $scale -z

	ConvolOneScale3D $ftPath2 dx2 $scale -x
	ConvolOneScale3D $ftPath2 dy2 $scale -y
	ConvolOneScale3D $ftPath2 dz2 $scale -z

	ConvolOneScale3D $ftPath3 dx3 $scale -x
	ConvolOneScale3D $ftPath3 dy3 $scale -y
	ConvolOneScale3D $ftPath3 dz3 $scale -z

    } elseif {$is3d == 1} {
	# the wavelet type (gaussian or mexican) is tested inside
	# ConvolOneScale3D
	ConvolOneScale3D $ftPath dx $scale -x
	ConvolOneScale3D $ftPath dy $scale -y
	ConvolOneScale3D $ftPath dz $scale -z
    } else {
	# scalar 2d    
	switch -exact -- $wavelet {
	    gaussian {
		if {$useFftw == 1} {
		    #logMsg "using fftw..."
		    ConvolOneScale $ftPath dx	$scale $gaussianDef_FFTW(dx,r)	$gaussianDef_FFTW(dx,i)
		    ConvolOneScale $ftPath dy	$scale $gaussianDef_FFTW(dy,r)	$gaussianDef_FFTW(dy,i)
		    if {$useNMaxSup == 0} {
			ConvolOneScale $ftPath dxx	$scale $gaussianDef_FFTW(dxx,r)	$gaussianDef_FFTW(dxx,i)
			ConvolOneScale $ftPath dxy	$scale $gaussianDef_FFTW(dxy,r)	$gaussianDef_FFTW(dxy,i)
			ConvolOneScale $ftPath dyy	$scale $gaussianDef_FFTW(dyy,r)	$gaussianDef_FFTW(dyy,i)
			ConvolOneScale $ftPath dxxx	$scale $gaussianDef_FFTW(dxxx,r)	$gaussianDef_FFTW(dxxx,i)
			ConvolOneScale $ftPath dxxy	$scale $gaussianDef_FFTW(dxxy,r)	$gaussianDef_FFTW(dxxy,i)
			ConvolOneScale $ftPath dxyy	$scale $gaussianDef_FFTW(dxyy,r)	$gaussianDef_FFTW(dxyy,i)
			ConvolOneScale $ftPath dyyy	$scale $gaussianDef_FFTW(dyyy,r)	$gaussianDef_FFTW(dyyy,i)
		    }
		} else { 
		    # use gfft
		    ConvolOneScale $ftPath dx	$scale $gaussianDef(dx,r)	$gaussianDef(dx,i)
		    ConvolOneScale $ftPath dy	$scale $gaussianDef(dy,r)	$gaussianDef(dy,i)
		    if {$useNMaxSup == 0} {
			ConvolOneScale $ftPath dxx	$scale $gaussianDef(dxx,r)	$gaussianDef(dxx,i)
			ConvolOneScale $ftPath dxy	$scale $gaussianDef(dxy,r)	$gaussianDef(dxy,i)
			ConvolOneScale $ftPath dyy	$scale $gaussianDef(dyy,r)	$gaussianDef(dyy,i)
			ConvolOneScale $ftPath dxxx	$scale $gaussianDef(dxxx,r)	$gaussianDef(dxxx,i)
			ConvolOneScale $ftPath dxxy	$scale $gaussianDef(dxxy,r)	$gaussianDef(dxxy,i)
			ConvolOneScale $ftPath dxyy	$scale $gaussianDef(dxyy,r)	$gaussianDef(dxyy,i)
			ConvolOneScale $ftPath dyyy	$scale $gaussianDef(dyyy,r)	$gaussianDef(dyyy,i)
		    }
		}
	    }
	    mexican {
		if {$useCv2d == 0} {
		    if {$useFftw == 1} {
			ConvolOneScale $ftPath dx	$scale $mexicanDef_FFTW(dx,r)	$mexicanDef_FFTW(dx,i)
			ConvolOneScale $ftPath dy	$scale $mexicanDef_FFTW(dy,r)	$mexicanDef_FFTW(dy,i)
			if {$useNMaxSup == 0} {
			    ConvolOneScale $ftPath dxx	$scale $mexicanDef_FFTW(dxx,r)	$mexicanDef_FFTW(dxx,i)
			    ConvolOneScale $ftPath dxy	$scale $mexicanDef_FFTW(dxy,r)	$mexicanDef_FFTW(dxy,i)
			    ConvolOneScale $ftPath dyy	$scale $mexicanDef_FFTW(dyy,r)	$mexicanDef_FFTW(dyy,i)
			    ConvolOneScale $ftPath dxxx	$scale $mexicanDef_FFTW(dxxx,r)	$mexicanDef_FFTW(dxxx,i)
			    ConvolOneScale $ftPath dxxy	$scale $mexicanDef_FFTW(dxxy,r)	$mexicanDef_FFTW(dxxy,i)
			    ConvolOneScale $ftPath dxyy	$scale $mexicanDef_FFTW(dxyy,r)	$mexicanDef_FFTW(dxyy,i)
			    ConvolOneScale $ftPath dyyy	$scale $mexicanDef_FFTW(dyyy,r)	$mexicanDef_FFTW(dyyy,i)
			}
		    } else {
			ConvolOneScale $ftPath dx	$scale $mexicanDef(dx,r)	$mexicanDef(dx,i)
			ConvolOneScale $ftPath dy	$scale $mexicanDef(dy,r)	$mexicanDef(dy,i)
			if {$useNMaxSup == 0} {
			    ConvolOneScale $ftPath dxx	$scale $mexicanDef(dxx,r)	$mexicanDef(dxx,i)
			    ConvolOneScale $ftPath dxy	$scale $mexicanDef(dxy,r)	$mexicanDef(dxy,i)
			    ConvolOneScale $ftPath dyy	$scale $mexicanDef(dyy,r)	$mexicanDef(dyy,i)
			    ConvolOneScale $ftPath dxxx	$scale $mexicanDef(dxxx,r)	$mexicanDef(dxxx,i)
			    ConvolOneScale $ftPath dxxy	$scale $mexicanDef(dxxy,r)	$mexicanDef(dxxy,i)
			    ConvolOneScale $ftPath dxyy	$scale $mexicanDef(dxyy,r)	$mexicanDef(dxyy,i)
			    ConvolOneScale $ftPath dyyy	$scale $mexicanDef(dyyy,r)	$mexicanDef(dyyy,i)
			}
		}
		} else {
		    # the following is old and probably not working...
		    # to be removed ???
		    if {$useDiskSwap == 1} {
			iload $imagePath __image_[pid]
			foreach w {dx dy dxx dxy dyy dxxx dxxy dxyy dyyy} {
			    iload ${cv2dPath}/${w}$scaleIdF __wavelet
			    cv2dn __image_[pid] __wavelet __res -mp
			    isave __res $w
			}
			delete __wavelet
			delete __res
			delete __image_[pid]
		    } else {
			foreach w {dx dy dxx dxy dyy dxxx dxxy dxyy dyyy} {
			    iload ${cv2dPath}/${w}$scaleIdF __wavelet
			    cv2dn $imageName __wavelet $w -mp
			}
			delete __wavelet
		    }
		}
	    }
	}
    }


    #
    # here begins the computation of the wtmm lines
    #
    if {$useDiskSwap == 0} {
	if {$useNMaxSup == 1} {
	    # use a simple non-maxima suppression routine
	    #logMsg "computing WTMM lines with non-maxima suppression routine"
	    if {$isVector2d == 1} {
		# vector 2d
		if {$islt == 1} {
		    ###### TODO #####
		} else {
		    wtmm2d dx1 dy1 max${scaleIdF} $scale mod$scaleIdF arg$scaleIdF -vector dx2 dy2
		}
	    } elseif {$isVector3d == 1} {
		# vector 3d
		wtmm3d dx1 dy1 dz1 max${scaleIdF} $scale mod$scaleIdF mmax$scaleIdF -vector dx2 dy2 dz2 dx3 dy3 dz3
	    } elseif {$is3d == 1} {
		# scalar 3d
		wtmm3d dx dy dz max${scaleIdF} $scale mod$scaleIdF mmax$scaleIdF
	    } else {
		# scalar 2d
		wtmm2d dx dy max${scaleIdF} $scale mod$scaleIdF arg$scaleIdF
	    }
	} else {
	    # use a more computational intensive method; see code of
	    # the C-defined routine named follow in interpreter/wt2d_cmds.c
	    garg dx dy arg$scaleIdF
	    gmod dx dy mod$scaleIdF
	    gkapap kapap${scaleIdF} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
	    delete dxxx dxxy dxyy dyyy
	    gkapa kapa${scaleIdF} dx dxx dy dyy dxy
	    delete dx dy dxx dxy dyy
	    
	    variable followVersion
	    
	    switch $followVersion {
		0 {
		    follow kapa${scaleIdF} kapap${scaleIdF} \
			mod${scaleIdF} arg${scaleIdF} max$scaleIdF $scale -old
		}
		1 {
		    follow kapa${scaleIdF} kapap${scaleIdF} \
			mod${scaleIdF} arg${scaleIdF} max$scaleIdF $scale
		}
		2 {
		    follow kapa${scaleIdF} kapap${scaleIdF} \
			mod${scaleIdF} arg${scaleIdF} max$scaleIdF $scale -v2
		}
		default {
		    follow kapa${scaleIdF} kapap${scaleIdF} \
			mod${scaleIdF} arg${scaleIdF} max$scaleIdF $scale
		}
	    }
	    
	    delete kapa${scaleIdF}
	    delete kapap${scaleIdF}
	}
    } else {
	set nb [expr $size/$partSize]
	inull mod${scaleIdF} $size
	isave mod${scaleIdF}
	delete mod${scaleIdF}
	inull arg${scaleIdF} $size
	isave arg${scaleIdF}
	delete arg${scaleIdF}
	inull kapa${scaleIdF} $size
	isave kapa${scaleIdF}
	delete kapa${scaleIdF}
	inull kapap${scaleIdF} $size
	isave kapap${scaleIdF}
	delete kapap${scaleIdF}

	set dataSize [expr { $size*$size }]
	set length [expr { $partSize*$partSize }]

	for {set pos 0} {$pos < $dataSize} {incr pos $length} {
	    if {$pos+$length > $dataSize} {
		set length [expr { $dataSize-$pos }]
	    }
	    SeekLoad {dx dxx dy dyy dxy dxxx dxxy dxyy dyyy} $pos $length
	    garg dx dy arg$scaleIdF
	    gmod dx dy mod$scaleIdF
	    gkapap kapap${scaleIdF} dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
	    delete dxxx dxxy dxyy dyyy
	    gkapa kapa${scaleIdF} dx dxx dy dyy dxy
	    delete dx dy dxx dxy dyy
	    SeekSave [list mod$scaleIdF arg$scaleIdF kapa${scaleIdF} kapap${scaleIdF}] $pos $length
	}

	delete arg${scaleIdF}
	delete mod${scaleIdF}

	iload kapa${scaleIdF}
	iload kapap${scaleIdF}
	follow2 kapa${scaleIdF} kapap${scaleIdF} max$scaleIdF $scale
	delete kapap${scaleIdF} kapa${scaleIdF}

	for {set pos 0} {$pos < $dataSize} {incr pos $length} {
	    if {$pos+$length > $dataSize} {
		set length [expr { $dataSize-$pos }]
	    }
	    iload arg${scaleIdF} -seek $pos $length $size
	    eiset max$scaleIdF \
		    -arg arg${scaleIdF} \
		    -position [expr {$pos%$size}] [expr {$pos/$size}] \
		    -flag
	    iload mod${scaleIdF} -seek $pos $length $size
	    iload kapa${scaleIdF} -seek $pos $length $size
	    eiset max$scaleIdF \
		    -mod mod${scaleIdF} kapa${scaleIdF} \
		    -position [expr {$pos%$size}] [expr {$pos/$size}] \
		    -flag
	}
	delete kapa${scaleIdF}

	file delete dx
	file delete dy
	file delete dxx
	file delete dxy
	file delete dyy
	file delete dxxx
	file delete dxxy
	file delete dxyy
	file delete dyyy
	file delete mod${scaleIdF}
	file delete arg${scaleIdF}
	file delete kapa${scaleIdF}
	file delete kapap${scaleIdF}
    }

    if {$is3d == 1 || $isVector3d} {
	return [list mod${scaleIdF} max$scaleIdF mmax$scaleIdF]
    } else {
	return [list mod${scaleIdF} arg${scaleIdF} max$scaleIdF]
    }
}


# imStudy::logMsg --
# usage : imStudy::logMsg str
#
#   Log a message in a way determined by the "logCmd" package variable.
#
# Parameters :
#   str - The string to log.
#
# Return value :
#   None.

proc imStudy::logMsg {msgStr} {
    variable logCmd

    catch {$logCmd $msgStr}
}


# imStudy::updateSpectrum --
# usage : imStudy::updateSpectrum image
#
#   Compute, (eventually) add and save a 2d spectrum using the fourier transform
# of an image.
#
# Parameters :
#   image - The fourier transform of the image (gfft format).
#
# Return value :
#   None.

proc imStudy::updateSpectrum {gfftIm} {
    variable spFileName
    variable size

    if {$spFileName == {}} {
	return
    }

    # Get the path from the name description.

    set theNamespace [namespace current]
    set theSpFileName [namespace inscope $theNamespace {subst $spFileName}]

    # Load or create the base spectrum

    if {[file exists $theSpFileName] == 1} {
	iload $theSpFileName __isp
    } else {
	inull __isp $size
    }

    # Compute the current spectrum.

    igfft2ri $gfftIm __r __i
    icomb __r __r x*x __r2_tmp
    icomb __i __i y*y __i2_tmp
    icomb __r2_tmp __i2_tmp x+y __isp_tmp
    iswap __isp_tmp

    # Add and save.

    icomb __isp __isp_tmp x+y __isp
    isave __isp $theSpFileName

    delete __r __i __r2_tmp __i2_tmp __isp __isp_tmp

    return
}


# imStudy::wtmmg --
# usage : imStudy::wtmmg str [["cond"] expr] script
#
#   Compute the wtmm at all current scales using the gradient lines method. For
# each scale this command computes the modulus and the argument for each point
# of the image and the contour lines.
#
# Parameters :
#   string - Image name.
#   expr   - At each scale this expression is evaluated. If it is false the wtmm
#            is not computed for this scale and the script is not executed.
#   script - Script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::wtmmg {args} {
    variable scale
    variable scaleId
    variable oct
    variable vox
    variable spFileName
    variable useCv2d
    variable useFftw
    variable useDiskSwap
    variable inMemory
    variable nThreads

    variable imageName

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # Get args.

    switch [llength $args] {
	4 {
	    # The second arg must be the string "cond"

	    if {[string compare [lindex $args 1] "cond"] != 0} {
		return -code error "wrong \# args: extra words after body script in \"wtmmg\" command"
	    }
	    set image [lindex $args 0]
	    set condition [lindex $args 2]
	    set script [lindex $args 3]
	}
	3 {
	    set image [lindex $args 0]
	    set condition [lindex $args 1]
	    set script [lindex $args 2]
	}
	2 {
	    set image [lindex $args 0]
	    set condition 1
	    set script [lindex $args 1]
	}
	1 {
	    return -code error "wrong \# args: no script after image name"
	}
	0 {
	    return -code error "wrong \# args: no image name"
	}
	default {
	    return -code error "wrong \# args: extra words after body script in \"wtmmg\" command"
	}
    }

    set theDir [pwd]

    if {$useFftw == 1} {
	isave   $image __image_[pid] 
	ifftw2d $image  -threads $nThreads
	if {$inMemory == 1} {
	    icopy   $image __ft_[pid]
	} else {
	    isave   $image __ft_[pid]
	}
	delete  $image
    } elseif {$useCv2d == 0} {
	igfft   $image __ft_[pid]
	isave   $image __image_[pid]
	delete  $image
	isave   __ft_[pid]

	if {$spFileName != {}} {
	    updateSpectrum __ft_[pid]
	}

	delete  __ft_[pid]
    } else {
	set imageName $image
	if {$useDiskSwap == 1} {
	    isave   $image __image_[pid]
	    delete  $image
	    variable imagePath
	    set imagePath [pwd]/__image_[pid]
	}
    }

    set result {}
    
    scalesLoop {
	if [uplevel expr $condition] {
	    logMsg "  Octave $oct - vox $vox - scale $scale ( $scaleId )"
	    set result [WtmmgCurrentScale __ft_[pid]]
	    set modId [lindex $result 0]
	    set argId [lindex $result 1]
	    set maxId [lindex $result 2]
	    set code [catch {uplevel $script} result]
	    switch $code {
		1 {
		    if {$useCv2d == 0} {
			iload ${theDir}/__image_[pid] $image
			catch {file delete ${theDir}/__image_[pid]}
			catch {file delete ${theDir}/__ft_[pid]}
		    }
			error $result
		}
		2 {
		    if {$useCv2d == 0} {
			iload ${theDir}/__image_[pid] $image
			catch {file delete ${theDir}/__image_[pid]}
			catch {file delete ${theDir}/__ft_[pid]}
		    }
		    return $result}
		3 {break}
		4 {continue}
		default {}
	    }
	}
    }

    if {$useCv2d == 0} {
	iload ${theDir}/__image_[pid] $image
	catch {file delete ${theDir}/__image_[pid]}
	catch {file delete ${theDir}/__ft_[pid]}
    }

    return $result
}


# imStudy::wtmmg2d_vector --
# usage : imStudy::wtmmg2d_vector str str [["cond"] expr] script
#
#   Compute the wtmm of a 2D -> 2D vector field at all current scales using
# the gradient lines method. For each scale this command computes the modulus
# and the argument for each point of the image and the contour lines.
#
# Parameters :
#   string - 1st component Image name.
#   string - 2nd component Image name.
#   expr   - At each scale this expression is evaluated. If it is false the wtmm
#            is not computed for this scale and the script is not executed.
#   script - Script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::wtmmg2d_vector {args} {
    variable scale
    variable scaleId
    variable oct
    variable vox
    variable spFileName
    variable useCv2d
    variable useFftw
    variable useDiskSwap
    variable nThreads

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # Get args.

    switch [llength $args] {
	5 {
	    # The third arg must be the string "cond"

	    if {[string compare [lindex $args 2] "cond"] != 0} {
		return -code error "wrong \# args: extra words after body script in \"wtmmg2d_vector\" command"
	    }
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set condition [lindex $args 3]
	    set script [lindex $args 4]
	}
	4 {
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set condition [lindex $args 2]
	    set script [lindex $args 3]
	}
	3 {
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set condition 1
	    set script [lindex $args 2]
	}
	2 {
	    return -code error "wrong \# args: no script after image name"
	}
	1 {
	    return -code error "wrong \# args: only one image name given"
	}
	0 {
	    return -code error "wrong \# args: no image names"
	}
	default {
	    return -code error "wrong \# args: extra words after body script in \"wtmmg2d_vector\" command"
	}
    }

    set theDir [pwd]

    if {$useFftw == 1} {
	isave   $image1 __image1_[pid] 
	ifftw2d $image1  -threads $nThreads
	isave   $image1 __ft1_[pid]	
	delete  $image1

	isave   $image2 __image2_[pid] 
	ifftw2d $image2  -threads $nThreads
	isave   $image2 __ft2_[pid]	
	delete  $image2
    }

    set result {}
    scalesLoop {
	if [uplevel expr $condition] {
	    logMsg "  Octave $oct - vox $vox - scale $scale ( $scaleId )"
	    set result [WtmmgCurrentScale $theDir/__ft1_[pid] -vector2d $theDir/__ft2_[pid]]
	    set modId [lindex $result 0]
	    set argId [lindex $result 1]
	    set maxId [lindex $result 2]
	    set code [catch {uplevel $script} result]
	}
    }

    catch {file delete ${theDir}/__image1_[pid]}
    catch {file delete ${theDir}/__image2_[pid]}
    catch {file delete ${theDir}/__ft1_[pid]}
    catch {file delete ${theDir}/__ft2_[pid]}
    
    return $result
}


# imStudy::wtmmg3d_scalar --
# usage : imStudy::wtmmg3d_scalar str [["cond"] expr] script
#
#   Compute the wtmm of a 3D scalar field at all current scales using
# the gradient surfaces method. For each scale this command computes the 
# modulus and the argument for each point of the image and the contour
# surfaces.
#
# Parameters :
#   string - Image name.
#   expr   - At each scale this expression is evaluated. 
#            If it is false the wtmm is not computed for this scale 
#            and the script is not executed.
#   script - Script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::wtmmg3d_scalar {args} {
    variable scale
    variable scaleId
    variable oct
    variable vox
    variable spFileName
    variable useFftw
    variable useDiskSwap
    variable nThreads

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # Get args.

    switch [llength $args] {
	4 {
	    # The second arg must be the string "cond"

	    if {[string compare [lindex $args 1] "cond"] != 0} {
		return -code error "wrong \# args: extra words after body script in \"wtmmg3d_scalar\" command"
	    }
	    set image [lindex $args 0]
	    set condition [lindex $args 2]
	    set script [lindex $args 3]
	}
	3 {
	    set image [lindex $args 0]
	    set condition [lindex $args 1]
	    set script [lindex $args 2]
	}
	2 {
	    set image [lindex $args 0]
	    set condition 1
	    set script [lindex $args 1]
	}
	1 {
	    return -code error "wrong \# args: no script after image name"
	}
	0 {
	    return -code error "wrong \# args: no image name"
	}
	default {
	    return -code error "wrong \# args: extra words after body script in \"wtmmg\" command"
	}
    }

    set theDir [pwd]


    i3Dsave $image __image_[pid] 
    ifftw3d $image -threads $nThreads
    i3Dsave $image __ft_[pid]	
    delete  $image
    

    set result {}
    scalesLoop {
	if [uplevel expr $condition] {
	    logMsg "  Octave $oct - vox $vox - scale $scale ( $scaleId )"
	    set result [WtmmgCurrentScale $theDir/__ft_[pid] -3d]
	    set modId [lindex $result 0]
	    set argId [lindex $result 1]
	    set maxId [lindex $result 2]
	    set code [catch {uplevel $script} result]
	}
    }

    catch {file delete ${theDir}/__image_[pid]}
    catch {file delete ${theDir}/__ft_[pid]}
    
    return $result
}


# imStudy::wtmmg3d_vector --
# usage : imStudy::wtmmg3d_vector str str str [["cond"] expr] script
#
#   Compute the wtmm of a 3D -> 3D vector field at all current scales using
# the gradient surfaces method. For each scale this command computes the 
# modulus and the argument for each point of the image and the contour
# surfaces.
#
# Parameters :
#   string - 1st component Image name.
#   string - 2nd component Image name.
#   string - 3rd component Image name.
#   expr   - At each scale this expression is evaluated. 
#            If it is false the wtmm is not computed for this scale 
#            and the script is not executed.
#   script - Script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::wtmmg3d_vector {args} {
    variable scale
    variable scaleId
    variable oct
    variable vox
    variable spFileName
    variable useFftw
    variable useDiskSwap
    variable nThreads

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # Get args.

    switch [llength $args] {
	6 {
	    # The fourth arg must be the string "cond"

	    if {[string compare [lindex $args 3] "cond"] != 0} {
		return -code error "wrong \# args: extra words after body script in \"wtmmg3d_vector\" command"
	    }
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set image3 [lindex $args 2]
	    set condition [lindex $args 4]
	    set script [lindex $args 5]
	}
	5 {
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set image3 [lindex $args 2]
	    set condition [lindex $args 3]
	    set script [lindex $args 4]
	}
	4 {
	    set image1 [lindex $args 0]
	    set image2 [lindex $args 1]
	    set image3 [lindex $args 2]
	    set condition 1
	    set script [lindex $args 3]
	}
	3 {
	    return -code error "wrong \# args: no script after image names"
	}
	default {
	    return -code error "wrong \# args: extra words after body script in \"wtmmg3d_vector\" command"
	}
    }

    set theDir [pwd]


    i3Dsave $image1 __image1_[pid] 
    ifftw3d $image1 -threads $nThreads
    i3Dsave $image1 __ft1_[pid]	
    delete  $image1
    
    i3Dsave $image2 __image2_[pid] 
    ifftw3d $image2 -threads $nThreads
    i3Dsave $image2 __ft2_[pid]	
    delete  $image2
    
    i3Dsave $image3 __image3_[pid] 
    ifftw3d $image3 -threads $nThreads
    i3Dsave $image3 __ft3_[pid]	
    delete  $image3
    

    set result {}
    scalesLoop {
	if [uplevel expr $condition] {
	    logMsg "  Octave $oct - vox $vox - scale $scale ( $scaleId )"
	    set result [WtmmgCurrentScale $theDir/__ft1_[pid] -vector3d $theDir/__ft2_[pid] $theDir/__ft3_[pid]]
	    set modId [lindex $result 0]
	    set argId [lindex $result 1]
	    set maxId [lindex $result 2]
	    set code [catch {uplevel $script} result]
	}
    }

    catch {file delete ${theDir}/__image1_[pid]}
    catch {file delete ${theDir}/__image2_[pid]}
    catch {file delete ${theDir}/__image3_[pid]}
    catch {file delete ${theDir}/__ft1_[pid]}
    catch {file delete ${theDir}/__ft2_[pid]}
    catch {file delete ${theDir}/__ft3_[pid]}
    
    return $result
}


# imStudy::wtmmg_partial --
# usage : imStudy::wtmmg_partial str [["cond"] expr] script
#
#   Compute the wtmm at scales defined by the lists octList and voxList
#  using the gradient lines method. For each scale this command computes
#  the modulus and the argument for each point of the image and the
#  contour lines.
#
# Parameters :
#   string - Image name.
#   expr   - At each scale this expression is evaluated. If it is false
#            the wtmm is not computed for this scale and the script is 
#            not executed.
#   script - Script to execute.
#
# Return value :
#   The result of the last command of the loop.

proc imStudy::wtmmg_partial {args} {
    variable scale
    variable scaleId
    variable oct
    variable octList
    variable vox
    variable voxList
    variable spFileName
    variable useCv2d
    variable useFftw
    variable useDiskSwap
    variable nThreads

    variable imageName

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    # Get args.

    switch [llength $args] {
	4 {
	    # The second arg must be the string "cond"

	    if {[string compare [lindex $args 1] "cond"] != 0} {
		return -code error "wrong \# args: extra words after body script in \"wtmmg\" command"
	    }
	    set image [lindex $args 0]
	    set condition [lindex $args 2]
	    set script [lindex $args 3]
	}
	3 {
	    set image [lindex $args 0]
	    set condition [lindex $args 1]
	    set script [lindex $args 2]
	}
	2 {
	    set image [lindex $args 0]
	    set condition 1
	    set script [lindex $args 1]
	}
	1 {
	    return -code error "wrong \# args: no script after image name"
	}
	0 {
	    return -code error "wrong \# args: no image name"
	}
	default {
	    return -code error "wrong \# args: extra words after body script in \"wtmmg\" command"
	}
    }

    set theDir [pwd]

    if {$useFftw == 1} {
	isave   $image __image_[pid] 
	ifftw2d $image -threads $nThreads
	isave   $image __ft_[pid]	
	delete  $image
    } elseif {$useCv2d == 0} {
	igfft   $image __ft_[pid]
	isave   $image __image_[pid]
	delete  $image
	isave   __ft_[pid]

	if {$spFileName != {}} {
	    updateSpectrum __ft_[pid]
	}

	delete  __ft_[pid]
    } else {
	set imageName $image
	if {$useDiskSwap == 1} {
	    isave   $image __image_[pid]
	    delete  $image
	    variable imagePath
	    set imagePath [pwd]/__image_[pid]
	}
    }

    set result {}
    
    foreachscale {
	if [uplevel expr $condition] {
	    logMsg "  Octave $oct - vox $vox - scale $scale ( $scaleId )"
	    set result [WtmmgCurrentScale $theDir/__ft_[pid]]
	    set modId [lindex $result 0]
	    set argId [lindex $result 1]
	    set maxId [lindex $result 2]
	    set code [catch {uplevel $script} result]
	    switch $code {
		1 {
		    if {$useCv2d == 0} {
			iload ${theDir}/__image_[pid] $image
			catch {file delete ${theDir}/__image_[pid]}
			catch {file delete ${theDir}/__ft_[pid]}
		    }
			error $result
		}
		2 {
		    if {$useCv2d == 0} {
			iload ${theDir}/__image_[pid] $image
			catch {file delete ${theDir}/__image_[pid]}
			catch {file delete ${theDir}/__ft_[pid]}
		    }
		    return $result}
		    3 {break}
		    4 {continue}
		    default {}
		}
	    }
    }

    if {$useCv2d == 0} {
	iload ${theDir}/__image_[pid] $image
	catch {file delete ${theDir}/__image_[pid]}
	catch {file delete ${theDir}/__ft_[pid]}
    }

    return $result
}


# imStudy::GetBorderSize -- PRIVATE
#

proc imStudy::GetBorderSize {} {
    variable amin
    variable noct
    variable nvox
    variable border_percent

    set no [expr $noct-1]
    set nv [expr $nvox-1]
    set scale_max [expr $amin*pow(2,$no+($nv/double($nvox)))]
    set scale_max [expr $scale_max*(6/0.86)]

    return [expr int($scale_max*$border_percent)]
}


# imStudy::GetScale -- PRIVATE
# usage : imStudy::GetScale int
#
#   Compute the value of a scale according to its id.
#
# Parameters :
#   int - The scale id. Must be between 0 and the number of scales minus one.
#         Can be the string "first" or the string "last".
#
# Return value :
#   The value of the scale.

proc imStudy::GetScale {theScaleId} {
    variable amin
    variable noct
    variable nvox

    switch $theScaleId {
	first {
	    set theScale [expr $amin*(6/0.86)]
	}
	last {
	    set no [expr $noct-1]
	    set nv [expr $nvox-1]
	    set theScale [expr $amin*pow(2,$no+($nv/double($nvox)))]
	    set theScale [expr $theScale*(6/0.86)]
	}
	default {
	    if {($theScaleId < 0) || ($theScaleId >= [expr $noct*$nvox])} {
		error "wrong scaleId"
	    }
	    set theScaleId [expr int($theScaleId)]
	    set theScale [expr $amin*pow(2,$theScaleId*1.0/$nvox)]
	    set theScale [expr $theScale*(6/0.86)]
	}
    }

    return $theScale
}


# imStudy::limitsG --
# usage : imStudy::limitsG
#
#   Compute the min and the max value of the modulus of the gradient for all
# the scales and for all the images of the study.
#
# Parameters :
#   None.
#
# Return value :
#   A list with the min and the max.

proc imStudy::limitsG {} {
    variable baseDir
    variable imIdF

    variable histFileName

    variable hist_min
    variable hist_max

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set theDir [pwd]
    set ftPath ${theDir}/__ft_[pid]

    set hist_min 1e30
    set hist_max 0

    set histDir ${baseDir}/histograms
    catch {file mkdir ${histDir}}

    set border_size [GetBorderSize]

    imagesLoop {
	logMsg "Image number $imIdF."
	iload ${baseDir}/${imIdF}/image image

	igfft   image __ft_[pid]
	delete  image
	isave   __ft_[pid] $ftPath
	delete  __ft_[pid]

	logMsg "  setting the limits of the modulus histograms :"
	logMsg "    with the upper scale..."
	set scale [GetScale last]
	ConvolOneScale $ftPath dx   $scale 0 x*exp(-x*x-y*y)
	ConvolOneScale $ftPath dy   $scale 0 y*exp(-x*x-y*y)
	gmod dx dy mod
	cutedge mod mod $border_size
	set result [im_extrema mod]
	set min_upper [lindex $result 0]
	set max_upper [lindex $result 1]

	logMsg "    and the lower scale..."
	set scale [GetScale first]
	ConvolOneScale $ftPath dx   $scale 0 x*exp(-x*x-y*y)
	ConvolOneScale $ftPath dy   $scale 0 y*exp(-x*x-y*y)
	gmod dx dy mod
	cutedge mod mod $border_size
	set result [im_extrema mod]
	set min_lower [lindex $result 0]
	set max_lower [lindex $result 1]

	if {$min_lower < $hist_min} {
	    set hist_min $min_lower
	}
	if {$min_upper < $hist_min} {
	    set hist_min $min_upper
	}
	if {$max_upper > $hist_max} {
	    set hist_max $max_upper
	}
	if {$max_lower > $hist_max} {
	    set hist_max $max_lower
	}
	logMsg ""
    }

    delete dx dy mod
    catch {file delete $ftPath} 

    logMsg "The range of modulus histogram for all images is :"
    logMsg "  from $hist_min to $hist_max."
    logMsg ""

    set hist_file_id [open [getHistFile] w]
    puts $hist_file_id "$hist_min"
    puts $hist_file_id "$hist_max"
    close $hist_file_id

    return [list $hist_min $hist_max]
}


# imStudy::histInitG --
# usage : imStudy::histInitG real real
#
#   Init the histograms for the gradient line method. Here is the list of the
# histograms : modulus (1D), argument (1D) and gradient (2D). These histograms
# are computed for the complete image, for contour lines and for the maxima on
# the contour lines.
#
# Parameters :
#   real - The min value of the modulus of the gradient.
#   real - The max value of the modulus of the gradient.
#
# Return value :
#   None.

proc imStudy::histInitG {hist_min hist_max} {
    variable pi
    variable m_pi
    variable nbox_mod
    variable nbox_arg
    variable calendos_size
    variable scaleIdF

    variable ishisto
    variable ismaxhisto

    variable baseDir
    variable histDirName

    variable imIdF

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set histDir ${baseDir}/histograms
    catch {file mkdir ${histDir}}

    # Initialisation of the global histograms at each scale.
    sinus histo_tmp 100
    shisto histo_tmp h_mod_tmp $nbox_mod -x $hist_min $hist_max
    shisto histo_tmp h_arg_tmp $nbox_arg -x $m_pi $pi
    delete histo_tmp

    sscamult h_mod_tmp 0.0 h_mod_tmp
    sscamult h_arg_tmp 0.0 h_arg_tmp

    inull calendos_tmp $calendos_size

    #if {$hist_min > 0.0} {
    #    set hist_log_min [expr log($hist_min)]
    #} else {
    #    set hist_log_min -15.0
    #}
    
    set hist_log_min [expr log($hist_min)]
    set hist_log_max [expr log($hist_max)]
    shisto histo_tmp h_log_mod_tmp $nbox_mod -x $hist_log_min $hist_log_max

    scalesLoop {
	if {$ishisto} {
	    #  Gradient histograms from all points (mod, arg, gradient vector)
	    ssave h_mod_tmp ${histDir}/h_i_mod$scaleIdF -sw
	    ssave h_arg_tmp ${histDir}/h_i_arg$scaleIdF -sw
	    isave calendos_tmp ${histDir}/h_i_cal$scaleIdF
	}

	if {$ismaxhisto} {
	    #  Gradient histograms from all points on lines (mod, arg,
	    # gradient vector)
	    ssave h_mod_tmp ${histDir}/h_l_mod$scaleIdF -sw
	    ssave h_arg_tmp ${histDir}/h_l_arg$scaleIdF -sw
	    isave calendos_tmp ${histDir}/h_l_cal$scaleIdF

	    #  Gradient histograms from all max on lines (mod, arg, gradient
	    # vector)
	    ssave h_mod_tmp ${histDir}/h_m_mod$scaleIdF -sw
	    ssave h_arg_tmp ${histDir}/h_m_arg$scaleIdF -sw
	    isave calendos_tmp ${histDir}/h_m_cal$scaleIdF
	    ssave h_log_mod_tmp ${histDir}/h_m_lmo$scaleIdF -sw
	}
    }
    delete h_mod_tmp
    delete h_arg_tmp
    delete calendos_tmp

    return
}


# imStudy::SAddAndSave --
# usage : imStudy::SAddAndSave sig str
#
#   Save a signal and eventually add it to a file. Use of the "sw" format.
#
# Parameters :
#   signal - Signal to save.
#   str    - name of the resulting file.
#
# Return value :
#   None.

proc imStudy::SAddAndSave {sig file} {
    if {[file exists $file] == 1} {
	sload  $file __SAddAndSave_tmp -sw
	sadd   __SAddAndSave_tmp $sig __SAddAndSave_tmp
	ssave  __SAddAndSave_tmp $file -sw
	delete __SAddAndSave_tmp
    } else {
	ssave $sig $file -sw
    }
    return
}


# imStudy::IAddAndSave --
# usage : imStudy::IAddAndSave sig str
#
#   Save an image and eventually add it to a file.
#
# Parameters :
#   image - Image to save.
#   str   - name of the resulting file.
#
# Return value :
#   None.

proc imStudy::IAddAndSave {sig file} {
    if {[file exists $file] == 1} {
	iload  $file __IAddAndSave_tmp
	iadd   __IAddAndSave_tmp $sig __IAddAndSave_tmp
	isave  __IAddAndSave_tmp $file
	delete __IAddAndSave_tmp
    } else {
	isave $sig $file
    }
    return
}


# imStudy::getHistDir --
# usage : imStudy::getHistDir
#
#   Get the path for the current histograms directory.
#
# Parameters :
#   None.
#
# Return value :
#   The path.

proc imStudy::getHistDir {} {
    set theNamespace [namespace current]
    set histDir [namespace inscope $theNamespace {subst $histDirName}]

    return $histDir
}


# imStudy::getHistFile --
# usage : imStudy::getHistFile
#
#   Get the path for the current histograms min/max file.
#
# Parameters :
#   None.
#
# Return value :
#   The path.

proc imStudy::getHistFile {} {
    set theNamespace [namespace current]
    set histFile [namespace inscope $theNamespace {subst $histFileName}]

    return $histFile
}


# imStudy::getLogHistFile --
# usage : imStudy::getLogHistFile
#
#   Get the path for the current histograms min/max file.
#
# Parameters :
#   None.
#
# Return value :
#   The path.

proc imStudy::getLogHistFile {} {
    set theNamespace [namespace current]
    set histFile [namespace inscope $theNamespace {subst $logHistFileName}]

    return $histFile
}


# imStudy::iHistOneScaleG --
# usage : imStudy::iHistOneScaleG str
#
#   Compute the histograms at one scale on all the points of a wavelet
# transform. Then the results are added to possibly previous histograms
# for this scale, and the they are saved on disk. The modulus and
# argument images for this scale must be in memory.
#
# Parameters :
#   str - The formatted scale id.
#
# Return value :
#   None.

proc imStudy::iHistOneScaleG {theScaleIdF} {
    variable nbox_mod
    variable nbox_arg
    variable calendos_size
    variable pi
    variable m_pi
    variable baseDir
    variable histDirName

    variable hist_max
    variable hist_min

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set theNamespace [namespace current]
    set histDir [namespace inscope $theNamespace {subst $histDirName}]

    set mod mod${theScaleIdF}
    set arg arg${theScaleIdF}

    # Get histograms parameters
    set histFile [getHistFile]

    if {[file exists $histFile]} {
	set hist_file_id [open $histFile r]
	gets $hist_file_id hist_min
	gets $hist_file_id hist_max
	close $hist_file_id
    }

    # Modulus histogram (1D).
    if {$hist_max == "none" || $hist_min == "none"} {
	ihisto $mod __tmp $nbox_mod
    } else {
	ihisto $mod __tmp $nbox_mod -x $hist_min $hist_max 
    }
    SAddAndSave __tmp ${histDir}/h_i_mod${theScaleIdF}

    # Argument histogram (1D).
    ihisto $arg __tmp $nbox_arg -x $m_pi $pi
    SAddAndSave __tmp ${histDir}/h_i_arg${theScaleIdF}

    # Gradient histogram (2D).
    if {$hist_max == "none" || $hist_min == "none"} {
	#ihisto $mod __tmp $calendos_size -grad $arg
    } else {
	#ihisto $mod __tmp $calendos_size -x -$hist_max $hist_max -grad $arg
    }
    #IAddAndSave __tmp ${histDir}/h_i_cal${theScaleIdF}

    delete __tmp

    return
}


# imStudy::lHistOneScaleG --
# usage : imStudy::lHistOneScaleG str [str]
#
#   Compute the histograms at one scale on contour lines of a 
# wavelet transform. Then the results are added to possibly previous 
# histograms for this scale, and then they are saved on disk. The 
# corresponding ext images must be in memory.
#
# Parameters :
#   str - The formatted scale id.
#   [str] - The base name of the ext images. Default is "max".
#
# Return value :
#   None.

proc imStudy::lHistOneScaleG {theScaleIdF {baseName max} {xmin -0.5} {xmax 0.5}} {
    variable nbox_mod
    variable nbox_arg
    variable nbox_gradx
    variable nbox_grady
    variable calendos_size
    variable pi
    variable m_pi
    variable baseDir
    variable histDirName

    variable hist_max
    variable hist_min

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set theNamespace [namespace current]
    set histDir [namespace inscope $theNamespace {subst $histDirName}]

    set max ${baseName}${theScaleIdF}

    # Get histograms parameters
    set histFile [getHistFile]

    if {[file exists $histFile]} {
	set hist_file_id [open $histFile r]
	gets $hist_file_id hist_min
	gets $hist_file_id hist_max
	close $hist_file_id
    }

    # Modulus histogram (1D).
    if {$hist_max == "none" || $hist_min == "none"} {
	ehisto2 $max __tmp $nbox_mod 
    } else {
	ehisto2 $max __tmp $nbox_mod -x $hist_min $hist_max 
    }
    SAddAndSave __tmp ${histDir}/h_l_mod${theScaleIdF}

    # Argument histogram (1D).
    ehisto2 $max __tmp $nbox_arg -x $m_pi $pi -arg
    SAddAndSave __tmp ${histDir}/h_l_arg${theScaleIdF}

    # Gradx histogram (1D).
    ehisto2 $max __tmp $nbox_gradx  -x $xmin $xmax -gradx
    SAddAndSave __tmp ${histDir}/h_l_gradx${theScaleIdF}

    # Grady histogram (1D).
    ehisto2 $max __tmp $nbox_grady  -x $xmin $xmax -grady
    SAddAndSave __tmp ${histDir}/h_l_grady${theScaleIdF}

            


    # Gradient histogram (2D).
    if {$hist_max == "none" || $hist_min == "none"} {
	#ehisto2 $max __tmp $calendos_size -grad
    } else {
	#ehisto2 $max __tmp $calendos_size -x -$hist_max $hist_max -grad
    }
    #IAddAndSave __tmp ${histDir}/h_l_cal${theScaleIdF}

    delete __tmp

    return
}


# imStudy::mHistOneScaleG --
# usage : imStudy::mHistOneScaleG str [str]
#
#   Compute the histograms at one scale on maxima of the contour lines of a
# wavelet transform. Then the results are added to possibly previous histograms
# for this scale, and then they are saved on disk. The corresponding ext images
# must be in memory.
#
# Parameters :
#   str   - The formatted scale id.
#   [str] - The base name of the ext images. Default is "max".
#
# Return value :
#   None.

proc imStudy::mHistOneScaleG {theScaleIdF {baseName max}} {
    variable nbox_mod
    variable nbox_arg
    variable calendos_size
    variable pi
    variable m_pi
    variable baseDir
    variable histDirName
    variable histFileName
    variable logHistFileName

    variable hist_max
    variable hist_min

    variable studyId
    if {$studyId == "none"} {
	return -code error "no current image study"
    }

    set theNamespace [namespace current]
    set histDir [namespace inscope $theNamespace {subst $histDirName}]

    set max ${baseName}${theScaleIdF}

    # Get histograms parameters
    set histFile [getHistFile]

    if {[file exists $histFile]} {
	set hist_file_id [open $histFile r]
	gets $hist_file_id hist_min
	gets $hist_file_id hist_max
	close $hist_file_id
    }

    # Modulus histogram (1D).
    if {$hist_max == "none" || $hist_min == "none"} {
	ehisto2 $max __tmp $nbox_mod -vc
    } else {
	ehisto2 $max __tmp $nbox_mod -x $hist_min $hist_max -vc
    }
    SAddAndSave __tmp ${histDir}/h_m_mod${theScaleIdF}

    # Argument histogram (1D).
    ehisto2 $max __tmp $nbox_arg -x $m_pi $pi -arg -vc
    SAddAndSave __tmp ${histDir}/h_m_arg${theScaleIdF}

    # Gradient histogram (2D).
    if {$hist_max == "none" || $hist_min == "none"} {
	ehisto2 $max __tmp $calendos_size -grad -vc
    } else {
	ehisto2 $max __tmp $calendos_size -x -$hist_max $hist_max -grad -vc
    }
    IAddAndSave __tmp ${histDir}/h_m_cal${theScaleIdF}

    # Log of the modulus histogram (1D).
    if {$hist_max == "none" || $hist_min == "none"} {
	ehisto2 $max __tmp $nbox_mod -vc  -fct log(x)
    } else {
	if {[string compare $logHistFileName "none"] == 0} {
	    if {$hist_min > 0} {
		set hist_log_min [expr log($hist_min)]
	    } else {
		return -code error "minimum for log histograms must be strictly positive"
	    }
	} else {
	    set logHistFile [getLogHistFile]

	    set hist_file_id [open $logHistFile r]
	    gets $hist_file_id hist_log_min
	    gets $hist_file_id hist_log_max
	    close $hist_file_id
	}
	set hist_log_max [expr log($hist_max)]
	ehisto2 $max __tmp $nbox_mod -x $hist_log_min $hist_log_max -vc -fct log(x)
    }
    SAddAndSave __tmp ${histDir}/h_m_lmo${theScaleIdF}

    delete __tmp

    return
}


# imStudy::convHistName --
# usage : imStudy::convHistName str
#
#   Convert histogram file names from a format to an other (old and new).
#
# Parameters :
#   string - Must be "new2old" or "old2new".
#
# Return value :
#   None.

proc imStudy::convHistName {{way new2old}} {
    #  h_mod			-> h_i_mod	: image modulus histogram.
    #  h_arg			-> h_i_arg	: image argument histogram.
    #  calendos_image		-> h_i_cal	: image gradient histogram.
    #  h_max_mod		-> h_l_mod	: contour lines modulus histogram.
    #  h_max_arg		-> h_l_arg	: contour lines argument histogram.
    #  calendos_max		-> h_l_cal	: contour lines gradient histogram.
    #  h_max_line_mod		-> h_m_mod	: maxima (on contour lines) modulus
    #						  histogram.
    #  h_max_line_arg		-> h_m_arg	: maxima (on contour lines) argument
    #						  histogram.
    #  calendos_max_line	-> h_m_cal	: maxima (on contour lines) gradient
    #					  	histogram.
    variable scaleIdF
    variable baseDir

    switch -- $way {
	new2old {
	    scalesLoop {
		file rename ${baseDir}/histograms/h_i_mod$scaleIdF \
			${baseDir}/histograms/h_mod$scaleIdF
		file rename ${baseDir}/histograms/h_i_arg$scaleIdF \
			${baseDir}/histograms/h_arg$scaleIdF
		file rename ${baseDir}/histograms/h_i_cal$scaleIdF \
			${baseDir}/histograms/calendos_image$scaleIdF

		file rename ${baseDir}/histograms/h_l_mod$scaleIdF \
			${baseDir}/histograms/h_max_mod$scaleIdF
		file rename ${baseDir}/histograms/h_l_arg$scaleIdF \
			${baseDir}/histograms/h_max_arg$scaleIdF
		file rename ${baseDir}/histograms/h_l_cal$scaleIdF \
			${baseDir}/histograms/calendos_max$scaleIdF

		file rename ${baseDir}/histograms/h_m_mod$scaleIdF \
			${baseDir}/histograms/h_max_line_mod$scaleIdF
		file rename ${baseDir}/histograms/h_m_arg$scaleIdF \
			${baseDir}/histograms/h_max_linearg$scaleIdF
		file rename ${baseDir}/histograms/h_m_cal$scaleIdF \
			${baseDir}/histograms/calendos_max_line$scaleIdF
	    }
	}   
	old2new {
	    scalesLoop {
		file rename ${baseDir}/histograms/h_mod$scaleIdF \
			${baseDir}/histograms/h_i_mod$scaleIdF
		file rename ${baseDir}/histograms/h_arg$scaleIdF \
			${baseDir}/histograms/h_i_arg$scaleIdF			
		file rename ${baseDir}/histograms/calendos_image$scaleIdF \
			${baseDir}/histograms/h_i_cal$scaleIdF

		file rename ${baseDir}/histograms/h_max_mod$scaleIdF \
			${baseDir}/histograms/h_l_mod$scaleIdF
		file rename ${baseDir}/histograms/h_max_arg$scaleIdF \
			${baseDir}/histograms/h_l_arg$scaleIdF
		file rename ${baseDir}/histograms/calendos_max$scaleIdF \
			${baseDir}/histograms/h_l_cal$scaleIdF

		file rename ${baseDir}/histograms/h_max_line_mod$scaleIdF \
			${baseDir}/histograms/h_m_mod$scaleIdF			
		file rename ${baseDir}/histograms/h_max_linearg$scaleIdF \
			${baseDir}/histograms/h_m_arg$scaleIdF			
		file rename ${baseDir}/histograms/calendos_max_line$scaleIdF \
			${baseDir}/histograms/h_m_cal$scaleIdF
	    }
	}   
    }

    return
}


# imStudy::hidisp --
# usage : imStudy::hidisp int int list [list]
#
#   Open a window that displays 1D histograms.
#
# Parameters :
#   integer - Number of columns of the window.
#   integer - Number of lines of the window.
#   list    - The list of histograms to display (i.e. h_m_mod, h_i_arg, ...)
#   list    - The list of scales to display. Default will be the first voice of
#             each octave.
#
# Return value :
#   The name of the window.

proc imStudy::hidisp {nC nL histLst {scaleIdLst ""}} {
    array set histLabel {
	h_i_mod "Module" \
	h_l_mod "Module (lines)" \
	h_m_mod "Module (max)" \
	h_i_arg "Argument" \
	h_l_arg "Argument (lines)" \
	h_m_arg "Argument (max)"
    }

    if {$histLst == "all"} {
	# We take all kind of 1D histograms.

	set histLst {h_i_mod h_i_arg h_l_mod h_l_arg h_m_mod h_m_arg}
    }

    if {$scaleIdLst == ""} {
	# We take the first voice of each octave.

	variable vox
	variable scaleId

	scalesLoop {
	    if {$vox == 0} {
		lappend scaleIdLst $scaleId
	    }
	}
    }

    set w [hdisp $nC $nL $histLst $scaleIdLst]

    set i 0
    for {set c 0} {$c < $nC} {incr c} {
	set cF [format "%.2d" $c]
	for {set l 0} {$l < $nL} {incr l} {
	    set lF [format "%.2d" $l]
	    set theHist [lindex $histLst $i]
	    set theLabel $histLabel($theHist)
	    ${w}gr$cF$lF set_label [list black $theLabel] allSigLabel
	    incr i
	    if {$i == [llength $histLst]} {
		break
	    }
	}
	if {$i == [llength $histLst]} {
	    break
	}
    }

    variable studyName
    variable wavelet
    $w setLabel "histograms - $studyName - $wavelet"
    $w switch_allgraph_flag
    $w gr set_disp_mode all
    $w gr init_disp

    return $w
}


# imStudy::hread --
# usage : imStudy::hread [int]
#
#   Read 1D histograms from files.
#
# Parameters :
#   [int] - Value of the zoom.
#
# Return value :
#   None.

proc imStudy::hread {{zoom ""}} {
    gsload h_i_mod $zoom
    gsload h_i_arg $zoom
    gsload h_l_mod $zoom
    gsload h_l_arg $zoom
    gsload h_m_mod $zoom
    gsload h_m_arg $zoom
    return
}

# Here are some commonly used scripts.

namespace eval imStudy {

    # initHistScr --
    #
    #   Init all needed histograms if parameter "ishistoinit" is set to 1. Each
    # histogram is set to 0 and save on the "histograms" directory.

    set initHistScr {
	if {$ishistoinit} {
	    if {$isgaussian} {
		logMsg "Histograms init (Gradient)."
		mylassign {hm hM} [limitsG]
		histInitG $hm $hM
	    }
	}
    }

    # computeWtmmgOneImageScr --
    #
    #   Compute the wtmm on the current image using the gradient method. For
    # each scale an ext image is saved on disk.
    #   If parameter "iscontpart" is set to 1, this script computes the
    # partition functions of the modulus for all the image.
    #   If parameter "ishisto" is set to 1, this script compute the histograms
    # (mod, arg and gradient) for all the image.

    set computeWtmmgOneImageScr {
	iload ${baseDir}/${imIdF}/image image
	wtmmg image {
	    cutedge mod$scaleIdF mod$scaleIdF $border_size
	    cutedge arg$scaleIdF arg$scaleIdF $border_size

	    # The following commented lines are not a default behavior.
	    # Uncomment tem in a new script if you need them.
	    #set border [expr $size-$border_size]
	    #rm_ext max$scaleIdF max$scaleIdF $border_size $border $border_size $border

	    esave max$scaleIdF ${baseDir}/${imIdF}/max$scaleIdF
	    if {$ishisto} {
		iHistOneScaleG $scaleIdF
	    }
	    if {$iscontpart} {
		# *** Sorry, no continuous partition functions for now... ***
	    }
	    delete mod$scaleIdF
	    delete arg$scaleIdF
	    delete max$scaleIdF
	}
    }

    # pfInitScr --
    #
    #   Initialisation of the pf that will contain partition functions on
    # contour lines and on their maxima.

    set pfInitScr {
	set pfDir ${baseDir}/partition
	if {$first_image == 0} {
	    set lpf [pf create]
	    pf init $lpf $amin $noct $nvox $q_lst $size "Gradient lines" $studyName
	    set mpf [pf create]
	    pf init $mpf $amin $noct $nvox $q_lst $size "Gradient max" $studyName
	    if {$isthetapart} {
		set num 0
		foreach {theta d_theta name} $theta_lst {
		    array set mtpfId [list $num [pf create]]
		    pf init $mtpfId($num) $amin $noct $nvox $q_lst $size \
			    "Gradient max - theta $theta dtheta $d_theta" $studyName
		    incr num
		}
	    }
	} else {
	    set lpf [pf create]
	    pf load $lpf ${pfDir}/lpf
	    set mpf [pf create]
	    pf load $mpf ${pfDir}/mpf
	    if {$isthetapart} {
		set num 0
		foreach {theta d_theta name} $theta_lst {
		    array set mtpfId [list $num [pf create]]
		    pf load $mtpfId($num) ${pfDir}/mtpf$num
		    incr num
		}
	    }
	}
    }

    # computeMaxStatGOneImageScr --
    #
    #   Compute the partition functions on contour lines and on their maxima.
    #   If "ismaxhisto" is set to 1, this script computes the histograms on
    # contour lines and on their maxima.

    set computeMaxStatGOneImageScr {
	set prevId -1
	scalesLoop {
	    set theMax max$scaleIdF
	    eload ${baseDir}/${imIdF}/max$scaleIdF $theMax
	    set border [expr $size-$border_size]
	    rm_ext max$scaleIdF max$scaleIdF $border_size $border $border_size $border
	    hsearch $theMax
	    ssm $theMax
	    
	    set box_size [expr int(log($scale)*2/log(2))]
	    if {$prevId == 0} {
		vchain max$prevIdF $theMax $box_size $similitude -first
	    } elseif {$prevId > 0} {
		vchain max$prevIdF $theMax $box_size $similitude
	    }

	    set prevId  $scaleId
	    set prevIdF $scaleIdF
	}
	if {$ismaxhisto} {
	    logMsg "Histograms on contour lines and their maxima."
	    scalesLoop {
		lHistOneScaleG $scaleIdF
		mHistOneScaleG $scaleIdF	    
	    }
	}
	set pfDir ${baseDir}/partition
	catch {file mkdir $pfDir}
	logMsg "Partition functions on contour lines."
	pf clear $lpf
	pf init $lpf $amin $noct $nvox $q_lst $size "Gradient lines" $studyName
	pf compute $lpf max
	pf save $lpf ${baseDir}/${imIdF}/lpf
	if {$imId != 0} {
	    pf load $lpf ${pfDir}/lpf
	}
	pf save $lpf ${pfDir}/lpf

	logMsg "Partition functions on contour lines maxima."
	pf clear $mpf
	pf init $mpf $amin $noct $nvox $q_lst $size "Gradient max" $studyName
	pf compute $mpf max
	pf save $mpf ${baseDir}/${imIdF}/mpf
	if {$imId != 0} {
	    pf load $mpf ${pfDir}/mpf
	}
	pf save $mpf ${pfDir}/mpf

	if {$isthetapart} {
	    logMsg "Partition functions on contour lines maxima conditionned by theta."
	    set num 0
	    foreach {theta d_theta name} $theta_lst {
		pf clear $mtpfId($num)
		pf init $mtpfId($num) $amin $noct $nvox $q_lst $size \
			"Gradient max - theta $theta dtheta $d_theta" $studyName
		pf compute $mtpfId($num) max
		pf save $mtpfId($num) ${baseDir}/${imIdF}/mtpf$num
		if {$imId != 0} {
		    pf load $mtpfId($num) ${pfDir}/mtpf$num
		}
		pf save $mtpfId($num) ${pfDir}/mtpf$num
		incr num
	    }
	}
	scalesLoop {
	    delete max$scaleIdF
	}
    }

    # pfEndScr --
    #
    #   End all the pf.

    set pfEndScr {
	pf destroy $lpf
	pf destroy $mpf
	if {$isthetapart} {
	    set num 0
	    foreach {theta d_theta name} $theta_lst {
		pf destroy $mtpfId($num)
		incr num
	    }
	}
    }

    # completeScr --
    #
    #   Calls "properly" (?!?) all the previous scripts...

    set completeScr {
	init

	set logCmd dputs

	eval $initHistScr
	
	logMsg "Computation."
	eval $pfInitScr

	if {$issave} {
	    catch {file mkdir sauve}
	}

	imagesLoop {
	    logMsg "Image $imIdF"
	    iload ${baseDir}/${imIdF}/image image
	    if {$isgaussian} {
		eval $computeWtmmgOneImageScr
	    }
	    logMsg "Contour lines and maxima statistics."
	    eval $computeMaxStatGOneImageScr

	    if {$issave == 1} {
		exec cp -r ${baseDir}/partition ${baseDir}/sauve
		variable histDirName
		set histDir [getHistDir]
		exec cp -r $histDir ${baseDir}/sauve
	    }
	}
	eval $pfEndScr
	logMsg "End."
    }

    # maxStatScr --
    #
    #   Compute stats on pre-computed ext images. For all the images.

    set maxStatScr {
	init
	set logCmd dputs

	logMsg "Computation."
	eval $pfInitScr
	imagesLoop {
	    logMsg "Image $imIdF"
	    logMsg "Contour lines and maxima statistics."
	    eval $computeMaxStatGOneImageScr
	}
	eval $pfEndScr
	logMsg "End."
    }

    # maxLinkScr --
    #
    #   Link all the ext image corresponding to the current image.

    set maxLinkScr {
	set prevId -1
	scalesLoop {
	    set theMax max$scaleIdF
	    eload ${baseDir}/${imIdF}/max$scaleIdF $theMax
	    set border [expr $size-$border_size]
	    rm_ext max$scaleIdF max$scaleIdF $border_size $border $border_size $border
	    hsearch $theMax
	    ssm $theMax
	    
	    set box_size [expr int(log($scale)*2/log(2))]
	    if {$prevId == 0} {
		vchain max$prevIdF $theMax $box_size $similitude -first
	    } elseif {$prevId > 0} {
		vchain max$prevIdF $theMax $box_size $similitude
	    }

	    set prevId  $scaleId
	    set prevIdF $scaleIdF
	}
    }

    # The following line is to avoid ugly effects when you source this file...
    return
}
