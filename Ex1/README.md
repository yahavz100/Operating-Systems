# Creating Linux shell using C
This is an implementation of Linux shell as part of Operating Systems course at Bar Ilan University.<br/>
The main goal of this exercise is to understand and work with processes, using fork and exec commands in C.

There are 2 different types of commands:
1. Built-in - commands executed by the parent process.
2. not built-in - commands executed by son process using different program code.

The user can invoke the commands in 2 different approaches:
1. Foreground - command executed by a child process, the parent will wait until the child process is finished.
2. Background - command executed by a child process, but the parent wont wait for the child process to finish.

Built-in commands implemented:
1. jobs - shows a list of running commands in the background(according to their chronological order).
2. history - shows all the commands the user invoked while using the shell(according to their chronological order).
3. cd - changes the current process working directory.
4. exit - stops the shell and exit the program.

![alt text](Extra/Capture.JPG)
