
#include "trad-nasm.h"

int etiquette = 0; /* numero de l'etiquette en nasm : .Ln avec n >= 0 */

/* pour deboguer
*/
#define debug

int search_var_symbol_table(Node* node, SymbolTable* symbol_table, int indice_fonct, int num_fonct){
    /* node pointe sur Identifieur
     * cherche l'identifieur de node dans la symbol_table
     * en fonction de indice_fonct et num_fonct dans laquelle il se trouve */
    int indice_var = -1;
    
    /* on cherche dans la fonction : Param ou Var */
    for(int i=indice_fonct; i<symbol_table->taille; i++){
        if(symbol_table->ST[i].role != Fonct 
        && symbol_table->ST[i].num_fonct == num_fonct 
        && strcmp(symbol_table->ST[i].name, node->u.identifier) == 0){
            indice_var = i;
            break;
        }
    }
    if(indice_var == -1){ /* on cherche dans les var globales */
        for(int i=0; i<symbol_table->taille; i++){
            if(symbol_table->ST[i].num_fonct == 0 && strcmp(symbol_table->ST[i].name, node->u.identifier) == 0){
                indice_var = i;
                break;
            }
        }
    }
    if(indice_var == -1){ 
        fprintf(stderr, "search_var_symbol_table : var %s not found in symbol_table\n",node->u.identifier); 
        exit(1);
    }
    return indice_var;
}


void writeIdentifieur(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Identifieur
     * trouve l'identifieur et on met la valeur dans la pile */
     
    int indice_var = search_var_symbol_table(node, symbol_table, indice_fonct, num_fonct);
    
    /* var globale */
    if(symbol_table->ST[indice_var].num_fonct == 0)
        fprintf(out, "\tpush qword [%s]\n", symbol_table->ST[indice_var].name);
    else{
        char registre[10], signe;
        int nb;
        sscanf(symbol_table->ST[indice_var].adresse, "%s %s %d", registre, &signe, &nb);
        if((symbol_table->ST[indice_var].role == Param) && (!strcmp(registre, "rdi") || !strcmp(registre, "rsi") || !strcmp(registre, "rdx") || !strcmp(registre, "rcx") || !strcmp(registre, "r8") || !strcmp(registre, "r9")))
                fprintf(out, "\tpush %s\n", registre);
        else{
            fprintf(out, "\tmov rax, [%s]\n", symbol_table->ST[indice_var].adresse);
            fprintf(out, "\tpush rax\n");
        }
    }
}


void writeAdr(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Adr (&) */
    
    Node *tmp = FIRSTCHILD(node); /* pointe sur l'id de l'identificateur de Adr */
    
    /* on cherche la var dans symbol_table */
    int indice_var = search_var_symbol_table(tmp, symbol_table, indice_fonct, num_fonct);
    
    if(symbol_table->ST[indice_var].num_fonct == 0){ /* var globale */
        if(node->kind == Pointeur)
            fprintf(out, "\tpush %s\n", tmp->u.identifier);
        else
            fprintf(out, "\tpush qword [%s]\n", tmp->u.identifier);
    }
    else{ /* var locale ou parametre */
        if(node->kind == Pointeur){
            /* si c'est un pointeur et adresse de forme rbp -/+ 8, on ne peut pas avoir directement accès à l'adresse */
            char registre[10], signe;
            int nb;
            sscanf(symbol_table->ST[indice_var].adresse, "%s %s %d", registre, &signe, &nb);
            if(!strcmp(registre, "rdi") || !strcmp(registre, "rsi") || !strcmp(registre, "rdx") || !strcmp(registre, "rcx") || !strcmp(registre, "r8") || !strcmp(registre, "r9"))
                fprintf(out, "\tpush %s\n", registre);
            else{
                fprintf(out, "\tmov rbx, %s\n", registre);
                switch(signe){
                    case '+': fprintf(out, "\tadd rbx, %d\n", nb); break;
                    case '-': fprintf(out, "\tsub rbx, %d\n", nb); break;
                }
                fprintf(out, "\tpush rbx\n");
            }
        }
        else
            fprintf(out, "\tpush qword [%s]\n", symbol_table->ST[indice_var].adresse);
    }
}


void writeNot(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Not
     * si 0 alors 1, sinon 0 */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tje .L%d\n", etiqTrue); /* == 0 */
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeMod(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Mod
     * idiv rbx : divise rdx:rax par rbx, met le quotient dans rax et le reste dans rdx */
    fprintf(out, "\tpush rdx\n");
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\txor rdx, rdx\n");
    fprintf(out, "\tidiv rbx\n");
    fprintf(out, "\tmov rax, rdx\n");
    fprintf(out, "\tpop rdx\n");
    fprintf(out, "\tpush rax\n");
}


void writeDiv(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Div
     * si le membre de gauche < 0 alors rdx=-1 sinon rdx=0
     * idiv rbx : divise rdx:rax par rbx, met le quotient dans rax et le reste dans rdx
     * sauvegarde un éventuel param rdx dans la pile */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    fprintf(out, "\tpush rdx\n");
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    
    /* calcul de la valeur de rdx */
    fprintf(out, "\tmov rdx, 0\n");
    fprintf(out, "\tcmp rdx, rax\n");
    fprintf(out, "\tjg .L%d\n", etiqTrue); /* > */
    /* ecrit le false */
    fprintf(out, "\txor rdx, rdx\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tmov rdx, -1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
    
    fprintf(out, "\tidiv rbx\n");
    fprintf(out, "\tpop rdx\n");
    fprintf(out, "\tpush rax\n");
}


void writeMult(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Mult */
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\timul rax, rbx\n");
    fprintf(out, "\tpush rax\n");
}


void writeMoins(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Moins
     *  -1 : 1 opéreandes
     * 4-1 : 2 opérandes */
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    if(THIRDCHILD(node) == NULL){ /* -1 */
        fprintf(out, "\tpop rbx\n");
        fprintf(out, "\txor rax, rax\n"); /* rax = 0 */
    }
    else{ /* 4-1 */
        writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
        fprintf(out, "\tpop rbx\n"); /* membre de droite */
        fprintf(out, "\tpop rax\n"); /* membre de gauche */
    }
    fprintf(out, "\tsub rax, rbx\n");
    fprintf(out, "\tpush rax\n");
}


void writePlus(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Plus */
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tadd rax, rbx\n");
    fprintf(out, "\tpush rax\n");
}


void writeInferieur(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Inferieur */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjng .L%d\n", etiqTrue); /* <= */
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeSuperieur(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Superieur
     * > ou == */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjg .L%d\n", etiqTrue); /* > */
    /* ecrit le false1 */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tje .L%d\n", etiqTrue); /* == */
    /* ecrit le false2 */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeStrict_Sup(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Strict_Sup */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjg .L%d\n", etiqTrue); /* > */
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeStrict_Inf(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Strict_Inf
     * <= et != */
    int etiqTrue1 = etiquette, etiqTrue2 = etiquette + 1, etiqFin = etiquette + 2;
    etiquette += 3;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjng .L%d\n", etiqTrue1); /* <= */
    /* ecrit le false1 */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai1 + verifier que ce n'est pas egal */
    fprintf(out, ".L%d:\n", etiqTrue1);
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue2); /* != */
    /* ecrit le false2 */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai2 */
    fprintf(out, ".L%d:\n", etiqTrue2);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeEgal(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Egal */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tje .L%d\n", etiqTrue);
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeInegal(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Inegal */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);  /* membre de gauche */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* membre de droite */
    fprintf(out, "\tpop rbx\n"); /* membre de droite */
    fprintf(out, "\tpop rax\n"); /* membre de gauche */
    fprintf(out, "\tcmp rax, rbx\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue);
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeWhile(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur While
     * evaluation paresseuse donc on fait etape par etape */
    int etiqTrue = etiquette, etiqFin = etiquette + 1, etiqWhile = etiquette + 2;
    etiquette += 3;
    
    fprintf(out, ".L%d:\n", etiqWhile);
    /* on traite les conditions du while */
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    /* recupère la valeur de la condition */
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue);
    /* ecrit le false */
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* interieur du while */
    fprintf(out, "\tjmp .L%d\n", etiqWhile); /* saute au debut du while */
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeReturn(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Return
     * met la valeur de retour dans rax */
    if(FIRSTCHILD(node) != NULL){
        writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
        fprintf(out, "\tpop rax\n");
    }
    fprintf(out, "\tmov rsp, rbp\n");
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
}


void writeOr(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur Or
     * évaluation paresseuse : si un fils est vrai alors on ne regarde pas les autres et on met 1 sur la pile
     * sinon on met rien sur la pile et on regarde les autres */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue);
    /* ecrit le false */
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    fprintf(out, "\tpush 1\n");
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeAnd(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur And
     * évaluation paresseuse : si un fils est faux alors on ne regarde pas les autres et on met 0 sur la pile
     * sinon on met rien sur la pile et on regarde les autres */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue);
    /* ecrit le false */
    fprintf(out, "\tpush 0\n");
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


void writeIfElse(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur If
     * on doit verifier si c'est un if/else ou juste if car le else sera ecrit avant le if
     * evaluation paresseuse donc on fait etape par etape */
    int etiqTrue = etiquette, etiqFin = etiquette + 1;
    etiquette += 2;
    
    /* on traite les conditions du if */
    writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
    /* recupère la valeur de la condition */
    fprintf(out, "\tpop rax\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tjne .L%d\n", etiqTrue);
    /* ecrit le false */
    if(node->nextSibling != NULL && node->nextSibling->kind == Else)
        writeSuiteInstr(FIRSTCHILD(node->nextSibling), symbol_table, out, indice_fonct, num_fonct); /* interieur du else */
    fprintf(out, "\tjmp .L%d\n", etiqFin);
    /* ecrit le vrai */
    fprintf(out, ".L%d:\n", etiqTrue);
    writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct); /* interieur du if */
    /* ecrit la fin */
    fprintf(out, ".L%d:\n", etiqFin);
}


int writeArguments(Node *node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct, int arg){
    /* node pointe sur un des arguments
     * on traite les arguments dans l'ordre decroissant : le dernier argument est traite en premier */
    int retour = arg;
    if(node != NULL){
        arg += 1;
        retour = writeArguments(node->nextSibling, symbol_table, out, indice_fonct, num_fonct, arg);
        writeSuiteInstr(node, symbol_table, out, indice_fonct, num_fonct);
        switch(arg){
            case 1: fprintf(out, "\tpop rdi\n"); break;
            case 2: fprintf(out, "\tpop rsi\n"); break;
            case 3: fprintf(out, "\tpop rdx\n"); break;
            case 4: fprintf(out, "\tpop rcx\n"); break;
            case 5: fprintf(out, "\tpop r8\n"); break;
            case 6: fprintf(out, "\tpop r9\n"); break;
            default: ; /* rien, on laisse sur la pile */
        }
    }
    return retour;
}


void writeCallBuilt_in(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur l'identifieur de la fonction appelee */
    fprintf(out, "\tpush rdi\n");
    fprintf(out, "\tpush rsi\n");
    fprintf(out, "\tpush r8\n");
    fprintf(out, "\tpush rbx\n");
    fprintf(out, "\tpush rcx\n");
    fprintf(out, "\tpush rdx\n");
    
    if(strcmp(node->u.identifier, "print") == 0){
        writeArguments(node->nextSibling, symbol_table, out, indice_fonct, num_fonct,0);
        /* recupere le type du print */
        Node *tmp = FIRSTCHILD(node->nextSibling);
        while(tmp->nextSibling != NULL){
            tmp = tmp->nextSibling;
        }
        switch(tmp->kind){
            case Int: fprintf(out, "\tcall printe\n"); break;
            case Char: fprintf(out, "\tcall printc\n"); break;
            default: fprintf(stderr, "writeCallBuilt_in : Can't reach this point\nKind = %d\n", tmp->kind);
        }
        fprintf(out, "\tmov rdi, 10\n");
        fprintf(out, "\tcall printc\n");
    }
    else{
        /* met l'adresse de l'identificateur dans rdi */
        int indice_var = search_var_symbol_table(node->nextSibling, symbol_table, indice_fonct, num_fonct);
        if(symbol_table->ST[indice_var].num_fonct == 0)/* var globale */
            fprintf(out, "\tmov rdi, %s\n", node->nextSibling->u.identifier);
        else{
            /* si c'est une adresse de forme rbp -/+ 8, on ne peut pas avoir directement accès à l'adresse */
            char registre[10], signe;
            int nb;
            sscanf(symbol_table->ST[indice_var].adresse, "%s %c %d", registre, &signe, &nb);
            if(!strcmp(registre, "rdi") || !strcmp(registre, "rsi") || !strcmp(registre, "rdx") || !strcmp(registre, "rcx") || !strcmp(registre, "r8") || !strcmp(registre, "r9"))
                fprintf(out, "\tmov rdi, %s\n", registre);
            else{
                fprintf(out, "\tmov rbx, %s\n", registre);
                switch(signe){
                    case '+': fprintf(out, "\tadd rbx, %d\n", nb); break;
                    case '-': fprintf(out, "\tsub rbx, %d\n", nb); break;
                }
                fprintf(out, "\tmov rdi, rbx\n");
            }
        }
        fprintf(out, "\tcall %s\n", node->u.identifier);
    }
    
    fprintf(out, "\tpop rdx\n");
    fprintf(out, "\tpop rcx\n");
    fprintf(out, "\tpop rbx\n");
    fprintf(out, "\tpop r8\n");
    fprintf(out, "\tpop rsi\n");
    fprintf(out, "\tpop rdi\n");
}


void writeCallFonction(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur l'identifieur de la fonction appelee
     * on gere les parametres, ensuite on appelle la fonction
     * si non void, valeur de retour dans rax (puis on push sur la pile)  */
    int nb_arg = 0;
    fprintf(out, "\tpush rdi\n");
    fprintf(out, "\tpush rsi\n");
    fprintf(out, "\tpush rdx\n");
    fprintf(out, "\tpush rcx\n");
    fprintf(out, "\tpush r8\n");
    fprintf(out, "\tpush r9\n");
    
    if(node->nextSibling != NULL)
        nb_arg = writeArguments(FIRSTCHILD(node->nextSibling), symbol_table, out, indice_fonct, num_fonct, nb_arg);
    fprintf(out, "\tcall %s\n", node->u.identifier);
    /* à partir du 7eme arg, il faut depiler les arguments mis sur la pile */
    while (nb_arg>=7) {
        fprintf(out, "\tpop rbx\n");
        nb_arg -= 1;
    }
    fprintf(out, "\tpop r9\n");
    fprintf(out, "\tpop r8\n");
    fprintf(out, "\tpop rcx\n");
    fprintf(out, "\tpop rdx\n");
    fprintf(out, "\tpop rsi\n");
    fprintf(out, "\tpop rdi\n");
    
    if(symbol_table->ST[indice_fonct].type != 'v')
        fprintf(out, "\tpush rax\n");
}


void writeAssign(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* node pointe sur la LValue */
    
    Node *tmp; /* pointe sur l'id de la var */
    if(node->kind == Pointeur)
        tmp = FIRSTCHILD(node);
    else
        tmp = node;
    
    /* on cherche la var dans symbol_table */
    int indice_var = search_var_symbol_table(tmp, symbol_table, indice_fonct, num_fonct);
    
    fprintf(out, "\tpop rax\n");
    if(symbol_table->ST[indice_var].num_fonct == 0){ /* var globale */
        if(node->kind == Pointeur && symbol_table->ST[indice_var].pointeur == 0) /**/
            fprintf(out, "\tmov %s, rax\n", tmp->u.identifier);
        else
            fprintf(out, "\tmov [%s], rax\n", tmp->u.identifier);
    }
    else{ /* var locale ou parametre */
        if(node->kind == Pointeur && symbol_table->ST[indice_var].pointeur == 0){/**/
            /* si c'est un pointeur et adresse de forme rbp -/+ 8, on ne peut pas avoir directement accès à l'adresse */
            char registre[10], signe;
            int nb;
            sscanf(symbol_table->ST[indice_var].adresse, "%s %c %d", registre, &signe, &nb);
            if(!strcmp(registre, "rdi") || !strcmp(registre, "rsi") || !strcmp(registre, "rdx") || !strcmp(registre, "rcx") || !strcmp(registre, "r8") || !strcmp(registre, "r9"))
                fprintf(out, "\tmov %s, rax\n", registre);
            else{
                fprintf(out, "\tmov rbx, %s\n", registre);
                switch(signe){
                    case '+': fprintf(out, "\tadd rbx, %d\n", nb); break;
                    case '-': fprintf(out, "\tsub rbx, %d\n", nb); break;
                }
                fprintf(out, "\tmov rbx, rax\n");
            }
        }
        else
            fprintf(out, "\tmov [%s], rax\n", symbol_table->ST[indice_var].adresse);
    }
}


void writeSuiteInstr(Node* node, SymbolTable* symbol_table, FILE *out, int indice_fonct, int num_fonct){
    /* les resultats de tous calculs se mettent sur la pile
     * si on utilise rax, on doit le vider juste après */
    
    if(node != NULL){
        switch(node->kind){
            case SuiteInstr:
                writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Assign: /* 2 enfants */
                #ifndef debug 
                fprintf(out, "Assign\n");
                #endif
                writeSuiteInstr(SECONDCHILD(node), symbol_table, out, indice_fonct, num_fonct);
                writeAssign(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Fonction:
                #ifndef debug 
                fprintf(out, "Fonction\n");
                #endif
                writeCallFonction(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
                /* cas où : if(fonct()) si on va voir nextSibling, on regarde l'intérieur du if */
                if(node->nextSibling != NULL && node->nextSibling->kind != SuiteInstr) 
                    writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Built_in:
                #ifndef debug 
                fprintf(out, "Built_in\n");
                #endif
                writeCallBuilt_in(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case If:
                #ifndef debug 
                fprintf(out, "If\n");
                #endif
                writeIfElse(node, symbol_table, out, indice_fonct, num_fonct);
                if(node->nextSibling != NULL && node->nextSibling->kind == Else)
                    writeSuiteInstr(node->nextSibling->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                else
                    writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case While:
                #ifndef debug 
                fprintf(out, "While\n");
                #endif
                writeWhile(node, symbol_table, out, indice_fonct, num_fonct);
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Return:
                #ifndef debug 
                fprintf(out, "Return\n");
                #endif
                writeReturn(node, symbol_table, out, indice_fonct, num_fonct);
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break;
            case And:
                #ifndef debug 
                fprintf(out, "And\n");
                #endif
                writeAnd(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Or:
                #ifndef debug 
                fprintf(out, "Or\n");
                #endif
                writeOr(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Egal:
                #ifndef debug 
                fprintf(out, "Egal\n");
                #endif
                writeEgal(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Inegal:
                #ifndef debug 
                fprintf(out, "Inegal\n");
                #endif
                writeInegal(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Strict_Sup:
                #ifndef debug 
                fprintf(out, "Strict_Sup\n");
                #endif
                writeStrict_Sup(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Strict_Inf:
                #ifndef debug 
                fprintf(out, "Strict_Inf\n");
                #endif
                writeStrict_Inf(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Superieur:
                #ifndef debug 
                fprintf(out, "Superieur\n");
                #endif
                writeSuperieur(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Inferieur:
                #ifndef debug 
                fprintf(out, "Inferieur\n");
                #endif
                writeInferieur(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Plus:
                #ifndef debug 
                fprintf(out, "Plus\n");
                #endif
                writePlus(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Moins:
                #ifndef debug 
                fprintf(out, "Moins\n");
                #endif
                writeMoins(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Mult:
                #ifndef debug 
                fprintf(out, "Mult\n");
                #endif
                writeMult(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Div:
                #ifndef debug 
                fprintf(out, "Div\n");
                #endif
                writeDiv(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Mod:
                #ifndef debug 
                fprintf(out, "Mod\n");
                #endif
                writeMod(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Not:
                #ifndef debug 
                fprintf(out, "Not\n");
                #endif
                writeNot(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Adr:
                #ifndef debug 
                fprintf(out, "Adr\n");
                #endif
                writeAdr(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case IntLitteral:
                #ifndef debug 
                fprintf(out, "IntLitteral\n");
                #endif
                fprintf(out, "\tmov rax, %d\n", node->u.integer);
                fprintf(out, "\tpush rax\n");
                break;
            case CharLitteral:
                #ifndef debug 
                fprintf(out, "CharLitteral\n");
                #endif
                fprintf(out, "\tmov rax, %d\n", node->u.character);
                fprintf(out, "\tpush rax\n");
                break;
            case Identifieur:
                #ifndef debug 
                fprintf(out, "Identifieur\n");
                #endif
                writeIdentifieur(node, symbol_table, out, indice_fonct, num_fonct);
                break;
            case Pointeur:
                writeSuiteInstr(FIRSTCHILD(node), symbol_table, out, indice_fonct, num_fonct);
            case Int: case Char:  
                writeSuiteInstr(node->nextSibling, symbol_table, out, indice_fonct, num_fonct);
                break; /* typage expression */
            default: fprintf(stderr, "writeSuiteInstr : Can't reach this point\nKind = %d\n", node->kind);
        }
    }
}


void writeFonction(Node* node, SymbolTable* symbol_table, FILE *out){
    /* ecris la fonction pointee par node dans out
     * node pointe sur l'identifieur de la fonction */
    
    int indice_fonct, num_fonct, nb_var=0;
    
    /* on cherche l'indice et le numéro de la fonction dans la table des symboles */
    for(int i=0; i<symbol_table->taille; i++){
        if(symbol_table->ST[i].role == Fonct && strcmp(symbol_table->ST[i].name, node->u.identifier) == 0){
            indice_fonct = i;
            num_fonct = symbol_table->ST[i].num_fonct;
            break;
        }
    }
    /* on cherche le nombre de variable dans la fonction */
    for(int i=indice_fonct; i<symbol_table->taille; i++){
        if(symbol_table->ST[i].num_fonct == num_fonct && symbol_table->ST[i].role == Var)
            nb_var += 1;
    }
    
    /* debut et fin de toute fonction */
    fprintf(out, "\n%s:\n", node->u.identifier);
    fprintf(out, "\tpush rbp\n");
    fprintf(out, "\tmov rbp, rsp\n");
    fprintf(out, "\tsub rsp, %d\n", nb_var*8);
    writeSuiteInstr(SECONDCHILD(node->nextSibling->nextSibling), symbol_table, out, indice_fonct, num_fonct);
    fprintf(out, "\tmov rsp, rbp\n");
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
}


void writeFonctions(Node* node, SymbolTable* symbol_table, FILE *out){
    /* ecris toutes les fonctions de node dans out
     * node pointe sur DeclFoncts ou un des ses enfants */
    if(node != NULL){
        switch(node->kind){
            case DeclFoncts:
                writeFonctions(FIRSTCHILD(node), symbol_table, out);
                break;
            case DeclFonct:
                if(FIRSTCHILD(node)->kind == Identifieur)
                    writeFonction(FIRSTCHILD(node), symbol_table, out);
                else
                    writeFonction(SECONDCHILD(node), symbol_table, out);
                writeFonctions(node->nextSibling, symbol_table, out);
                break;
            default: fprintf(stderr, "fonctions : Can't reach this point\nKind = %d\n", node->kind);
        }
    }
}


void writePrinte(FILE *out){
    /* ecris la fonction print integer */
    fprintf(out, "\nprinte:\n");
    fprintf(out, "\tpush rbp\n");
    fprintf(out, "\tmov rbp, rsp\n");
    fprintf(out, "\txor rbx, rbx\n");
    
    fprintf(out, "\
\tcmp rdi, 0\n\
\tjg .label0\n\
\tcmp rdi, 0\n\
\tje .label0\n\
\tmov rbx, 1\n\
\tsub rdi, 1\n\
\txor rdi, -1\n\
.label0:\n");
    
    fprintf(out, "\txor rcx, rcx\n");
    fprintf(out, "\tmov rax, rdi\n");
    fprintf(out, "\tmov rdi, 10\n");
    
    fprintf(out, ".printewhile:\n");
    fprintf(out, "\txor rdx, rdx\n");
    fprintf(out, "\tidiv rdi\n");
    fprintf(out, "\tadd rdx, 48\n");
    fprintf(out, "\tpush rdx\n");
    fprintf(out, "\tadd rcx, 1\n");
    fprintf(out, "\tcmp rax, 0\n");
    fprintf(out, "\tjne .printewhile\n");
    fprintf(out, "\
\tcmp rbx, 0\n\
\tjng .label2\n\
\tadd rcx, 1\n\
\tmov rax, 45\n\
\tpush rax\n\
.label2:\n");
    
    fprintf(out, "\tmov rax, 1\n");
    fprintf(out, "\tmov rdi, 1\n");
    fprintf(out, "\tmov rsi, rsp\n");
    fprintf(out, "\tmov rdx, rcx\n");
    fprintf(out, "\timul rdx, 8\n");
    fprintf(out, "\tpush rcx\n");
    fprintf(out, "\tsyscall\n");
    fprintf(out, "\tpop rcx\n");
    
    fprintf(out, ".printeend:\n");
    fprintf(out, "\tpop rdx\n");
    fprintf(out, "\tsub rcx, 1\n");
    fprintf(out, "\tcmp rcx, 0\n");
    fprintf(out, "\tjne .printeend\n");
    
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
}


void writePrintc(FILE *out){
    /* ecris la fonction print character */
    fprintf(out, "\nprintc:\n");
    fprintf(out, "\tpush rbp\n");
    fprintf(out, "\tmov rbp, rsp\n");
    fprintf(out, "\tpush rdi\n");
    fprintf(out, "\tmov rax, 1\n");
    fprintf(out, "\tmov rdi, 1\n");
    fprintf(out, "\tmov rsi, rsp\n");
    fprintf(out, "\tmov rdx, 8\n");
    fprintf(out, "\tpush rcx\n");
    fprintf(out, "\tsyscall\n");
    fprintf(out, "\tpop rcx\n");
    fprintf(out, "\tpop rdi\n");
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
}

void writeReade(FILE *out){
    /* ecris la fonction read integer */
    fprintf(out, "\
reade:\n\
\tpush rbp\n\
\tmov rbp, rsp\n\
\tpush rdi\n\
\t\n\
\txor r8, r8 \n\
\txor rcx, rcx\n\
\txor rbx, rbx\n\
\t\n\
.reade0:\n\
\tmov rax, 0\n\
\tmov rsi, tmp\n\
\tmov rdi, 1\n\
\tmov rdx, 1\n\
\tpush rcx\n\
\tsyscall\n\
\tpop rcx\n\
\t\n\
\tcmp rcx, 0\n\
\tjne .reade1\n\
\tcmp byte [tmp], 45\n\
\tjne .reade1\n\
\tmov r8, 1\n\
\tjmp .reade0\n\
\n\
.reade1:\n\
\tcmp byte [tmp], 47\n\
\tjng .reade2\n\
\tcmp byte [tmp], 57\n\
\tjg .reade2\n\
\t\n\
\timul rbx, 10\n\
\txor rdx, rdx\n\
\tmov dl, byte [tmp]\n\
\tsub rdx, 48\n\
\tadd rbx, rdx\n\
\tadd rcx, 1\n\
\tjmp .reade0\n\
\t\n\
.reade2:\n\
\tcmp r8, 1\n\
\tjne .reade3\n\
\timul rbx, -1\n\
\n\
.reade3:\n\
\tpop rax\n\
\tmov qword [rax], rbx\n\
\tpop rbp\n\
\tret\n\
\t\n");
}


void writeReadc(FILE *out){
    /* ecris la fonction read character */
    fprintf(out, "\nreadc:\n");
    fprintf(out, "\tpush rbp\n");
    fprintf(out, "\tmov rbp, rsp\n");
    fprintf(out, "\tmov qword[rdi], 0\n");
    fprintf(out, "\tcmp byte [tmp], 0\n");
    fprintf(out, "\tje readc0\n");
    fprintf(out, "\tmov al, byte [tmp]\n");
    fprintf(out, "\tmov byte [rdi], al\n");
    fprintf(out, "\tmov qword [tmp], 0\n");
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
    
    fprintf(out, "readc0:\n");
    fprintf(out, "\txor rax, rax\n");
    fprintf(out, "\tmov rsi, rdi\n");
    fprintf(out, "\tmov rdi, 1\n");
    fprintf(out, "\tmov rdx, 1\n");
    fprintf(out, "\tsyscall\n");
    fprintf(out, "\tpop rbp\n");
    fprintf(out, "\tret\n");
}


void writeVarGlobales(SymbolTable* symbol_table, FILE *out){
    /* ecris dans out les var globales de symbol_table */
    for(int i=0; i<symbol_table->taille; i++){
        if(symbol_table->ST[i].num_fonct == 0){
            fprintf(out, "%s dq 0\n", symbol_table->ST[i].name);
        }
    }
}


void writeNasmStart(Node* node, SymbolTable* symbol_table, FILE *out){
    /* ecris les elements principaux du fichier out */
    fprintf(out, "\nsection .data\n");
    fprintf(out, "tmp: dq 0\n");
    writeVarGlobales(symbol_table, out);
    fprintf(out, "\nsection .text\n");
    fprintf(out, "global _start\n");
    writePrinte(out);
    writePrintc(out);
    writeReade(out);
    writeReadc(out);
    writeFonctions(SECONDCHILD(node), symbol_table, out);
    fprintf(out, "\n_start:\n\tcall main\n\tmov rax, 60\n\tmov rdi, 0\n\tsyscall");
}


void writeNasm(Node* node, SymbolTable* symbol_table, FILE *out){
    /* ecrit le code nasm dans out */
    writeNasmStart(node, symbol_table, out);
    fclose(out);
}
