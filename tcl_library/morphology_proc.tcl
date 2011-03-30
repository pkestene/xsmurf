
# Ce fichier contient toutes les procedures developpees pour la morphologie
# mathematique

# last modified by Pierre Kestener (2000/03/14).


proc ssload {name num_lst {options ""}} {
    foreach num $num_lst {
	set frmt_num [format "%.3d" $num]
	sload ${name}${frmt_num} $options
    }
}


# gserode --
# usage: gserode image str str
#
#    computes a grayscale function-set erosion
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc gserode {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m e -i g -s g -o s -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m e -i g -s g -o s -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m e -i g -s g -o s -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m e -i g -s g -o s -k auto
    } else {
	echo enter a valid Structure Element : 3x3, 5x5, plus or auto
    }
}

# gsdilate --
# usage: gsdilate image str str
#
#    computes a grayscale function-set dilation
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc gsdilate {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m d -i g -s g -o s -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m d -i g -s g -o s -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m d -i g -s g -o s -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m d -i g -s g -o s -k auto
    } else {
	echo invalid Structure Element : 3x3, 5x5, plus or auto
    }
}

# gsopen --
# usage: gsopen image str str
#
#    computes a grayscale function-set opening
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc gsopen {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m o -i g -s g -o s -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m o -i g -s g -o s -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m o -i g -s g -o s -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m o -i g -s g -o s -k auto
    } else {
	echo invalid Structure Element : 3x3, 5x5, plus or auto
    }
}

# gsclose --
# usage: gsclose image str str
#
#    computes a grayscale function-set closing
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc gsclose {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m c -i g -s g -o s -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m c -i g -s g -o s -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m c -i g -s g -o s -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m c -i g -s g -o s -k auto
    } else {
	echo invalid Structure Element : 3x3, 5x5, plus or auto
    }
}

# tophat --
# usage: tophat image str str
#
#    computes a tophat transform (see source code in file 
#    ~/dev/xsmurf/morph2d/morph_sub)
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc tophat {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m t -i g -s g -o s -z -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m t -i g -s g -o s -z -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m t -i g -s g -o s -z -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m t -i g -s g -o s -z -k auto
    } else {
	echo invalid Structure Element : 3x3, 5x5, plus or auto
    }
}

# invtop --
# usage: invtop image str str
#
#    computes a inverted tophat transform (bothat) (see source code in file 
#    ~/dev/xsmurf/morph2d/morph_sub)
#
# Parameters:
#   image     - name of the image to treat
#   str       - name of the result (image of same size)
#   str       - name of the structure element : 3x3, 5x5, plus or auto
#
# Return value:
#   None.


proc invtop {im name_out struct_elm} {
    if { $struct_elm == "3x3"} {
	morph $im $name_out -m b -i g -s g -o s -z -k 3x3
    } elseif {$struct_elm == "5x5"} {
	morph $im $name_out -m b -i g -s g -o s -z -k 5x5
    } elseif {$struct_elm == "plus"} {
	morph $im $name_out -m b -i g -s g -o s -z -k plus
    } elseif {$struct_elm == "auto"} {
	morph $im $name_out -m b -i g -s g -o s -z -k auto
    } else {
	echo invalid Structure Element : 3x3, 5x5, plus or auto
    }
}







