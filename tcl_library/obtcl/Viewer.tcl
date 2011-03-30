
class Viewer
Viewer inherit Widget

Viewer method init args {
    instvar display
    next
    eval $self conf_verify $args
    eval $self conf_init $args
    puts $display
}

Viewer method destroy args {
    next
}

Viewer method see args {
    instvar display

    puts $display
}

Viewer option {-display} false verify {
} init {
    set display true
} configure {
}