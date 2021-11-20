# Custom C language

Flex/Bison project done in L3

# Wat is dis ?

This is a student project done in collaboration with Fanny who helped me a lot to translate syntax tree into NASM.

The subject is in `Subject.pdf` but is in French so good luck to read it.

This custom compiler uses Flex to identify, Bison for the syntax and C to write compiled code (in NASM).

The recognized language is a really basic language with the same syntaxe than C.  

There's 2 types : int and char.  
This language also provide pointers of these two types, but no pointer of pointer.  
The pointers can only be dereferenced (with \*).

The allowed operations are :
 - Comparizons (<, <=, >=, >, ==, !=)
 - Basic numeric operations (+, -, /, \*, %)
 - Logical (||, &&, !)

The structures availables are :
 - if ... else if ... else (like in C)
 - while loop (but no for loop)
 - function
 - Commentary, only /* ... */ comments availables. Nested comments aren't allowed!

There's 3 default functions :
 - READE(`identifier`), that reads an integer from stardard input and stores it in `identifier`.
 - READC(`identifier`), that does the same thing for character.
 - PRINT(`operation`), that prints the result of the `operation` in the standard output.

# How to run

First of all run the makefile (`make all`)

Then you can compile a custom file named `im_a_file` by doing
`./bin/compil relative_or_absolute_path/im_a_file`  
If the file is correct, the compiled code will be in `tmp.asm`.

You can also check if everything is working by running `compile.sh`
that will check if the custom compiler is doing a great job.

# Important

This project is not maintained nor updated and has even no guarantee to work.
It was made at the end of the last semester of my third year of License and was working at this moment on the uni's computers.
