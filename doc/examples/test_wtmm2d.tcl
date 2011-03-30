#
# test_wtmm2d.tcl --
#
# This is a small script that shows
# how to use several xsmurf commands
#
# You just have to source this script
# in the xsmurf console
#
# P. Kestener (CEA, DSM/DAPNIA/SEDI)


# create a square 256x256 Brownian image
# (Hurst H=0.5) named ima
ibro ima 256

# resize this image to be 200x125
iicut ima ima 0 0 200 125

# set scale parameter
set scale 30.0


###################################
# compute gradient (gradx,grady) 
#at scale $scale along x-axis using
# a FFT-based convolution method

# copy ima in a new image named gradx
icopy ima gradx
# perform direct Fourier Transform
ifftw2d gradx
# perform filter in the Fourier space
ifftwfilter gradx $scale 0 y*exp(-x*x-y*y)
# perform reverse Fourier Transform  
ifftw2d gradx

# the job for computing grady
icopy ima grady
ifftw2d grady
ifftwfilter grady $scale 0 x*exp(-x*x-y*y)
ifftw2d grady

# compute WTMM edges at scale $scale
wtmm2d gradx grady result $scale mod arg

# display edges
eaff result

###########
# display #
###########
iaff ima -ext result
iaff gradx 
iaff grady 
iaff mod
iaff arg

# display background
#icopy ima i
# then type <Ctrl-i> b inside the edge's window