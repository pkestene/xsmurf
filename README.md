[![DOI](https://zenodo.org/badge/22606/pkestene/xsmurf.svg)](https://zenodo.org/badge/latestdoi/22606/pkestene/xsmurf)

# What is xsmurf ?

Xsmurf is a C/Tcl/Tk software implementation of the image processing WTMM method used to perform multifractal analysis.
WTMM stands for Wavelet-based Modulus Maxima.

See the following article about multifractal analysis.

http://www.scholarpedia.org/article/Wavelet-based_multifractal_analysis

# Before installing Xsmurf

## External libraries

### Required

Tcl/Tk (with header files), libfftw (single precision), libjpeg, libmatheval (with headers), X11 (libxi-dev, libxt-dev, ...)

   On Ubuntu/Debian like system:
```bash
   sudo apt-get install libfftw3-dev tcl8.5-dev tk8.5-dev libmatheval-dev libjpeg-dev
   sudo apt-get install libx11-dev libxi-dev libxt-dev libxmu-dev libxau-dev
```

   Notice: make sure that library fftw 3.x is installed with float enabled 
(symbol FFTW_ENABLE_FLOAT must be defined). This is OK, if you installed FFTW3 using apt-get, but if you installed it from sources, make sure to have used option '--enable-float' when configuring fftw3 sources:

```bash
./bootstrap.sh
./configure --enable-float --prefix=/some/directory --enable-shared --disable-static --enable-type-prefix
make
make install
```

### OPTIONNAL

libvtk5

vtk-tcl (tcl bindings for libvtk)

Tcllib

# XSMURF INSTALLATION

## Configure

Execute following commands:

```shell
	./configure --with-tcl=/usr/lib/tcl8.5 --with-tk=/usr/lib/tk8.5 --with-hdf5=no
```

## Build

```shell
   make
```

# Run

launch executable (launch tk console with our customized tcl interpreter)

```shell
  ./main/xsmurf
```

# Documentation

Have a look at documentation

	doc/examples contains a few commented scripts to use the tools

	doc/templates contains full template project for analyzing 2d/3d
	scalar/vector-valued data

	doc/tutorial contains a few examples that can be used as templates
	for you own 2D/3D scalar/vector field study

	doc/tutorial_tcl contains a copy of the TCL language tutorial by
	Clif Flynt (http://www.msen.com/~clif/TclTutor.html) 

	doc/tcldoc contains documentation generated by the tool tcldoc for all
	the tcl-based commands defined in scripts in the tcl_library
	sub-directory

To have more info about TkCon:
[in french]
http://wfr.tcl.tk/fichiers/pub/CoursTkCon.pdf
http://wfr.tcl.tk/282

# Tutorial

See directory doc/templates which contains example Tcl scripts to perform WTMM analysis.


