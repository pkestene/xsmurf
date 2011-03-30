# edit.tcl --
#
#       This file implements the Tcl code for ...
#
#   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France.
#   Written by Nicolas Decoster.
#
#  RCS : $Id: edit.tcl,v 1.4 1998/10/21 17:59:13 decoster Exp $
#

# edit --
# Edit a script and eval it.

proc edit {} {
    global pid
    global sm_editor
    set local_tmp_dir /tmp

    set file_name ~/.smurf/last_script.tcl
    catch {exec mkdir ~/.smurf}
    eval exec $sm_editor $file_name
    set file_id [open $file_name r]
    set script [read -nonewline $file_id]
    close $file_id
    uplevel $script
}


# sedit --
# Eval the last script edited with the command "edit".

proc sedit {} {
    global pid
    set local_tmp_dir /tmp

    set file_name ~/.smurf/last_script.tcl
    catch {exec mkdir ~/.smurf}
    set file_id [open $file_name r]
    set script [read -nonewline $file_id]
    close $file_id
    uplevel $script
}


# eScr --
# usage : eScr dtr
#
#   Edit a script in a file and eval it.
#
# Parameters :
#   string - The name of the file.
#
# Return value :
#   Result of the script

proc eScr {fileName} {
    global pid
    global sm_editor

    eval exec $sm_editor $fileName
    set file_id [open $fileName r]
    set script [read -nonewline $file_id]
    close $file_id
    uplevel $script
}

