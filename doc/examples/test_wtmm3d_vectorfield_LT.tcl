#
# test_wtmm3d_vectorfield_LT.tcl --
#
# This is a small script that shows
# how to use several xsmurf commands
# related to WTMM3D for vector fields
# with longitudinal / transversal information
#
# You just have to source this script
# in the xsmurf console
#
# P. Kestener (CEA, MDLS) January 2017

# set data size
set size 128
# set scale parameter
set scale 8.0

##
## Create a 3D Gaussian random vector field 
#ibro3D ima $size -h -0.7
#i3DvectorGrand i1 i2 i3 $size
itest3Dvector i1 i2 i3 $size 0.3 -sigma 16. -coef -1.0 1.0

## transform object ima into ima.vtk (file format VTK visualization tools)
puts "save data into a file using the VTK file format (for visualization with mayavi)\n"
## if you want to save data to disk, uncomment the following line
#i3Dsave ima -noheader
#i3Dvector_2_vtk i1 i2 i3 ima.vtk
#i3D_2_vtk i1 i1.vtk -binary

# resize this image (just for test purpose)
# set size1 [expr $size-$size/10]
# set size2 [expr $size-2*$size/10]
# set size3 [expr $size-3*$size/10]
# iicut3D i1 i1 0 0 0 $size1 $size2 $size3
# iicut3D i2 i2 0 0 0 $size1 $size2 $size3
# iicut3D i3 i3 0 0 0 $size1 $size2 $size3

#set size1 $size
#set size2 $size
#set size3 $size

######################################
puts "compute gradient (gradx,grady,gradz)"
puts "at scale $scale along x-axis using"
puts "a FFT-based convolution method\n"

# copy image in a new image named gradx
i3Dcopy i1 gradx1 
i3Dcopy i1 grady1 
i3Dcopy i1 gradz1 
# perform direct Fourier Transform
ifftw3d gradx1
ifftw3d grady1
ifftw3d gradz1
# perform filter in the Fourier space
ifftw3dfilter gradx1 $scale -gaussian -x
ifftw3dfilter grady1 $scale -gaussian -y
ifftw3dfilter gradz1 $scale -gaussian -z
# perform reverse Fourier Transform  
ifftw3d gradx1
ifftw3d grady1
ifftw3d gradz1

# eventually save intermediate results in a VTK file
#i3Dsave gradx1 -noheader
#i3D_2_vtk gradx1 gradx1.vtk -binary

# Do the job for the 2 other components
puts "then compute othe components..."
i3Dcopy i2 gradx2 
i3Dcopy i2 grady2 
i3Dcopy i2 gradz2

i3Dcopy i3 gradx3
i3Dcopy i3 grady3 
i3Dcopy i3 gradz3

 
ifftw3d gradx2
ifftw3d grady2
ifftw3d gradz2

ifftw3d gradx3
ifftw3d grady3
ifftw3d gradz3

ifftw3dfilter gradx2 $scale -gaussian -x
ifftw3dfilter grady2 $scale -gaussian -y
ifftw3dfilter gradz2 $scale -gaussian -z

ifftw3dfilter gradx3 $scale -gaussian -x
ifftw3dfilter grady3 $scale -gaussian -y
ifftw3dfilter gradz3 $scale -gaussian -z


ifftw3d gradx2
ifftw3d grady2
ifftw3d gradz2

ifftw3d gradx3
ifftw3d grady3
ifftw3d gradz3

#i3Dsave gradx2 -noheader
#i3D_2_vtk gradx2 gradx2.vtk -binary


#puts "save (gradx,grady,gradz) as a 3d vector field (the gradient !)\n"
#vector3D_2_vtk gradx grady gradz $size grad.vtk -border 10

# just save gradx1 / grady1 / gradz1 (that we be modified by the following call)
# icopy gradx1 gradx1_copy
# icopy grady1 grady1_copy
# icopy gradz1 gradz1_copy

#
# Compute WTMM edges at scale $scale
#
wtmm3d gradx1 grady1 gradz1 result $scale mod result2 -vector gradx2 grady2 gradz2 gradx3 grady3 gradz3 -svd_LT modL modT -svd_LT_max modLext modText

ginfo result
i3Dsave mod
i3D_2_vtk mod mod.vtk -border 10

i3D_2_vtk modL modL.vtk -border 10
i3D_2_vtk modT modT.vtk -border 10

# normally you should check that mod^2 = modL^2 + modT^2

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
