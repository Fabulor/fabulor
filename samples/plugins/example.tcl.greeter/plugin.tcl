proc onMessage {eventData} {
    return
}

proc init {} {
    array set user [zoitechat::get_user_info]
    set target "#fabulor"
    if {[info exists user(channel)] && $user(channel) ne ""} {
        set target $user(channel)
    }

    zoitechat::log "Tcl sample plugin initialised"
    zoitechat::send_message $target "Hello from the Tcl sample plugin"
    zoitechat::register_callback message onMessage
}
