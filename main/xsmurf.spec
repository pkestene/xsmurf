Summary: Event driven digital circuit simulator with a tcl/tk interface.
Name: xsmurf
%define version 2.0
Version: %{version}
Release: 1
Group: Applications/Simulation
Copyright: GPL
Source: xsmurf-%{version}.tar.gz
URL: http://pierre.kestener.free.fr
Requires: tcl >= 8, tk >= 8
Packager: Pierre Kestener <CEA/DSM/DAPNIA/SEDI>

%description
Xsmurf is a graphical Application Programming Interface for image processing.
Xsmurf is devoted to multifractal image
analysis and implements the 2D/3D WTMM (Wavelet Transform Maxima Modulus)
methodology. Xsmurf provides a graphical user interface based on Tk Console
(http://tkcon.sourceforge.net/) along with a customized Tcl shell
(WTMM specific commands are interfaced to C-defined routines).
Supported image file formats include PGM, TIF, ... (8bits or 16bits).

The distribution comes with a few examples scripts and tutorials
 (see doc directory). Type help in the console to have a list of available
 commands.

Xsmurf Features:
- Tk Console GUI window.
- Image Processing.
- 2D/3D scalar/vector WTMM methodology.

%prep
%setup

%build
./configure
make

%install
make install
make install.man

%files
/usr/local/bin/xsmurf
/usr/local/lib/xsmurf-2.0
%doc /usr/local/man/man1/xsmurf.1x
%doc COPYING README doc
