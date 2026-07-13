# Fabulor-Name: Away aliases
# Fabulor-Version: 1.0.0
# Fabulor-Description: Adds all-network away and back commands

namespace eval fabulor::addons::aliases {}

proc fabulor::addons::aliases::allaway {arguments} {
    set message [string trim $arguments]
    if {$message eq ""} {
        set message "I am away."
    }

    zoitechat::command ALLSERV AWAY $message
}

proc fabulor::addons::aliases::allback {arguments} {
    zoitechat::command ALLSERV BACK
}

proc init {} {
    zoitechat::register_command ALLAWAY fabulor::addons::aliases::allaway
    zoitechat::register_command ALLBACK fabulor::addons::aliases::allback
    zoitechat::log "Away aliases initialised"
}
