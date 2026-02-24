# smallshell
smallshell project (archive)

# smsh — Small Shell

A minimal Unix shell written in C. Built as a learning project to understand how shells work under the hood.

## Features

- Command execution with `execvp`
- Foreground and background process support (`&`)
- Command chaining with semicolons (`;`)
- Built-in commands: `cd`, `pwd`, `exit`
- Backslash escaping for special characters
- Comment support (`#`)

## Building

```bash
gcc smsh.c -o smsh
```

## Usage

```bash
./smsh
```

You'll get a prompt like `[smsh:/current/path]#`. Type commands as you normally would in any shell.

```
[smsh:~]# ls -la
[smsh:~]# cd /tmp ; pwd
[smsh:~]# sleep 10 &
[smsh:~]# exit
```

## Notes

- Input lines are capped at 512 characters
- `cd -` switches to the previous directory
- `cd` with no arguments goes to `$HOME`
