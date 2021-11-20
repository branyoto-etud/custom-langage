
#ifndef __TRAD_NASM__

    #define __TRAD_NASM__
    #include "symbol-table.h"
    #include "abstract-tree.h"
    #include <stdio.h>
    
    void writeSuiteInstr(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct);
    void writeNasm(Node* node, SymbolTable* symbol_table, FILE *out);

#endif


