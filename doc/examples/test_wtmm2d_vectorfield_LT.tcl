#
# test_wtmm2d_vectorfield_LT.tcl --
#
# This is a small script that shows
# how to use several xsmurf commands
# related to WTMM2D for vector fields
# with longitudinal / trnasversal information
#
# You just have to source this script
# in the xsmurf console
#
# P. Kestener (CEA, MDLS)

# create a square 256x256 Brownian vectorfield
# image (Hurst H=0.7). The two components are 
# ima1 and ima2
ibro2Dfield ima1 ima2 256 -h 0.7

# resize these images to be 200x125
#iicut ima1 ima1 0 0 200 125
#iicut ima2 ima2 0 0 200 125

# set scale parameter
set scale 30.0


###################################
# compute gradient (gradx1,grady1,gradx2,grady2) 
# at scale $scale along x-axis using
# a FFT-based convolution method

# copy ima1 in a new image named gradx1
icopy ima1 gradx1
icopy ima1 grady1
# perform direct Fourier Transform
ifftw2d gradx1
ifftw2d grady1
# perform filter in the Fourier space
ifftwfilter gradx1 $scale 0 y*exp(-x*x-y*y)
ifftwfilter grady1 $scale 0 x*exp(-x*x-y*y)
# perform reverse Fourier Transform  
ifftw2d gradx1
ifftw2d grady1

# the same job for the other component
icopy ima2 gradx2
icopy ima2 grady2

ifftw2d gradx2
ifftw2d grady2

ifftwfilter gradx2 $scale 0 y*exp(-x*x-y*y)
ifftwfilter grady2 $scale 0 x*exp(-x*x-y*y)

ifftw2d gradx2
ifftw2d grady2

###########
# display #
###########
# iaff ima1
# iaff ima2
# iaff gradx1
# iaff grady1
# iaff gradx2
# iaff grady2

# just save gradx1 / grady1 (that we be modified by the following call)
icopy gradx1 gradx1_copy
icopy grady1 grady1_copy

# compute WTMM edges at scale $scale
wtmm2d gradx1 grady1 result $scale mod arg -vector gradx2 grady2 -svd_LT modL modT -svd_LT_max modLext modText

# display WT edges
eaff result

# display WT edges with longitudinal information
eaff modLext

# display WT edges with longitudinal information
eaff modText


# display background
#icopy ima1 i
# then type <Ctrl-i> b inside the edge's window

#iaff ima1 -ext result
#iaff ima2 -ext result
#iaff mod
#iaff arg
