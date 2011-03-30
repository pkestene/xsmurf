# test.tst
# I avoided extension tcl so it won't show up in the tclIndex.
#
# Embryo to the real test-suite.

class I
I method init {} {
	instvar Level1
puts "I init"
	next
	set Level1 0
}
I method Level1 { n } {
	instvar Level1
	incr Level1 $n
}

class J
J method init {} {
	instvar Level2
puts "J init"
	next
	set Level2 0
}
J method Level2 { n } {
	instvar Level2
	incr Level2 $n
}

class W
W inherit I J

W method init {} {
	instvar Cnt
	next
	set Cnt 0
}
W method add { n } {
	instvar Cnt
	incr Cnt $n
}
W new tst
tst Level1 4711
tst Level2 4711

# Now redefine class and see what happens
class W

puts -nonewline stderr "Testing: Redefining class removes methods"
set tmp [W info methods]
if { "$tmp" != "" } {
	puts stderr ""
	puts stderr "Error: Redefining class does not remove methods"
	puts stderr "       `W info methods' gave: $tmp"
	puts stderr "       Should have been empty"
} else {
	puts stderr ": OK"
}
puts -nonewline stderr "Testing: Redefining class removes cached methods"
set tmp [W info cached]
if { "$tmp" != "info" } {
	puts stderr ""
	puts stderr "Error: Redefining class does not flush cache"
	puts stderr "       `W info cached' gave: $tmp"
	puts stderr "       Should have been: info"
} else {
	puts stderr ": OK"
}


W inherit I
tst Level1 1
W inherit I

puts -nonewline stderr "Testing: Redefining inheritance removes cached methods"
set tmp [W info cached]
if { "$tmp" != "info" } {
	puts stderr ""
	puts stderr "Error: Redefining inheritance does not flush cache"
	puts stderr "       `W info cached' gave: $tmp"
	puts stderr "       Should have been: info"
} else {
	puts stderr ": OK"
}


puts -nonewline stderr "Testing: Using inherited proc creates cache-proc"
tst Level1 1
set tmp [W info cached]
if { "$tmp" != "Level1 info" } {
	puts stderr ""
	puts stderr "Error: `W info cached' gave '$tmp'"
	puts stderr "       Should have been 'Level1 info'"
} else {
	puts stderr ": OK"
}


class I
puts -nonewline stderr "Testing: Redefining inherited class removes cached methods"
set tmp [W info cached]
if { "$tmp" != "info" } {
	puts stderr ""
	puts stderr "Error: `W info cached' gave '$tmp'"
	puts stderr "       Should have been 'info'"
} else {
	puts stderr ": OK"
}




