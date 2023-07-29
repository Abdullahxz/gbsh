# GBSH

gbsh is a custom shell implementation for Linux systems written in C. It supports various basic shell commands, as well as features like environment variables, input/output redirection, pipes, and background processing.

## Features

- Basic shell commands like `cd`, `pwd`, `ls`, `exit`, and more.
- Support for environment variables manipulation (`setenv` and `unsetenv`).
- Input/output redirection using `<` and `>` symbols.
- Pipes to execute multiple commands in sequence.

## How to Compile

To compile the gbsh shell, use the following command:

```bash
gcc gbsh.c -o gbsh
```

## How to Use

1. Run the compiled executable to start the shell:

```bash
./gbsh
```

2. The prompt will be displayed as `username@hostname:current_directory$`.

3. Enter commands as you would in a regular shell. For example:

   - Basic commands:
     - `cd directory`: Change the current directory.
     - `pwd`: Print the current working directory.
     - `ls`: List files in the current directory.
     - `clear`: Clear the terminal screen.

   - Environment variables:
     - `setenv var_name value`: Create or update an environment variable.
     - `unsetenv var_name`: Remove an environment variable.
     - `environ`: Display all environment variables.

   - Input/output redirection:
     - `command > output_file`: Redirect the output of a command to a file.
     - `command < input_file`: Redirect the input of a command from a file.

   - Pipes:
     - `command1 | command2`: Execute `command1`, and pass its output as input to `command2`.

   - Background processing:
     - Add `&` at the end of the command to run it in the background.

4. To exit the shell, simply type `exit`.

## Example Usage

```bash
./gbsh

user@hostname:/home/user$ ls
file1.txt
file2.txt
file3.txt

user@hostname:/home/user$ pwd
/home/user

user@hostname:/home/user$ setenv MY_VAR hello
A new environment variable created.

user@hostname:/home/user$ environ
USER=user
SHELL=/path/to/gbsh
MY_VAR=hello

user@hostname:/home/user$ echo "This is a test" > test.txt

user@hostname:/home/user$ cat test.txt
This is a test

user@hostname:/home/user$ ls > file_list.txt

user@hostname:/home/user$ cat file_list.txt
file1.txt
file2.txt
file3.txt
test.txt

user@hostname:/home/user$ ls | grep "file"
file1.txt
file2.txt
file3.txt

user@hostname:/home/user$ ls &   # Run ls command in the background
8345

user@hostname:/home/user$   # Shell prompt returns immediately for new commands
```

## Note

gbsh is a simple shell implementation with limited features and may not be suitable for all use cases. It is intended for educational purposes and to demonstrate the basics of linux operating system programming.

## License

This project is licensed under the MIT License. Feel free to use and modify it as per your requirements.

## Author

The gbsh shell was created by [Abdullah](https://github.com/Abdullahxz/).
