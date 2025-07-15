# Mini Unix Shell & Tokenizer

This project implements a minimal Unix-style shell in C, complete with robust command parsing, tokenization, and execution capabilities. It also includes utilities for arithmetic tokenization and command line token splitting.

---

## Features

- **Custom Shell (`shell.c`)**
  - Command prompt with user input parsing
  - Support for command sequencing (`;`)
  - Piping (`|`) and input/output redirection (`<`, `>`)
  - Built-in commands: `cd`, `exit`, `help`, `source`, `prev`
  - Command history (`prev`)
  - Script execution with `source`
- **Tokenization Utilities**
  - `tokens.h`/`tokens.c`: Splits input into shell tokens, handling special symbols
  - `tokenize.c`: Standalone program that prints out shell tokens from a single line of input
  - `tokenize_expr.c`: Simple tokenizer for arithmetic expressions (numbers and operators)

---

## Usage

The [Makefile](Makefile) contains the following targets:

- `make all` - compile everything
- `make tokenize` - compile the tokenizer demo
- `make shell` - compile the shell
- `make shell-tests` - run a few tests against the shell
- `make test` - compile and run all the tests
- `make clean` - perform a minimal clean-up of the source tree


The [examples](examples/) directory contains an example tokenizer. 


### Build & Run

1. **Compile Everything:**  
   ```bash
   make all

2. **Run the Program:**  
   ```bash
   ./shell






