# Author: Ted Sternberg, strnbrg@trhj.homeunix.net
#

set fildes [open "|./draw" w]

proc fd n {
    global fildes
    puts $fildes "fd $n"
    flush $fildes
}
proc bk n {
    global fildes
    puts $fildes "bk $n"
    flush $fildes
}

proc sfd n {
    global fildes
    puts $fildes "sfd $n"
    flush $fildes
}
proc sbk n {
    global fildes
    puts $fildes "sbk $n"
    flush $fildes
}

proc rt a {
    global fildes
    puts $fildes "rt $a"
    flush $fildes
}
proc lt a {
    global fildes
    puts $fildes "lt $a"
    flush $fildes
}

proc up a {
    global fildes
    puts $fildes "up $a"
    flush $fildes
}
proc dn a {
    global fildes
    puts $fildes "dn $a"
    flush $fildes
}

proc rl a {
    global fildes
    puts $fildes "rl $a"
    flush $fildes
}

proc ht {} {
    global fildes
    puts $fildes "ht"
    flush $fildes
}
proc st {} {
    global fildes
    puts $fildes "st"
    flush $fildes
}

proc pd {} {
    global fildes
    puts $fildes "pd"
    flush $fildes
}
proc pu {} {
    global fildes
    puts $fildes "pu"
    flush $fildes
}

proc clean {} {
    global fildes
    puts $fildes "clean"
    flush $fildes
}

proc setpencolor {r g b} {
    global fildes
    puts $fildes "setpencolor $r $g $b"
    flush $fildes
}

proc sphere {on_off} {
    global fildes
    if {$on_off==0} {
        puts $fildes "sphere 0"
    } else {
        puts $fildes "sphere 1"
    }
    flush $fildes
}

proc lighting {on_off} {
    global fildes
    if {$on_off==0} {
        puts $fildes "lighting 0"
    } else {
        puts $fildes "lighting 1"
    }
    flush $fildes
}

proc bye {} {
    global fildes
    puts $fildes "bye"
    flush $fildes
    exit
}

proc repeat {n cmd} {
    for {set i 0} {$i < $n} {incr i} {
        eval $cmd
    }
}
