#ifndef __SYMBOL_TABLE__
    #define __SYMBOL_TABLE__
    
    #define TAILLE 256
    #include "abstract-tree.h"
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    
    
    typedef enum{
        VarUndeclared,
        FonctUndeclared,
        Redeclared,
        Redefinition,
        UsuallyAFunction,
        Uninitialized,
        IncompatiblePointeurType,
        TypePointerChar,
        TypeCharPointeur,
        TypePointerInt,
        TypeIntPointeur,
        IntInChar,
        IntOrChar,
        ReadePointer,
        ReadeChar,
        PointerOperation,
        NotEnoughArgs,
        TooMuchArgs,
        ReturnNotVoid,
        ReturnVoid,
        NoReturn
    }TypeOfWorries;
    
    typedef struct Problem{
        int line;
        char name[64];
        TypeOfWorries tos;
    }Problem;
    
    typedef struct TableOfProblems{
        Problem *tab;
        int taille;
        int taille_max;
    }TableOfProblems;
    
    
    typedef enum{
        Fonct,
        Param,
        Var
    }Role;
    
    typedef struct VarFonct{
        char name[64];
        char type; /* i ou c ou v */
        int pointeur; /* 0 : non, 1 : oui */
        int num_fonct; /* numéro de la fonction auquel appartient la var
                        * 0 = var globale */
        Role role;
        int initialized;
        char adresse[15];
    }VarFonct;
    
    typedef struct SymbolTable{
        VarFonct *ST;
        int taille;
        int taille_max;
    }SymbolTable;
    
    int isInTOP(Node* node, TableOfProblems* top);
    void initTableOfProblems(TableOfProblems *top);
    void freeTableOfProblems(TableOfProblems *top);
    int printTableOfProblems(TableOfProblems *top);
    
    void initSymbolTable(SymbolTable *symbol_table);
    void freeSymbolTable(SymbolTable *symbol_table);
    void printSymbolTable(SymbolTable *symbol_table);
    
    void printLine(int line);
    int SymbolTableDontHaveMain(SymbolTable *symbol_table);
    void analyseExp(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number);
    void analyseInstr(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number);
    void parcoursTree(Node *node, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top);
    void verifFonctionDef(Node *node, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top, Node* type);
    
    
#endif
