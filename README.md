# Simple Shell in C++

## Overview

This project is a simple implementation of a Unix-like shell in C++. It handles basic command execution, including handling built-in commands, redirection, and pipes. The shell can execute commands, change directories, and more, providing a base for further expansion and customization.

## Features

- **Command Execution**: Executes standard Unix commands entered by the user.
- **Piping**: Supports piping between commands (`cmd1 | cmd2`).
- **Redirection**: Supports output redirection (`cmd > file`).
- **Built-in Commands**:
  - `cd <directory>`: Changes the current directory.
  - `pwd`: Prints the current directory.
  - `exit`: Exits the shell.

## Compilation and Running

To compile and run the shell, use the following commands:

```bash
g++ -o sshell main.cpp
./sshell
