# led

A text editor in the terminal with lexer-based syntax highlighting for C, rust, javascript, C# and onyx.

Only alacritty is tested, but most modern terminal emulators with support for ANSI escape codes should work.

No LSP support. This is by design. Fight me.

## Config

Before launching led, copy the ```example.conf``` file to ```~/.config/led/led.conf```.

Available options are listed within the example config.

## Keybinds

These keybinds are built into the editor and cannot be changed, others can be found/added in the example config.

| Key       | Description           |
| :-------: | :-------------------: |
| CTRL+S    | Save current file     |
| CTRL+E    | Exit                  |
| CTRL+O    | Open file             |
| CTRL+\    | Execute command       |
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
| F4        | Switch header/source  |

The cursor is moved with the arrow keys.

Using CTRL+Horizontal arrows moves word-by-word.
CTRL+Vertical arrows moves up/down ```editor.vstep * <consecutive_keypresses>``` lines.

ALT+Horizontal arrows moves left/right by half of the line length.

ALT+Vertical arrows moves the cursor up half a page and centers the cursor.

As expected, moving the cursor with shift held down adds text to the selection.

Selecting text with a mouse works just like any other gui application (given that your terminal emulator supports mouse events).

## Commands
Pressing CTRL+\ brings up a command input, this is a series of single character commands that each perform one action.
Spaces or any other symbol that is not a valid command are ignored.

In the following descriptions, a ```<string>``` is any number of characters, terminated either by the end of the command, or a ```\```` character.
Strings can contain ```\``` characters, escape sequences, and can be followed by ```n```, ```r```, ```t```, ```v```, ```b```, ```\```, ```\```` or ```xHH``` (where HH is a hexadecimal byte).

Similarly a ```<block>``` is any number of commands, terminated either by the end of the command or a ```\```` character.

```j<position>``` jumps to <position>.

```s<position>``` selectes from cursor to <position>.

```c<clipboard>``` copies the current selection to clipboard ```<clipboard>```, where clipboard is a number between 0 and 15.

```p<clipboard>``` pastes the contents of clipboard <clipboard> at the cursor position, where ```<clipboard>``` is a number between 0 and 15.

```l<counter><block>``` repeat commands in <block>, <counter> times.

```w<string>``` insert <string> at cursor position.

```i<condition><true_block>[e<false_block>]``` execute <true_block> if <condition> is true, otherwise execute <false_block> if ```e``` is present.

```f<string>``` jump to next occurence of <string>.

### Positions

```f<number>``` moves forward ```<number>``` columns.

```b<number>``` moves backward ```<number>``` columns.

```u<number>``` moves up ```<number>``` lines.

```d<number>``` moves down ```<number>``` lines.


```wf``` moves forward one word.

```wb``` moves backward one word.


```t``` moves to the top of the current file.

```e``` moves to the end of the current file.

```ss``` moves to the beginning of the current selection.

```se``` moves to the end of the current selection.


```l<number>``` moves to line ```<number>```.

```c<number>``` moves to column ```<number>```.


```ls``` moves to the start of the current line.

```le``` moves to the end of the current line.


```i``` moves to the end of the leading indentation of the current line.

### Conditions

```sp```: if selection is present.

```c<clipboard>p```: if clipboard <clipboard> is present (length is not zero).

