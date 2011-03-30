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

# copy ima in a new image named imafft
icopy ima imafft
# perform direct Fourier Transform
ifftw2d imafft
# perform filter in the Fourier space
icopy imafft dx
ifftwfilter dx $scale 0 y*exp(-x*x-y*y)
# perform reverse Fourier Transform  
ifftw2d dx

# the job for computing gradient along y : dy
icopy imafft dy
ifftwfilter dy $scale 0 x*exp(-x*x-y*y)
ifftw2d dy

# do the job for the second derivatives
icopy imafft dxx
ifftwfilter dxx $scale -y*y*exp(-x*x-y*y) 0
ifftw2d dxx

icopy imafft dyy
ifftwfilter dyy $scale -x*x*exp(-x*x-y*y) 0
ifftw2d dyy

icopy imafft dxy
ifftwfilter dxy $scale -x*y*exp(-x*x-y*y) 0
ifftw2d dxy

# do the job for the third derivatives
icopy imafft dxxx
ifftwfilter dxxx $scale 0 -y*y*y*exp(-x*x-y*y)
ifftw2d dxxx

icopy imafft dxxy
ifftwfilter dxxy $scale 0 -y*y*x*exp(-x*x-y*y)
ifftw2d dxxy

icopy imafft dxyy
ifftwfilter dxyy $scale 0 -y*x*x*exp(-x*x-y*y)
ifftw2d dxyy

icopy imafft dyyy
ifftwfilter dyyy $scale 0 -x*x*x*exp(-x*x-y*y)
ifftw2d dyyy

# compute mod and arg
garg dx dy arg
gmod dx dy mod

# compute kapap and kapa
gkapap kapap dx dxx dy dyy dxy dxxx dxxy dxyy dyyy
gkapa kapa dx dxx dy dyy dxy
delete dx dy dxx dxy dyy

# compute WTMM edges at scale $scale
follow kapa kapap mod arg result $scale

# display edges
eaff result

###########
# display #
###########
iaff ima -ext result
#iaff gradx 
#iaff grady 
#iaff mod
#iaff arg

# display background
#icopy ima i
# then type <Ctrl-i> b inside the edge's window