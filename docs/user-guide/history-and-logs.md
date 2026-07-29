# Input History And Conversation Logs

Fabulor keeps input history and conversation logs separate. Input history
contains commands and messages entered into the input box. Conversation logs
contain the text displayed in a server, channel, or private-message tab.

## Input History

Saved input history is scoped to the current network and tab. Commands entered
on one network or channel do not appear when browsing the history of another.

On Windows, history is stored under:

```text
%APPDATA%\Fabulor\history\<network>\<channel>.log
```

The network's server tab uses `server.log`. Characters that cannot be used in a
Windows filename are replaced with underscores, matching conversation-log
filename handling.

History files are written when their tab closes or when Fabulor closes
normally. A file is created only after that context has new history to save.

The following preferences control saved history:

- `input_history_save`: enables saving and restoring input history.
- `input_history_max`: sets the maximum number of retained entries per
  network and tab.

These settings are available under **Settings > Preferences > Interface >
Input box**.

Use this command to clear the current network and tab's input history:

```text
/CLEAR HISTORY
```

This clears the in-memory history and removes its saved history file. Histories
belonging to other networks and tabs are not changed.

The legacy profile-level `input-history.conf` file is not imported because its
entries cannot be assigned reliably to their original networks and channels.

## Conversation Logs

When conversation logging is enabled, relative log masks are written beneath:

```text
%APPDATA%\Fabulor\logs
```

The log path and filename follow the configured `irc_logmask`. Logging can be
configured under **Settings > Preferences > Chatting > Logging**.

Use this command to clear the current server, channel, or private-message log:

```text
/CLEAR LOG
```

Fabulor closes and removes only the current context's log file. If logging is
enabled, logging then resumes for that context, so the file may be recreated
with a new logging marker and subsequent messages. Other log files are not
changed.

`/CLEAR LOG` does not provide a bulk-delete operation. Removing all archived
logs remains an explicit filesystem maintenance action.
