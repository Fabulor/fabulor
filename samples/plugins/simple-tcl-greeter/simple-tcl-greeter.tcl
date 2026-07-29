# Fabulor-Name: Simple Tcl Greeter
# Fabulor-Version: 1.0.0
# Fabulor-Description: Minimal simple Tcl add-on.

proc init {} {
    array set user [fabulor::get_user_info]
    set nickname "unknown"
    if {[info exists user(nick)] && $user(nick) ne ""} {
        set nickname $user(nick)
    }

    fabulor::log "Hello, $nickname. Simple Tcl add-on ready."
}
