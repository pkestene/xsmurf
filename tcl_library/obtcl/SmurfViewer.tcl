
class SmurfViewer
SmurfViewer inherit Widget

SmurfViewer method init args {
    instvar display_widget_packed
    instvar zoom size

    set display_widget_packed false
    set zoom 1

    next

    frame $self.display -relief raised -bd 1 -height 1c

    viewer $self.viewer
    pack   $self.viewer -padx 1m -pady 1m -side left -fill both

    #bind $self.viewer <Control-c> "$self destroy"
    bind $self.viewer <d> "$self pack_display_widget"
}

SmurfViewer method pack_display_widget {} {
    instvar display_widget_packed

    if {$display_widget_packed == "true"} {
	pack forget $self.display
	set display_widget_packed false
    } else {
	pack $self.display -fill both -side left
	set display_widget_packed true
    }
}

SmurfViewer method refresh_all_widgets {} {
    instvar display_widget_packed

    pack   $self.viewer -padx 1m -pady 1m -side left -fill both
    if {$display_widget_packed == "true"} {
	pack $self.display -fill both -side left
    } else {
	pack forget $self.display
    }
}
