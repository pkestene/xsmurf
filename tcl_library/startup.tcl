#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#   or  decoster@info.enserb.u-bordeaux.fr
#

#
# Graphic version initialisation
#
proc xstartup {} {
    global bmp_num
    # a mettre dans un fichier a part ??
    option add *background       grey70
    option add *foreground       black
    option add *activeForeground white
    option add *activeBackground grey50
    option add *font             7x14

    smurf_start

    set bmp_num 0
    wm withdraw .
}

#
# Text version initialisation
#
proc tstartup {} {
    smurf_start
}


#
# Initialisation common part.
#
proc smurf_start {} {
    global smurf_lib macros_lib

    catch {auto_mkindex $smurf_lib *.tcl}
    catch {obtcl_mkindex ${smurf_lib}/obtcl *.tcl}
    catch {auto_mkindex $macros_lib *.tcl}
}

