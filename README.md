# lutcreator

A QTCreator-like text editor in the terminal with C syntax highlighting.

Only alacritty is officially supported, but most modern terminal emulators should work well enough.

## Config
Before launching lutcreator, copy the ```example.conf``` file to ```~/.config/led/led.conf```.

Available options are listed within the example config.

## Keybinds

lutcreator is only developed for personal use, so I don't see a point in adding customizable keybinds.

| Key       | Description           |
| :-------: | :-------------------: |
| CTRL+S    | Save current file     |
| CTRL+C    | Copy selection        |
| CTRL+X    | Cut selection         |
| CTRL+V    | Paste clipboard       |
| CTRL+E    | Exit lutcreator       |
| CTRL+O    | Open file             |
| CTRL+L    | Goto line             |
| CTRL+K    | Browse open files     |
| CTRL+Q    | Close current file    |
| CTRL+F    | Find string           |
| CTRL+P    | Comment selection     |
| TAB       | Intent selection      |
| Shift+TAB | Unindent selection    |
| PageUp    | Move up one page      |
| PageDown  | Move down one page    |
| Home      | Jump to start of line |
| End       | Jump to end of line   |
| Shift+TAB | Unindent selection    |
| F4        | Switch header/source  |

The cursor is moved with the arrow keys. 

Using CTRL+LeftArrow/RightArrow moves word-by-word.

CTRL+UpArrow/DownArrow moves up/down ```editor.vstep``` lines.

As expected, moving the cursor with shift held down adds text to the selection.

Selecting text with a mouse works just like any other gui application (given that your terminal emulator supports mouse events).