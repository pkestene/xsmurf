####################################################
# Andre Khalil 1/03/2005                           #  
# This is a hack routine to read ".fits" file      #
# using the fitsTcl library.                       # 
#                                                  # 
# WARNING: You must use a slightly modified        #
# fitsTcl lib:                                     #
# routine 'saveImageToAscii' located in            #
# ~/src/lheasoft/src/tcltk2/fitsTcl/fitsIO.c       #
# must be changed to take into account that data   #
# might contains "NULL" values that are not        #
# understood so far by xsmurf                      #
####################################################

proc fitsload { {fits_image} {xsm_image} }  {

    puts "Reading file: $fits_image"

    # load fitsTcl shared library.
    # sources of this library can be obtained at:
    # http://heasarc.gsfc.nasa.gov/docs/software/ftools/fv/fitsTcl_home.html
    load libfitstcl.so

    # The fitsTcl User's Guide is here:
    # http://heasarc.gsfc.nasa.gov/docs/software/ftools/fv/fitsTcl212.html

    # open fits image.
    fits open $fits_image 1 ima
    
    # get image dimensions.
    set imgdim [ima info imgdim]

    # Get x and y dimensions.
    screate caca 0 0 $imgdim
    set dimtmp [sgetlst caca]
    set size_x [lindex $dimtmp 0]
    set size_y [lindex $dimtmp 1]
    puts "Size of fits file (x,y) : ($size_x,$size_y)"

    # Write a temporary file using the Xsmurf ASCII file format.

    # Check that temporary is does not exist; if not we delete it.
    set is_file [file exists imatmp.dat]
    if {$is_file == 1} {
	exec rm imatmp.dat
    }

    # Wrtite the Xsmurf Ascii file format header.
    set size [expr $size_x*$size_y]
    set fileId [open imatmp.dat w]
    puts $fileId "Ascii 1 ${size_x}x${size_y} $size"
    close $fileId

    # Use the sascii command to write data.
    ima sascii image imatmp.dat 1 1 $size_x 1 $size_y 0 0 0 " "

    # close the fits file.
    ima close

    # load the temporary file.
    iload imatmp.dat $xsm_image -ascii
    
    # delete the temporary file.
    exec rm imatmp.dat
}
