# RPAL Compiler and CSE Machine

## Introduction

This project implements a lexical analyzer, parser, and CSE machine for the RPAL language. The RPAL language specifications can be found in the provided documents: RPAL_Lex.pdf for lexical rules and RPAL_Grammar.pdf for grammar details.

## Requirements

- MinGW's g++ compiler (for Windows)

## Installation

### Windows (MinGW)

1. **Download MinGW**: Visit the [MinGW website](http://www.mingw.org/) and download the MinGW installer.

2. **Install MinGW**: Run the installer and follow the installation instructions. Make sure to include the g++ compiler component during installation.

3. **Set PATH**: After installation, add the MinGW `bin` directory to your system PATH. This allows you to run `g++` from the command line.

### Compiling C++ Code

g++ -std=c++17 -IOperations main.cpp -o myRpal.exe

Or can use

make

#### Running the Program

.\myRpal.exe <FileName>

#### Visualizing Abstract Syntax Tree

To visualize the abstract syntax tree,
use the -ast switch:

    .\myRpal.exe <FileName> -ast
# Rpal-Language-Intepreter
# Rpal-Language-Intepreter
