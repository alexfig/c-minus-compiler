c-minus-compiler
================
<h1>Overview</h1>
C-Minus is a extremly simplified version of C. The spec can be found [here](www.cs.dartmouth.edu/~cs57/Project/C-%20Spec.pdf).

This is a very basic C-Minus compiler with no optimization. 
To run enter the following commands:

    yacc -d cminus.y
    lex cminus.lex
    cc lex.yy.c y.tab.c emitcode.c symtable.c
    ./a.out << 'sourcecode'

The compiler will write an output.asm file which will contain nasm formated x86 assembly code which you can assemble and link using nasm and gcc. The code follows linux calling convention so it won't link on Windows or OSX.

