#
# test_wtmm3d.tcl --
#
# This is a small script that shows
# how to use several xsmurf commands
#
# You just have to source this script
# in the xsmurf console
#
# P. Kestener (CEA, DSM/DAPNIA/SEDI)

# set data size
set size 128
# set scale parameter
set scale 28.0

##
## Create a 3D Brownian scalar field
## (Hurst H=0.33) as an object named ima
##
#puts "\ncreating a 3D ${size}x${size}x${size} Brownian scalar field\n"
##
#ibro3D ima $size -h -0.7

## uncomment the following line if you prefer to play with the data 
## used in Fig 3.4 of P. Kestener PhD manuscript, page 89
itest3D ima $size 0.3 -posS 51 51 51 -posG 76 76 76 -sigma 16. -coef -1.0 1.0

## transform object ima into ima.vtk (file format VTK visualization tools)
puts "save data into a file using the VTK file format (for visualization with mayavi)\n"
## if you want to save data to disk, uncomment the following line
#i3Dsave ima -noheader
#i3D_2_vtk ima ima.vtk

# resize this image (just for test purpose)
set size1 [expr $size-$size/10]
set size2 [expr $size-2*$size/10]
set size3 [expr $size-3*$size/10]
iicut3D ima ima 0 0 0 $size1 $size2 $size3

#set size1 $size
#set size2 $size
#set size3 $size

######################################
puts "compute gradient (gradx,grady,gradz)"
puts "at scale $scale along x-axis using"
puts "a FFT-based convolution method\n"

# copy ima in a new image named gradx
i3Dcopy ima gradx 
# perform direct Fourier Transform
ifftw3d gradx
# perform filter in the Fourier space
ifftw3dfilter gradx $scale -gaussian -x
# perform reverse Fourier Transform  
ifftw3d gradx
i3Dsave gradx -noheader
i3D_2_vtk gradx gradx.vtk

# Do the job for computing grady and gradz
puts "then compute grady..."
i3Dcopy ima grady 
ifftw3d grady
ifftw3dfilter grady $scale -gaussian -y
ifftw3d grady
i3Dsave grady -noheader
i3D_2_vtk grady grady.vtk


puts "then compute gradz..."
i3Dcopy ima gradz 
ifftw3d gradz
ifftw3dfilter gradz $scale -gaussian -z
ifftw3d gradz
i3Dsave gradz -noheader
i3D_2_vtk gradz gradz.vtk

puts "save (gradx,grady,gradz) as a 3d vector field (the gradient !)\n"
#vector3D_2_vtk gradx grady gradz $size grad.vtk -border 10

#
# Compute WTMM edges at scale $scale
#
wtmm3d gradx grady gradz result $scale mod result2
ginfo result
i3Dsave mod
i3D_2_vtk mod mod.vtk -border 10

#
# Save for VTK visualization
#

# remove border
set b1 [expr $size1/10]
set b2 [expr $size2/10]
set b3 [expr $size3/10]
set bb1 [expr $size1-$b1]
set bb2 [expr $size2-$b2]
set bb3 [expr $size3-$b3]
ecut3Dsmall result result $b1 $b2 $b3 $bb1 $bb2 $bb3

esave3Dsmall result
ei3Dsmallsave_4_vtkpolydata result result.vtk -minmax 0 1

puts "you can now visualize the WTMM surfaces at scale $scale by using"
puts "the visu_wtmmsurfaces.tcl script. To launch it, just type the"
puts "following command line in a terminal window:"
puts " "
puts "    tclsh ./visu_wtsurfaces.tcl result.vtk"

# display edges with VTK
#eload3D_display result 16 res
#eaff res
