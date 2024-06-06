## My Information

Name - Parthkumar Darji
Email - pdarji@stevens.edu

## Any bugs or issues that remain in the program

There is no bugs or issues in the program

## A description of a bug or issue that you ran into along the way 

I have ran into memory leak issue and I spent 2-3 days to figure out from where this bug is comming from. With the help of prof. I finnaly figure out that I was not terminating my argv with null and that's why execv was accessing unintialized memory which was leading to memory leak issue.

## Why are exit and cd built-in commands, as opposed to external executables (like ls or echo)?

These commands are not built-in commands because minishell itself need to change it's working directory or get exit. If this commands were executed as external commands, change of directory and exit only happens on child process so our main/mini shell's directory will never change or it will never exit.