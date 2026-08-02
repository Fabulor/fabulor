# Fabulor Settings Reference

Status: source-verified draft generated from the current Windows preference
schema; installed release-candidate verification is still required.

This page lists every setting exposed by `/SET` in the Windows build of
Fabulor. It is a Fabulor-specific inventory, not a compatibility copy of an
XChat or HexChat settings list.

Use **Settings > Preferences** for ordinary changes. Preferences supplies
labels, valid ranges, related controls, and restart guidance. `/SET` remains
useful for inspection, support, scripting, and settings that have no direct
control in Preferences.

## Reading And Changing Settings

```text
/SET
/SET <name>
/SET <pattern>
/SET <name> <value>
/SET -e <text-setting>
```

Boolean settings accept `ON`, `OFF`, `YES`, `NO`, `1`, or `0`. Integer
settings require a number. Text settings accept the remaining command text;
`-e` clears a text setting. Wildcards such as `/SET gui_tray*` list matching
names.

Changes are saved to the active profile. A misspelled or retired name reports
`No such variable.` See [Commands](commands.md) for the complete `/SET`
syntax.

## Important Cautions

- Do not share unredacted `/SET` output. Identity, paths, proxy details,
  highlight words, and other personal configuration may be present.
- `net_proxy_pass` is sensitive. Change it through Preferences and do not
  display, log, or paste its value.
- Geometry, pane sizes, selected rows, and search-state settings are normally
  maintained by the interface. Manual values can create unusable layouts.
- Numeric menu settings should normally be changed through Preferences. A raw
  number outside the supported choices may be normalised or behave
  unexpectedly.
- Network-specific servers, passwords, SASL, TLS, auto-connect, and autojoin
  settings belong in **Fabulor > Network List**, not this global list.

## Away Settings

| Setting | Type | Purpose |
| --- | --- | --- |
| `away_auto_unmark` | Boolean | Automatically return from away before sending a message. |
| `away_omit_alerts` | Boolean | Suppress configured alerts while marked away. |
| `away_reason` | Text | Default reason used when marking yourself away. |
| `away_show_once` | Boolean | Show an identical away message only once. |
| `away_size_max` | Integer | Maximum channel size for detailed away colouring. |
| `away_timeout` | Integer | Interval, in seconds, between away-state checks. |
| `away_track` | Boolean | Track and colour away users in the user list. |

## Nick Completion

| Setting | Type | Purpose |
| --- | --- | --- |
| `completion_amount` | Integer | Number of matches at which completion lists candidates instead of cycling. |
| `completion_auto` | Boolean | Enable automatic nickname completion. |
| `completion_sort` | Integer | Control completion ordering, including recent-talk ordering. |
| `completion_suffix` | Text | Text appended after a completed nickname. |

## DCC And File Transfers

| Setting | Type | Purpose |
| --- | --- | --- |
| `dcc_auto_chat` | Boolean | Automatically accept incoming DCC chat offers. |
| `dcc_auto_recv` | Integer | File-offer handling: ask, choose a folder, or save automatically. |
| `dcc_auto_resume` | Boolean | Automatically resume compatible interrupted transfers. |
| `dcc_blocksize` | Integer | DCC transfer block size in bytes. |
| `dcc_completed_dir` | Text | Directory to which completed downloads are moved. |
| `dcc_dir` | Text | Default directory for received files. |
| `dcc_global_max_get_cps` | Integer | Combined download speed limit in bytes per second; `0` is unlimited. |
| `dcc_global_max_send_cps` | Integer | Combined upload speed limit in bytes per second; `0` is unlimited. |
| `dcc_ip` | Text | Address advertised for DCC when not learned from the IRC server. |
| `dcc_ip_from_server` | Boolean | Ask the IRC server for the address to advertise for DCC. |
| `dcc_max_get_cps` | Integer | Per-download speed limit in bytes per second; `0` is unlimited. |
| `dcc_max_send_cps` | Integer | Per-upload speed limit in bytes per second; `0` is unlimited. |
| `dcc_permissions` | Integer | Compatibility file-permission value for received files. |
| `dcc_port_first` | Integer | First listening port in the DCC range; `0` permits the full range. |
| `dcc_port_last` | Integer | Last listening port in the DCC range; `0` permits the full range. |
| `dcc_remove` | Boolean | Remove completed and failed transfers from the transfer window automatically. |
| `dcc_save_nick` | Boolean | Include the sender's nickname in received filenames. |
| `dcc_send_fillspaces` | Boolean | Replace spaces with underscores in offered filenames. |
| `dcc_stall_timeout` | Integer | Seconds without transfer progress before a DCC send is considered stalled. |
| `dcc_timeout` | Integer | Seconds an unaccepted DCC offer remains pending. |

## Flood Protection

| Setting | Type | Purpose |
| --- | --- | --- |
| `flood_ctcp_num` | Integer | Number of CTCP messages that trigger flood handling. |
| `flood_ctcp_time` | Integer | Time window, in seconds, used by `flood_ctcp_num`. |
| `flood_msg_num` | Integer | Number of ordinary messages that trigger flood handling. |
| `flood_msg_time` | Integer | Time window, in seconds, used by `flood_msg_num`. |

## Interface And Window Settings

These settings use the `gui_` prefix. Values describing coordinates,
dimensions, selected rows, or window state are normally maintained by Fabulor.

| Setting | Type | Purpose |
| --- | --- | --- |
| `gui_autoopen_chat` | Boolean | Open the Direct Chat window for a DCC chat. |
| `gui_autoopen_dialog` | Boolean | Open a private-message tab when one is received. |
| `gui_autoopen_recv` | Boolean | Open the transfer window for a received-file offer. |
| `gui_autoopen_send` | Boolean | Open the transfer window when sending a file. |
| `gui_chanlist_maxusers` | Integer | Maximum user count included in Channel List results. |
| `gui_chanlist_minusers` | Integer | Minimum user count included in Channel List results. |
| `gui_chanlist_width_channel` | Integer | Saved Channel List channel-column width. |
| `gui_chanlist_width_topic` | Integer | Saved Channel List topic-column width. |
| `gui_chanlist_width_users` | Integer | Saved Channel List user-count-column width. |
| `gui_compact` | Boolean | Reduce spacing in channel-switcher and user-list rows. |
| `gui_ctrlq_quit` | Boolean | Allow `Ctrl+Q` to quit Fabulor. |
| `gui_dialog_height` | Integer | Default height of separate private-message windows. |
| `gui_dialog_left` | Integer | Saved horizontal position of separate private-message windows. |
| `gui_dialog_top` | Integer | Saved vertical position of separate private-message windows. |
| `gui_dialog_width` | Integer | Default width of separate private-message windows. |
| `gui_filesize_iec` | Boolean | Display file sizes using IEC binary units. |
| `gui_focus_omitalerts` | Boolean | Suppress alerts while the relevant Fabulor window is focused. |
| `gui_hide_menu` | Boolean | Hide the main menu bar. |
| `gui_input_attr` | Boolean | Enable IRC formatting attributes in the input box. |
| `gui_input_icon` | Boolean | Show the current user-mode icon beside the input box. |
| `gui_input_nick` | Boolean | Show the current nickname beside the input box. |
| `gui_input_spell` | Boolean | Enable input-box spell checking. |
| `gui_input_style` | Boolean | Apply transcript font and colours to the input box. |
| `gui_join_dialog` | Boolean | Offer the join-channel dialog after connecting. |
| `gui_lagometer` | Integer | Lag meter presentation: off, graph, text, or both. |
| `gui_lang` | Integer | Selected packaged interface language. Use Preferences to change it. |
| `gui_manifest_plugins` | Boolean | Enable discovered manifest plugins after explicit user opt-in. |
| `gui_mode_buttons` | Boolean | Show channel mode controls. |
| `gui_mode_buttons_inline` | Boolean | Place channel mode controls beside the topic. |
| `gui_mouse_scroll_speed` | Integer | Number of transcript lines moved by a mouse-wheel step. |
| `gui_pane_divider_position` | Integer | Saved divider position in the main conversation layout. |
| `gui_pane_left_size` | Integer | Saved width of the channel-switcher pane. |
| `gui_pane_right_size` | Integer | Saved width of the user-list pane. |
| `gui_pane_right_size_min` | Integer | Minimum width allowed for the user-list pane. |
| `gui_quit_dialog` | Boolean | Ask for confirmation before quitting in configured circumstances. |
| `gui_scroll_bottom_button` | Boolean | Show the down-arrow overlay when the transcript is scrolled up. |
| `gui_search_pos` | Integer | Saved transcript-search bar position. |
| `gui_slist_fav` | Boolean | Limit the Network List to favourite networks. |
| `gui_slist_select` | Integer | Saved selected row in the Network List. |
| `gui_slist_skip` | Boolean | Skip the Network List when Fabulor starts. |
| `gui_tab_chans` | Boolean | Include channels in the channel switcher. |
| `gui_tab_closebuttons` | Boolean | Show close buttons on tab-style switcher items. |
| `gui_tab_dialogs` | Boolean | Include private-message contexts in the channel switcher. |
| `gui_tab_dots` | Boolean | Show activity indicators on switcher entries. |
| `gui_tab_icons` | Boolean | Show context icons in the channel switcher. |
| `gui_tab_layout` | Integer | Channel-switcher layout: `0` tabs or `2` tree. |
| `gui_tab_middleclose` | Boolean | Close a context by middle-clicking its switcher item. |
| `gui_tab_newtofront` | Integer | Decide when newly created contexts receive focus. |
| `gui_tab_pos` | Integer | Position of the channel switcher around the main window. |
| `gui_tab_scrollchans` | Boolean | Use mouse-wheel scrolling to change channel contexts. |
| `gui_tab_server` | Boolean | Keep a separate server tab for server messages. |
| `gui_tab_small` | Integer | Use the smaller channel-switcher text presentation. |
| `gui_tab_sort` | Boolean | Sort channel-switcher entries. |
| `gui_tab_trunc` | Integer | Maximum switcher-label length before truncation. |
| `gui_tab_utils` | Boolean | Include utility windows in the channel switcher. |
| `gui_throttlemeter` | Integer | Send-throttle meter presentation: off, graph, text, or both. |
| `gui_topicbar` | Boolean | Show the topic bar. |
| `gui_topicbar_multiline` | Boolean | Allow the topic bar to use multiple lines. |
| `gui_transparency` | Integer | Main-window opacity value. Use Preferences to keep it in range. |
| `gui_tray` | Boolean | Enable the Windows notification-area icon. |
| `gui_tray_away` | Boolean | Mark yourself away when Fabulor is hidden to the notification area. |
| `gui_tray_blink` | Boolean | Blink or change the tray icon for relevant activity. |
| `gui_tray_close` | Boolean | Send the main window to the tray when it is closed. |
| `gui_tray_minimize` | Boolean | Send the main window to the tray when it is minimized. |
| `gui_tray_quiet` | Boolean | Show Windows notifications only while Fabulor is hidden or minimized. |
| `gui_ulist_buttons` | Boolean | Show the configured buttons beneath the user list. |
| `gui_ulist_color` | Boolean | Colour user-list nicknames by IRC privilege or configured state. |
| `gui_ulist_count` | Boolean | Show the channel's user count above the user list. |
| `gui_ulist_doubleclick` | Text | Command executed when a user-list nickname is double-clicked. |
| `gui_ulist_hide` | Boolean | Hide the user list. |
| `gui_ulist_host_width` | Integer | Width reserved for displayed hostmasks. |
| `gui_ulist_icons` | Boolean | Show privilege icons in the user list. |
| `gui_ulist_nick_width` | Integer | Width reserved for user-list nicknames. |
| `gui_ulist_pos` | Integer | Position of the user list around the conversation area. |
| `gui_ulist_resizable` | Boolean | Allow the user-list width to resize with the window. |
| `gui_ulist_show_hosts` | Boolean | Show hostmasks beside nicknames in the user list. |
| `gui_ulist_sort` | Integer | User-list ordering mode. Use Preferences to select it. |
| `gui_url_mod` | Integer | Modifier-key policy for opening transcript URLs. |
| `gui_usermenu` | Boolean | Show the User menu on the main menu bar. |
| `gui_win_fullscreen` | Integer | Saved fullscreen state. |
| `gui_win_height` | Integer | Saved main-window height. |
| `gui_win_left` | Integer | Saved main-window horizontal position. |
| `gui_win_modes` | Boolean | Include channel modes in the window title. |
| `gui_win_nick` | Boolean | Include the current nickname in the window title. |
| `gui_win_save` | Boolean | Save and restore main-window geometry. |
| `gui_win_state` | Integer | Saved normal, minimized, or maximized window state. |
| `gui_win_swap` | Boolean | Swap the channel-switcher and user-list sides. |
| `gui_win_top` | Integer | Saved main-window vertical position. |
| `gui_win_ucount` | Boolean | Include the user count in the window title. |
| `gui_win_width` | Integer | Saved main-window width. |
| `gui_dark_mode` | Integer | System appearance preference used when no custom GTK4 theme overrides it. |
| `gui_gtk4_theme` | Text | Selected GTK4 desktop-theme name, or the system-default selection. |
| `gui_gtk4_variant` | Integer | Selected GTK4 light, dark, or follow-system variant. |

## Input History And Alerts

| Setting | Type | Purpose |
| --- | --- | --- |
| `input_history_max` | Integer | Maximum saved input-history entries for each network and tab. |
| `input_history_save` | Boolean | Save and restore per-network and per-tab input history. |
| `input_balloon_chans` | Boolean | Show a Windows notification for channel activity. |
| `input_balloon_hilight` | Boolean | Show a Windows notification for highlights. |
| `input_balloon_priv` | Boolean | Show a Windows notification for private messages. |
| `input_beep_chans` | Boolean | Play the configured beep or sound for channel activity. |
| `input_beep_hilight` | Boolean | Play the configured beep or sound for highlights. |
| `input_beep_priv` | Boolean | Play the configured beep or sound for private messages. |
| `input_command_char` | Text | Character that identifies input as a command; normally `/`. |
| `input_filter_beep` | Boolean | Filter incoming terminal bell characters. |
| `input_flash_chans` | Boolean | Request taskbar attention for channel activity. |
| `input_flash_hilight` | Boolean | Request taskbar attention for highlights. |
| `input_flash_priv` | Boolean | Request taskbar attention for private messages. |
| `input_perc_ascii` | Boolean | Enable percent-prefixed character-number input compatibility. |
| `input_perc_color` | Boolean | Enable percent-prefixed IRC colour input compatibility. |
| `input_tray_chans` | Boolean | Mark channel activity on the tray icon. |
| `input_tray_hilight` | Boolean | Mark highlights on the tray icon. |
| `input_tray_priv` | Boolean | Mark private messages on the tray icon. |

## IRC Behaviour And Identity

| Setting | Type | Purpose |
| --- | --- | --- |
| `irc_auto_rejoin` | Boolean | Rejoin a channel automatically after being kicked. |
| `irc_reconnect_rejoin` | Boolean | Rejoin channels after reconnecting. |
| `irc_ban_type` | Integer | Hostmask pattern used by ban and quiet actions. Use Preferences to select it. |
| `irc_conf_mode` | Boolean | Show mode changes in a condensed form. |
| `irc_extra_hilight` | Text | Additional comma-separated words that trigger highlights. |
| `irc_hide_join_part_hostmask` | Boolean | Omit hostmasks from displayed join and part events. |
| `irc_hide_nickchange` | Boolean | Hide nickname-change events. |
| `irc_hide_version` | Boolean | Hide the default CTCP VERSION reply. |
| `irc_hidehost` | Boolean | Request user mode `+x` where the network supports hidden hosts. |
| `irc_id_ntext` | Text | Text used by the failed-identification command path. |
| `irc_id_ytext` | Text | Text used by the successful-identification command path. |
| `irc_invisible` | Boolean | Request IRC user mode `+i` during connection. |
| `irc_join_delay` | Integer | Delay, in seconds, before configured autojoin begins. |
| `irc_logging` | Boolean | Enable conversation logging by default. |
| `irc_logmask` | Text | Relative directory and filename mask for conversation logs. |
| `irc_nick1` | Text | Primary global nickname. |
| `irc_nick2` | Text | Second-choice global nickname. |
| `irc_nick3` | Text | Third-choice global nickname. |
| `irc_nick_hilight` | Text | Additional nickname forms that count as mentions. |
| `irc_no_hilight` | Text | Words or patterns excluded from highlight matching. |
| `irc_notice_pos` | Integer | Routing policy for incoming notices. Use Preferences to select it. |
| `irc_part_reason` | Text | Default channel-part reason. |
| `irc_quit_reason` | Text | Default IRC quit reason. |
| `irc_raw_modes` | Boolean | Display raw mode changes instead of combining them. |
| `irc_real_name` | Text | Global IRC real-name field. Do not place a secret here. |
| `irc_servernotice` | Boolean | Show server notices. |
| `irc_skip_motd` | Boolean | Suppress normal display of the server MOTD. |
| `irc_user_name` | Text | Global IRC username. |
| `irc_wallops` | Boolean | Request receipt of WALLOPS messages where supported. |
| `irc_who_join` | Boolean | Request WHO information after joining to improve user details and ban masks. |
| `irc_whois_front` | Boolean | Bring WHOIS output to the current context. |

Server-time capabilities are negotiated automatically and therefore have no
`irc_cap_server_time` setting.

## Network And Proxy Settings

| Setting | Type | Purpose |
| --- | --- | --- |
| `net_auto_reconnect` | Boolean | Reconnect automatically after an unexpected disconnect. |
| `net_bind_host` | Text | Local address or interface name to bind for outgoing connections. |
| `net_keepalive_count` | Integer | Number of failed TCP keepalive probes before the connection is considered dead. |
| `net_keepalive_idle` | Integer | Idle seconds before TCP keepalive probing begins. |
| `net_keepalive_interval` | Integer | Seconds between TCP keepalive probes. |
| `net_lag_check` | Integer | Interval, in seconds, between IRC lag checks. |
| `net_ping_timeout` | Integer | Seconds without a valid response before the connection is considered timed out. |
| `net_proxy_auth` | Boolean | Use username/password authentication for supported proxy types. |
| `net_proxy_host` | Text | Proxy hostname or address. |
| `net_proxy_pass` | Text | **Sensitive:** proxy password. Change it through Preferences. |
| `net_proxy_port` | Integer | Proxy TCP port. |
| `net_proxy_type` | Integer | Proxy type: `0` disabled, `2` SOCKS4, `3` SOCKS5, `4` HTTP, `5` automatic. Retired value `1` becomes disabled. |
| `net_proxy_use` | Integer | Proxy scope: `0` all connections, `1` IRC only, or `2` DCC only. |
| `net_proxy_user` | Text | Proxy authentication username. |
| `net_reconnect_delay` | Integer | Seconds before an automatic reconnect attempt. |
| `net_throttle` | Boolean | Apply the IRC send-rate throttle. |

## Notify List

| Setting | Type | Purpose |
| --- | --- | --- |
| `notify_timeout` | Integer | Interval, in seconds, between notify-list checks. |
| `notify_whois_online` | Boolean | Request WHOIS information when a notify-list user comes online. |

## Timestamps

| Setting | Type | Purpose |
| --- | --- | --- |
| `stamp_log` | Boolean | Include timestamps in conversation logs. |
| `stamp_log_format` | Text | Time-format string used in conversation logs. |
| `stamp_text` | Boolean | Show timestamps in the transcript. |
| `stamp_text_format` | Text | Time-format string used in the transcript. |

## Transcript, Fonts, And Search

| Setting | Type | Purpose |
| --- | --- | --- |
| `text_autocopy_color` | Boolean | Preserve IRC colour codes when automatically copying selected text. |
| `text_autocopy_stamp` | Boolean | Include timestamps when automatically copying selected text. |
| `text_autocopy_text` | Boolean | Copy a completed transcript selection to the clipboard automatically. |
| `text_background` | Text | Background-image path used by the transcript. |
| `text_color_nicks` | Boolean | Assign different transcript colours to nicknames. |
| `text_font` | Text | Compatibility transcript font setting used outside the Windows-specific font path. |
| `text_font_alternative` | Text | Comma-separated fallback fonts for missing glyphs. |
| `text_font_main` | Text | Main transcript font used by the Windows build. |
| `text_indent` | Boolean | Right-align nicknames against the transcript separator. |
| `text_max_indent` | Integer | Maximum width reserved for nickname indentation. |
| `text_max_lines` | Integer | Maximum transcript lines retained in memory for a context. |
| `text_replay` | Boolean | Restore recent logged text when opening a context. |
| `text_search_case_match` | Boolean | Make transcript search case-sensitive. |
| `text_search_follow` | Boolean | Continue following new matching text while search is open. |
| `text_search_highlight_all` | Boolean | Highlight every transcript search match. |
| `text_search_regexp` | Boolean | Interpret transcript search text as a regular expression. |
| `text_show_marker` | Boolean | Show the marker line separating earlier and newer text. |
| `text_show_sep` | Boolean | Show the separator between nickname and message columns. |
| `text_spell_langs` | Text | Comma-separated spell-check language codes. |
| `text_stripcolor_msg` | Boolean | Remove incoming IRC formatting from newly displayed messages. |
| `text_stripcolor_replay` | Boolean | Remove IRC formatting from restored scrollback. |
| `text_stripcolor_topic` | Boolean | Remove IRC formatting from displayed topics. |
| `text_thin_sep` | Boolean | Use the thin transcript separator presentation. |
| `text_wordwrap` | Boolean | Wrap long transcript lines to the available width. |

## URL Collection

| Setting | Type | Purpose |
| --- | --- | --- |
| `url_grabber` | Boolean | Collect detected URLs in the URL Grabber window. |
| `url_grabber_limit` | Integer | Maximum number of URLs retained by the URL Grabber. |
| `url_logging` | Boolean | Save detected URLs to the profile URL log. |

## Retired Names

The following inherited settings are deliberately absent and report
`No such variable.`:

- `gui_ulist_style`;
- `identd_port` and `identd_server`;
- `irc_cap_server_time`;
- `perl_warnings`; and
- `text_transparent`.

This is not a complete list of every setting ever used by an ancestor client.
Only names in the tables above are part of Fabulor's current Windows `/SET`
schema.
