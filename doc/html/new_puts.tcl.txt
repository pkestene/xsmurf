#
#   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
#   Written by Nicolas Decoster and Stephane Roux.
#
#   The author may be reached (Email) at the address
#       decoster@crpp.u-bordeaux.fr
#
# New commands to display things.

# Puts the time before an output.
#
proc dputs {string} {
    catch {exec date +%H.%M.%S} msg
    puts -nonewline $msg
    puts " $string"

}

# Puts blanks before output to go with dputs.
#
proc nputs {string} {
    puts "         $string"
}
