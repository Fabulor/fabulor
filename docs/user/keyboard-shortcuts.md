# Keyboard Shortcuts

This guide lists Fabulor's default Windows shortcuts and their intended
actions. Start with focus in the chat input box for the main-window shortcuts.
When focus is in a list, menu or dialog, the keys may act on that control
instead. To see your profile's bindings, open **Settings > Keyboard Shortcuts**.

## Known Issues In 1.0.9

- `Ctrl+U` can delete text to the left of the cursor. Use a disposable draft
  if you test it.
- `Alt+1` through `Alt+9` and Alt + backtick can type the corresponding
  characters into the chat input box.
- Other reported failures affect `Ctrl+Shift+T`, `Alt+A`, `Page Up`,
  `Page Down`, `Shift+Up`, `Shift+Down`, `Ctrl+Home`, `Ctrl+End`,
  `Ctrl+Shift+M`, `Ctrl+G`, `Ctrl+Shift+G`, and the chat-input completion
  bindings `Shift+Page Down`, `Shift+Tab`, `Ctrl+Tab` and `Shift+Page Up`.

If you find another shortcut that behaves differently, include your Fabulor
version, the control that had focus, the key combination, and what happened
in a [GitHub bug report](https://github.com/Fabulor/fabulor/issues/new?template=bug-report.yml).

## Tabs And Windows

| Shortcut | Action |
| --- | --- |
| `Ctrl+Page Up` / `Ctrl+Page Down` | Select the previous or next tab or tree entry. |
| `Alt+1` through `Alt+9` | Select entries 1 through 9 in switcher order. |
| Alt + backtick (<kbd>Alt</kbd> + <kbd>&#96;</kbd>) | Jump to a tab with activity, prioritising private conversations, then channel highlights, messages and other events. |
| `Alt+Left` / `Alt+Right` | Move the current tab earlier or later in its group. |
| `Ctrl+Shift+Page Up` / `Ctrl+Shift+Page Down` | Move the current network's group of tabs earlier or later. |
| `Ctrl+S` | Open the Network List. |
| `Ctrl+T` / `Ctrl+N` | Open a new server tab or a new server window. |
| `Ctrl+W` | Close the current tab or window. |
| `Ctrl+Shift+T` | Reopen the most recently closed channel tab. |
| `Ctrl+Q` | Quit Fabulor when **Enable Ctrl+Q to quit** is on under **Settings > Preferences > Chatting > General**. |
| `F7` | Show or hide the user list. |
| `Ctrl+F9` | Show or hide the menu bar. |
| `F11` | Enter or leave full-screen mode. |
| `Alt+A` | Toggle away status. |
| `F1` | Open the manual. |

## Transcript

| Shortcut | Action |
| --- | --- |
| `Page Up` / `Page Down` | Scroll up or down one page. |
| `Shift+Up` / `Shift+Down` | Scroll up or down one line. |
| `Ctrl+Home` / `Ctrl+End` | Jump to the top or bottom of the transcript. |
| `Ctrl+M` | Reset the marker line. |
| `Ctrl+Shift+M` | Jump to the marker line. |
| `Ctrl+Shift+C` | Copy the selected transcript text. |
| `Ctrl+F` | Open transcript search. |
| `Ctrl+G` / `Ctrl+Shift+G` | Find the next or previous match. |

## Chat Input

| Shortcut | Action |
| --- | --- |
| `Up` / `Down` | Recall older or newer entries from [input history](history-and-logs.md#input-history). |
| `Tab` or `Shift+Page Down` | Complete a nickname, slash command or channel name, or cycle forward through matches. |
| `Shift+Tab`, `Ctrl+Tab` or `Shift+Page Up` | Cycle backwards through completion matches. |
| `Enter` (including the numeric keypad) | Send the input or run a slash command. |
| `Ctrl+;` or `Ctrl+.` | Open the [emoji picker](main-window.md#emoji-and-flags). |
| `Shift+F10` or the context-menu key | Open the input box context menu, including spelling suggestions for the word at the cursor when available. |

Fabulor checks the last word against **Settings > Auto Replace** when you press
`Space` or `Enter`. The following shortcuts insert IRC formatting codes into
the message you're writing.

| Shortcut | Formatting |
| --- | --- |
| `Ctrl+B` | Bold |
| `Ctrl+I` | Italic |
| `Ctrl+U` | Underline |
| `Ctrl+Alt+S` | Strikethrough |
| `Ctrl+K` | Colour |
| `Ctrl+R` | Reverse foreground and background colours |
| `Ctrl+O` | Reset formatting |

## Activity List

Open **View > Activity List**. Use `Tab` or `Shift+Tab` to move between
**Highlights only** and the list. Press `Space` on **Highlights only** to toggle
the filter. With focus in the list:

| Shortcut | Action |
| --- | --- |
| `Up` / `Down` | Select an unread row. |
| `Enter` | Open the selected context. |
| `Shift+F10` or the context-menu key | Open the selected row's menu. Choose **Mark as Read** and press `Enter` to clear its unread counts in the list and the channel switcher without opening the context. |

## Change A Shortcut

1. Open **Settings > Keyboard Shortcuts**.
2. Find the action you want to change, or select **Add** to create a binding.
3. Click the **Key** cell and press the new key combination. For a new binding,
   choose an **Action** and fill in **Data1** or **Data2** if the action's help
   text calls for them.
4. Select **Save**. Use **Cancel** to discard edits before saving.

To remove a binding, select its row and choose **Delete**, then **Save**.
**Reset** restores the built-in bindings while keeping custom rows. Select
**Save** to apply the reset.

If a shortcut behaves differently, first check which control has focus, then
look for the binding in **Settings > Keyboard Shortcuts**. The emoji picker and
Activity List context menu use keys handled by their own controls.
