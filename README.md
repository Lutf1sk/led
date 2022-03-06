# led

A text editor in the terminal with C syntax highlighting.

Only alacritty is officially supported, but most modern terminal emulators should work well enough.

## Config
Before launching led, copy the ```example.conf``` file to ```~/.config/led/led.conf```.

Available options are listed within the example config.

## Keybinds

led is only developed for personal use, so I don't see a point in adding customizable keybinds.

| Key       | Description           |
| :-------: | :-------------------: |
| CTRL+S    | Save current file     |
| CTRL+D    | Select current line   |
| CTRL+C    | Copy selection        |
| CTRL+X    | Cut selection         |
| CTRL+V    | Paste clipboard       |
| CTRL+E    | Exit lutcreator       |
| CTRL+O    | Open file             |
| CTRL+\    | Goto line             |
| CTRL+K    | Browse open files     |
| CTRL+Q    | Close current file    |
| CTRL+F    | Local find string     |
| CTRL+/    | Comment selection     |
| ALT+P     | Move to other bracket |
| TAB       | Intent selection      |
| Shift+TAB | Unindent selection    |
| Page Up   | Move up one page      |
| Page Down | Move down one page    |
| Home      | Jump to start of line |
| End       | Jump to end of line   |
| Shift+TAB | Unindent selection    |
| F4        | Switch header/source  |

The cursor is moved with the arrow keys.

CTRL+Shift+Home/End selects and moves to the end/start of the current line.

Using CTRL+Horizontal arrows moves word-by-word.

CTRL+Vertical arrows moves up/down ```editor.vstep``` lines.

ALT+Horizontal arrows moves left/right by half of the line length.

ALT+Vertical arrows moves the cursor up half a page and centers the cursor.

As expected, moving the cursor with shift held down adds text to the selection.

Selecting text with a mouse works just like any other gui application (given that your terminal emulator supports mouse events).
