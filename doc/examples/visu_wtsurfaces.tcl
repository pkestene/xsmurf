#################################
##
## This simple script allows
## for visualizing WTMM surfaces
## as computed with the xsmurf
## command namde "wtmm3d"
## 
## see the script test_wtmm3d.tcl
## for an example
##
## to launch the script:
## tclsh ./visu_wtsurfaces.tcl data_filename

#
# First we include the VTK Tcl packages which will make available
# all of the VTK commands to Tcl.
#
package require vtk
package require vtkinteraction


## for colors
package require vtktesting
# you can also source the following file
#source /usr/lib/vtk-5.0/tcl/vtktesting/colors.tcl

#puts "argc: $argc"

# parse input data
if {$argc <1 || $argc>2} {
    puts "error: you must provide at least one argument (name of data file)"
    exit
} else {
    set data [lindex $argv 0]
    if {![file exists $data]} {
	puts "error: data file does not exist !!!"
	exit
    }
}


#
# Parse filename for output screenshot
#
if {$argc >= 2} {
    set saveName [lindex $argv 2]
} else {
    set saveName toto
}

#
# Create the RenderWindow, Renderer, and RenderWindowInteractor
#
vtkRenderer ren1

vtkRenderWindow renWin
  renWin AddRenderer ren1

vtkRenderWindowInteractor iren
  iren SetRenderWindow renWin

#
# Create a hue lookup table
#
vtkLookupTable lut
# blue to red
    lut SetHueRange  0.66667 0.0
    lut SetSaturationRange 1 1
#   lut SetSaturationRange .7 .7
    lut SetValueRange 1 1
    lut SetAlphaRange 1 1
    lut SetNumberOfColors 256
    lut Build


#
# Create pipeline
#
#vtkDataSetReader reader
vtkPolyDataReader reader
#  reader SetFileName "$VTK_DATA/uGridEx.vtk"
#  reader SetFileName "${theDir}/ext${id}cpoly2.vtk"
  reader SetFileName "${data}"
  reader SetScalarsName "scalars"
  reader SetVectorsName "vectors"


set valuerange [[reader GetOutput] GetScalarRange]
set minv [lindex $valuerange 0]
set maxv [lindex $valuerange 1]
puts "data range = $minv .. $maxv"



vtkDelaunay3D del
    del SetInput [reader GetOutput]
    #del SetInput [clipper1 GetOutput]
    del BoundingTriangulationOn
    del SetTolerance 0.01
    del SetAlpha 2
    del BoundingTriangulationOff

#vtkUnstructuredGridWriter toto
#    toto SetInput [del GetOutput]
#    toto SetFileName kk.vtk
#    toto Write 


vtkDataSetTriangleFilter tris
#  tris SetInput [smoother GetOutput]
#  tris SetInput [reader GetOutput]
  tris SetInput [del GetOutput]

vtkShrinkFilter shrink
  shrink SetInput [tris GetOutput]
#  shrink SetInput [reader GetOutput]
  shrink SetShrinkFactor .8


vtkDataSetMapper mapper
#  mapper SetInput [shrink GetOutput]
#  mapper SetInput [tris GetOutput]
  mapper SetInput [del GetOutput]
#  mapper SetInput [clipper1 GetOutput]
  eval mapper SetScalarRange $minv $maxv
#  mapper SetColorModeToMapScalars 
#   mapper SetColorModeToDefault
   mapper SetLookupTable lut

#vtkPolyDataWriter dd
#  dd SetInput [clipper GetOutput]


vtkGlyph3D glyph
   glyph SetInput [reader GetOutput]
   glyph SetScaleFactor 20
   glyph SetScaleModeToScaleByVector



vtkActor actor
  actor SetMapper mapper
  [actor GetProperty] SetOpacity 1.0

# add the actor to the renderer; set the size
#

#vtkOutlineFilter outline
#    outline SetInput [reader GetOutput]
vtkPolyDataReader outline
    outline SetFileName "data_vtk/outline128.vtk"

vtkPolyDataMapper outlineMapper
    outlineMapper SetInput [outline GetOutput]
vtkActor outlineActor
    outlineActor SetMapper outlineMapper
    eval [outlineActor GetProperty] SetColor $black



ren1 AddActor actor
#ren1 AddActor actor2
ren1 AddActor outlineActor

renWin SetSize 450 450
ren1 SetBackground 1 1 1


[ren1 GetActiveCamera] SetPosition 118 -47 -43
[ren1 GetActiveCamera] SetFocalPoint 17 30 24.5
[ren1 GetActiveCamera] SetViewAngle 30
[ren1 GetActiveCamera] SetViewUp -0.37 -0.84 0.39


renWin Render

# render the image
#
#iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# prevent the tk window from showing up then start the event loop
#wm withdraw .


proc colorb {} {
    global minv maxv
    set minvalue [.ff.minv get]
    set maxvalue [.ff.maxv get]
    if {$minvalue == $maxvalue } {
	#       puts "color mapping to full range: min = $minv max= $maxv"
	#text $minv $maxv
	eval mapper SetScalarRange $minv $maxv
    } else {
	#       puts "color mapping: min = $minvalue max= $maxvalue"
	#text $minvalue $maxvalue
	eval mapper SetScalarRange $minvalue $maxvalue
    }
    renWin Render
}





# add Tk user interface

frame .f
label .f.il -text "scale factor MMTO ="
entry .f.ilv  -width 10 -relief sunken -textvariable level
button .f.ppm -text "generate ${saveName}.ppm"
button .f.s -text "stop/exit"
pack .f.il .f.ilv .f.ppm .f.s
pack .f

# define event binding

bind .f.ilv <Return> {
# puts "generate new isosurface with level = $level"
#  iso SetValue 0 $level
# render new isosurface
  glyph SetScaleFactor $level
  renWin Render
  }

#eval renWin SetFileName "${saveName}.ppm"
bind .f.ppm <Button-1> {
    puts "generate basicTk.ppm"
    renWin SetFileName "${saveName}.ppm"
    renWin SaveImageAsPPM
}

bind .f.s <Button-1> {
    # puts "stop/exit"
    exit
}

if {0==0} {

# camera aha

frame .ff
label .ff.lprop -text "Camera properties"
label .ff.lpos -text "position = "
entry .ff.epos -width 30 -relief sunken -bd 2 -textvariable pos
label .ff.lfp -text  "focal point ="
entry .ff.efp -width 30 -relief sunken -textvariable  fp
label .ff.lva -text  "view angle   = "
entry .ff.eva -width 30 -relief sunken -textvariable  va
label .ff.lvu -text  "view up   = "
entry .ff.evu -width 30 -relief sunken -textvariable  vu
button .ff.p -text "get camera properties"
button .ff.sp -text "set camera properties"

label .ff.min -text "minvalue colormap ="
entry .ff.minv -width 10 -relief sunken -textvariable minvalue
label .ff.max -text "maxvalue colormap ="
entry .ff.maxv -width 10 -relief sunken -textvariable maxvalue

button .ff.s -text "stop"
pack .ff.lprop .ff.lpos .ff.epos .ff.lfp .ff.efp .ff.lva .ff.eva .ff.lvu .ff.evu .ff.min .ff.minv .ff.max .ff.maxv .ff.p .ff.sp .ff.s
pack .ff


# event handlers ...
bind .ff.p  <Button-1> {
#  puts "cam prop .."
   set pos [[ren1 GetActiveCamera] GetPosition]
   set fp  [[ren1 GetActiveCamera] GetFocalPoint]
   set va  [[ren1 GetActiveCamera] GetViewAngle]
   set vu  [[ren1 GetActiveCamera] GetViewUp]
#   puts "set pos  $pos" 
#   puts "set fp $fp " 
#   puts "set va $va " 
#   puts "set vu $vu " 
    
  } 

bind .ff.sp  <Button-1> {
eval [ren1 GetActiveCamera] SetPosition $pos
eval [ren1 GetActiveCamera] SetFocalPoint $fp
eval [ren1 GetActiveCamera] SetViewAngle $va 
eval [ren1 GetActiveCamera] SetViewUp $vu
     renWin Render
}

bind .ff.minv <Return> {colorb}
bind .ff.maxv <Return> {colorb}


bind .f.s  <Button-1> {
  exit
  } 

}




