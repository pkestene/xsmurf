# myimg.tcl --
#
#       This file implements the Tcl code for various image format handling.
#
#   Copyright 2001 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Pierre Kestener.
#

package ifneeded Img 1.2.4 [list load [file join [pwd]/tcl_library libimg1.2.so] Img]

package require Img

package provide myimg 0.0

proc imgload {filename} {
    image create photo picture -file $filename
    set pic_wid [image width picture]
    set pic_hei [image height picture]
    
    toplevel .c
    set w [canvas .c.c -width $pic_wid -height $pic_hei]
    pack $w
    $w create image 1 1 -anchor nw -image picture -tags "myimage"
    #$w create image 1 1 -anchor nw -image pic_piece -tags "myimage"
    bind .c <Control-x><Control-d> [list destroy .c]
}


##############################################
# IDEE :
# creer une commande qui transforme une photo
# en structure image au sens de xsmurf !!!!
###############################################
