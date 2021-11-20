#include "symbol-table.h"

/* ----- Table Of Problems ----- */

extern char* StringFromKind[];

void initTableOfProblems(TableOfProblems *top){
    top->taille = 0;
    top->taille_max = TAILLE;
    top->tab = (Problem *)malloc(sizeof(Problem) * TAILLE);
    if(top->tab == NULL){
        fprintf(stderr, "initTableOfProblems\n");
        exit(1);
    }
}


void reallocTableOfProblems(TableOfProblems *top){
    top->taille_max = 2 * top->taille_max;
    top->tab = (Problem *)realloc(top->tab, sizeof(Problem) * top->taille_max);
    if(top->tab == NULL){
        fprintf(stderr, "reallocTableOfProblems\n");
        exit(1);
    }
}


void freeTableOfProblems(TableOfProblems *top){
    free(top->tab);
    top->tab = NULL;
}

int printProblem(Problem *prob){
	/* renvoie 1 pour une erreur
	 * renvoie 0 pour un warning */
    switch(prob->tos){
        case VarUndeclared: 
            fprintf(stderr, "line %d: error: '%s' undeclared\n", prob->line, prob->name);
            return 1;
        case FonctUndeclared: 
            fprintf(stderr, "line %d: error: function '%s' undeclared\n", prob->line, prob->name);
            return 1;
        case Redeclared: 
            fprintf(stderr, "line %d: error: '%s' redeclared as different kind of symbol\n", prob->line, prob->name);
            return 1;
        case Redefinition: 
            fprintf(stderr, "line %d: error: redefinition of '%s'\n", prob->line, prob->name);
            return 1;
        case UsuallyAFunction: 
            fprintf(stderr, "line %d: warning: '%s' is usually a function\n", prob->line, prob->name);
            return 0;
        case Uninitialized: 
            fprintf(stderr, "line %d: warning: '%s' is used uninitialized in this function\n", prob->line, prob->name);
            return 0;
        case IncompatiblePointeurType:
            fprintf(stderr, "line %d: warning: incompatible pointeur type\n", prob->line);
            return 0;
        case TypePointerInt:
            fprintf(stderr, "line %d: error: makes pointer from integer\n", prob->line);
            return 1;
        case TypeIntPointeur:
            fprintf(stderr, "line %d: error: makes integer from pointer\n", prob->line);
            return 1;
        case TypePointerChar:
            fprintf(stderr, "line %d: error: makes pointer from character\n", prob->line);
            return 1;
        case TypeCharPointeur:
            fprintf(stderr, "line %d: error: makes character from pointer\n", prob->line);
            return 1;
        case IntInChar: 
            fprintf(stderr, "line %d: warning: conversion of an integer in a character\n", prob->line);
            return 0;
        case IntOrChar:
            fprintf(stderr, "line %d: error: '%s' takes an integer or a character\n", prob->line, prob->name);
            return 1;
        case ReadePointer: 
            fprintf(stderr, "line %d: error: 'reade' takes an integer\n", prob->line);
            return 1;
        case ReadeChar: 
            fprintf(stderr, "line %d: warning: 'reade' takes an integer\n", prob->line);
            return 0;
        case PointerOperation: 
            fprintf(stderr, "line %d: error: cannot perform operation on pointer\n", prob->line);
            return 1;
        case NotEnoughArgs: 
            fprintf(stderr, "line %d: error: too few arguments to the function %s\n", prob->line, prob->name);
            return 1;
        case TooMuchArgs: 
            fprintf(stderr, "line %d: error: too much arguments to the function %s\n", prob->line, prob->name);
            return 1;
        case ReturnNotVoid: 
            fprintf(stderr, "line %d: warning: ‘return’ with a value, in function returning void\n", prob->line);
            return 0;
        case ReturnVoid: 
            fprintf(stderr, "line %d: warning: ‘return’ with no value, in function returning non-void\n", prob->line);
            return 0;
        case NoReturn:
            fprintf(stderr, "line %d: error: reaches end of non-void function\n", prob->line);
            return 1;
    }
    return 0;
}


void printLine(int line){
    /* affiche la ligne line du fichier de stdin */
    int lineno;
    char c;
    rewind(stdin);
    lineno = 1;
    while(lineno != line){
        if(getchar() == '\n')
            lineno += 1;
    }
    do{
        c=getchar();
        if(c != EOF)
            fprintf(stderr, "%c",c);
    }while(c != '\n' && c != EOF);
}


int printTableOfProblems(TableOfProblems *top){
    /* renvoie le nombre d'erreurs fatales */
    int i, error=0;
    
    for(i=0; i<top->taille; i++){
        error += printProblem(&(top->tab[i]));
        printLine(top->tab[i].line);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
    return error;
}



/* ----- Symbol Table ----- */


void initSymbolTable(SymbolTable *symbol_table){
    symbol_table->taille = 0;
    symbol_table->taille_max = TAILLE;
    symbol_table->ST = (VarFonct *)malloc(sizeof(VarFonct) * TAILLE);
    if(symbol_table->ST == NULL){
        fprintf(stderr, "initSymbolTable\n");
        exit(1);
    }
}


void reallocSymbolTable(SymbolTable *symbol_table){
    symbol_table->taille_max = 2 * symbol_table->taille_max;
    symbol_table->ST = (VarFonct *)realloc(symbol_table->ST, sizeof(VarFonct) * symbol_table->taille_max);
    if(symbol_table->ST == NULL){
        fprintf(stderr, "reallocSymbolTable\n");
        exit(1);
    }
}


void freeSymbolTable(SymbolTable *symbol_table){
    free(symbol_table->ST);
    symbol_table->ST = NULL;
}


void printVarFonct(VarFonct *VF){
    char tmp[16];
    switch(VF->role){
        case Param: sprintf(tmp,"parametre"); break; 
        case Fonct: sprintf(tmp,"fonction"); break; 
        case Var: sprintf(tmp,"variable"); break;
    }
    printf("\tfonction n°%d, role : %s, pointeur : %s, %s %s, adresse : %s, initialized: %s\n", 
    VF->num_fonct, 
    tmp, 
    (VF->pointeur) ? "oui" : "non", 
    (VF->type == 'i') ? "int" : ((VF->type == 'c') ? "char" : "void"), 
    VF->name,
    VF->adresse,
    VF->initialized?"True":"False");
}


void printSymbolTable(SymbolTable *symbol_table){
    int i;
    printf("\nTable des symboles :\n");
    for(i=0; i<symbol_table->taille; i++){
        printVarFonct(&(symbol_table->ST[i]));
    }
    printf("\n");
}



/* --------------------- */


int builtin(char id[64]){
    return !strcmp(id, "print") || !strcmp(id, "reade") || !strcmp(id, "readc");
}


int SymbolTableDontHaveMain(SymbolTable *symbol_table){
    /* renvoie 1 si la table des symboles possède la fonction main
     * renvoie 0 sinon */
    int i;
    for(i=0; i<symbol_table->taille; i++){
        if(symbol_table->ST[i].role == Fonct && strcmp(symbol_table->ST[i].name, "main") == 0){
            return 0;
        }
    }
    fprintf(stderr, "error: undefined reference to 'main'\n");
    return 1;
}


void addTableOfProblems(Node *id, TableOfProblems *top, TypeOfWorries tos){
    /* on ajoute id à top */
    
    if(top->taille == top->taille_max)
        reallocTableOfProblems(top);
    top->tab[top->taille].line = id->lineno;
    top->tab[top->taille].tos = tos;
    sprintf(top->tab[top->taille].name, "%s", id->u.identifier);
    top->taille += 1;
}


/* Fonctions pour vérifier que l'id utilisé a été déclaré */


int inSymbolTableVerifIdFonct(Node *id, SymbolTable *symbol_table, TableOfProblems *top){
    /* renvoie la position si la fonction de id est dans symbol_table
     * renvoie 0 sinon */
    /* id peut être un nom de fonction ou "print", "reade", "readc" */
    
    int i;
    
    if(builtin(id->u.identifier))
        return 1;
    
    for(i=0; i<symbol_table->taille; i++){
        if( symbol_table->ST[i].role == Fonct && strcmp(symbol_table->ST[i].name, id->u.identifier) == 0 )
            return i;
    }
    addTableOfProblems(id, top, FonctUndeclared);
    return 0;
}


int inSymbolTableVerifIdVar(Node *id, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top){
    /* renvoie la position si la variable de node est dans symbol_table
     * renvoie 0 sinon */
     /* id peut etre un param/var de la fonction ou une var globale */
    
    int i;
    
    for(i=0; i<symbol_table->taille; i++){
        
        if( (symbol_table->ST[i].num_fonct == 0 
            || (symbol_table->ST[i].num_fonct == num_fonct && symbol_table->ST[i].role != Fonct)) 
            && (strcmp(symbol_table->ST[i].name, id->u.identifier) == 0) 
        ){
            return i;
        }
    }
    addTableOfProblems(id, top, VarUndeclared);
    return 0;
}


int inSymbolTableVerifId(Node *id, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top, Role role){
    /* id doit etre un pointeur sur un Identifieur
     * renvoie la position si la var/fonction de id est déjà présente dans symbol_table
     * renvoie 0 sinon */
    if(symbol_table->taille == 0) /* table vide */
        return 0;
    
    if (role == Fonct) 
        return inSymbolTableVerifIdFonct(id, symbol_table, top);
    return inSymbolTableVerifIdVar(id, symbol_table, num_fonct, top);
}


void verifId(Node *node, SymbolTable *symbol_table, TableOfProblems *top, int num_fonct){
    /* vérifie que les var locales et fonctions utilisées dans une fonction soient délcarées
     * node est un pointeur sur un noeud du sous ensemble de type SuiteInstr
     * num_fonct est le numéro de la fonction dans laquelle on se trouve */
    if(node != NULL){
        switch(node->kind){
            case Built_in:
                verifId(node->nextSibling, symbol_table, top, num_fonct);
                break;
            case Fonction:
                inSymbolTableVerifId(FIRSTCHILD(node), symbol_table, num_fonct, top, Fonct);
                verifId(SECONDCHILD(node), symbol_table, top, num_fonct);
                verifId(node->nextSibling, symbol_table, top, num_fonct);
                break;
            case Identifieur: 
                inSymbolTableVerifId(node, symbol_table, num_fonct, top, Var);
            default:
                verifId(FIRSTCHILD(node), symbol_table, top, num_fonct);
                verifId(node->nextSibling, symbol_table, top, num_fonct);
        }
    }
}

int typeCoherence(Node* t1, Node* t2) {
    /* Vérifie que le type du noeud t1 peut être mis dans le type du noeud t2
     * Renvoie 0 si c'est possible sinon son code d'erreur (voir l'enum TypeOfWorries)
     */
    if (t1->kind == 0 || t2->kind == 0) {
        return 0;
    }
    if (t1->kind == Char) {
        return (t2->kind == Pointeur) ? TypePointerChar : 0;
    }
    if (t1->kind == Int) {
        if (t2->kind == Char) return IntInChar;
        if (t2->kind == Pointeur) return TypePointerInt;
        return 0;
    }
    if (t2->kind == Pointeur) {
        return (typeCoherence(FIRSTCHILD(t1), FIRSTCHILD(t2)) == 0) ? 0 : IncompatiblePointeurType;
    }
    return (t2->kind == Int) ? TypeIntPointeur : TypeCharPointeur;
}

/* Fonctions pour vérifier que l'id utilisé n'est pas déjà utilisé */


int inSymbolTableDeclFonct(Node *id, SymbolTable *symbol_table, TableOfProblems *top){
    /* vérifie que l'id n'est pas déjà dans symbol_table, 
     * auquel cas on le met dans top
     * renvoie 1 si la variable de id est dans symbol_table
     * renvoie 0 sinon */
     
    int i;
    if(builtin(id->u.identifier)){
        addTableOfProblems(id, top, Redefinition);
        return 1;
    }
    for(i=0; i<symbol_table->taille; i++){
        
        if( (symbol_table->ST[i].num_fonct == 0 
            || symbol_table->ST[i].role == Fonct )
            && (strcmp(symbol_table->ST[i].name, id->u.identifier) == 0)
        ){
            
            switch(symbol_table->ST[i].role){
                case Fonct:
                    addTableOfProblems(id, top, Redefinition);
                    return 1;
                case Var:
                    addTableOfProblems(id, top, Redeclared);
                    return 1;;
                default:;
            }
        }
    }
    return 0;
}


int inSymbolTableDeclVar(Node *id, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top, Role role){
    /* vérifie que l'id n'est pas déjà dans symbol_table, 
     * auquel cas on le met dans top
     * renvoie 1 si la variable de id est dans symbol_table
     * renvoie 0 sinon */
    
    int i;
    
    if(builtin(id->u.identifier) && num_fonct == 0){
        addTableOfProblems(id, top, Redeclared);
        return 1;
    }
    
    for(i=0; i<symbol_table->taille; i++){
        
        if( (symbol_table->ST[i].num_fonct == 0 
            || symbol_table->ST[i].role == Fonct 
            || symbol_table->ST[i].num_fonct == num_fonct )
            && (strcmp(symbol_table->ST[i].name, id->u.identifier) == 0)
        ){
            
            switch(symbol_table->ST[i].role){
                case Fonct:
                    if(role == Var) {
                        addTableOfProblems(id, top, UsuallyAFunction);
                        return 0;
                    }
                    break;
                case Param:
                    if(role == Var)
                        addTableOfProblems(id, top, Redeclared);
                    else 
                        addTableOfProblems(id, top, Redefinition);
                    return 1;
                case Var:
                    if( (symbol_table->ST[i].num_fonct != 0) || (symbol_table->ST[i].num_fonct == 0 && num_fonct == 0) ){
                        addTableOfProblems(id, top, Redefinition);
                        return 1;
                    }
                    break;
            }
        }
    }
    return 0;
}


int inSymbolTableDecl(Node *id, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top, Role role){
    /* id doit etre un pointeur sur un Identifieur
     * num_fonct est le numéro de la fonction auquel appartient id
     * renvoie 1 si la var/fonction de id est déjà présente dans symbol_table
     * renvoie 0 sinon */
     
    if(symbol_table->taille == 0) /* table vide */
        return 0;
    if(role == Fonct)
        return inSymbolTableDeclFonct(id, symbol_table, top);
    return inSymbolTableDeclVar(id, symbol_table, num_fonct, top, role);
}


/* ----- */


void initializeSymbolTable(Node* node, SymbolTable* symbol_table, int num_fonct, int value) {
    int i;
    for (i = 0; i < symbol_table->taille; i++) {
        VarFonct *s = symbol_table->ST + i;
        if (strcmp(node->u.identifier, s->name)==0 && num_fonct == s->num_fonct
            && s->role != Fonct) {
            s->initialized = value;
            return;
        }
        
    }
}

int isInitialized(Node* node, SymbolTable* symbol_table, int num_fonct) {
    int i;
    for (i = 0; i < symbol_table->taille; i++) {
        VarFonct *s = symbol_table->ST + i;
        if (strcmp(node->u.identifier, s->name)==0 && num_fonct == s->num_fonct
            && s->role != Fonct) {
            return s->initialized;
        }
        
    }
    return -1;
}

int addSymbolTable(Node *node, SymbolTable *symbol_table, int num_fonct, Role role){
    /* node doit etre un pointeur sur le premier fils de Declarateurs, DeclFonct ou Parametre
     * renvoie 0 si la fonction de node est de type void
     * renvoie 2 si le symbole est déjà dans la table des symboles
     * renvoie 1 si autre */
    int nb, i;
    /* vérifie qu'il y a encore de la place */
    if(symbol_table->taille_max == symbol_table->taille)
        reallocSymbolTable(symbol_table);
    
    symbol_table->ST[symbol_table->taille].role = role;
    symbol_table->ST[symbol_table->taille].initialized = (num_fonct == 0 || role != Var);
    
    /* règle spéciale pour une fonction de type retour void */
    if(node->kind == Identifieur){
        symbol_table->ST[symbol_table->taille].pointeur = 0;
        symbol_table->ST[symbol_table->taille].type = 'v';
        symbol_table->ST[symbol_table->taille].num_fonct = num_fonct;
        sprintf(symbol_table->ST[symbol_table->taille].name, "%s", node->u.identifier);
        symbol_table->taille += 1;
        return 0;
    }
    switch(node->kind){
        case Pointeur:
            symbol_table->ST[symbol_table->taille].pointeur = 1;
            if(node->firstChild->kind == Int)
                symbol_table->ST[symbol_table->taille].type = 'i';
            else
                symbol_table->ST[symbol_table->taille].type = 'c';
            break;
        case Int:
            symbol_table->ST[symbol_table->taille].pointeur = 0;
            symbol_table->ST[symbol_table->taille].type = 'i';
            break;
        case Char:
            symbol_table->ST[symbol_table->taille].pointeur = 0;
            symbol_table->ST[symbol_table->taille].type = 'c';
            break;
        default: fprintf(stderr, "ajoutSymbolTable : on arrive jamais ici.\nKind=%d\n",node->kind); exit(1);
    }
    
    /* ajoute une adresse si c'est un parametre de fonction ou une variable dans une fonction */
    if(num_fonct != 0 && role != Fonct){
        switch(role){
            case Var:
                nb = 1;
                i = symbol_table->taille-1;
                while(symbol_table->ST[i].role == Var){
                    nb += 1;
                    i--;
                }
                sprintf(symbol_table->ST[symbol_table->taille].adresse, "rbp - %d", 8*nb);
                break;
            case Param:
                nb = 1;
                i = symbol_table->taille-1;
                while(symbol_table->ST[i].role == Param){
                    nb += 1;
                    i--;
                }
                switch(nb){
                    case 1: sprintf(symbol_table->ST[symbol_table->taille].adresse, "rdi"); break;
                    case 2: sprintf(symbol_table->ST[symbol_table->taille].adresse, "rsi"); break;
                    case 3: sprintf(symbol_table->ST[symbol_table->taille].adresse, "rdx"); break;
                    case 4: sprintf(symbol_table->ST[symbol_table->taille].adresse, "rcx"); break;
                    case 5: sprintf(symbol_table->ST[symbol_table->taille].adresse, "r8"); break;
                    case 6: sprintf(symbol_table->ST[symbol_table->taille].adresse, "r9"); break;
                    default: sprintf(symbol_table->ST[symbol_table->taille].adresse, "rbp + %d", (nb-5)*8);
                }
                
                break;
            default: fprintf(stderr, "ajoutSymbolTable : on arrive jamais ici.\nRole=%d\n",role); exit(1);;
        }
    }
    
    symbol_table->ST[symbol_table->taille].num_fonct = num_fonct;
    sprintf(symbol_table->ST[symbol_table->taille].name, "%s", node->nextSibling->u.identifier);
    symbol_table->taille += 1;
    return 1;
}

Node* copyNode(Node* node) {
    Node* new = malloc(sizeof(Node));
    if (node == NULL) return NULL;
    if (new == NULL) {
        fprintf(stderr, "Run out of memory\n"); exit(1);
    }
    new->kind = node->kind;
    new->firstChild = node->firstChild;
    new->nextSibling = node->nextSibling;
    new->u = node->u;
    new->lineno=node->lineno;
    return new;
}

Node* lastChild(Node* node) {
    Node* tmp = FIRSTCHILD(node);
    if (tmp == NULL) return NULL;
    while (tmp->nextSibling != NULL) {
        tmp = tmp->nextSibling;
    }
    return tmp;
}

Node* varFonctToNode(VarFonct pos) {
    Node *tmp = NULL;
    if (pos.pointeur) { 
        tmp = makeNode(Pointeur);
        addChild(tmp, makeNode(pos.type=='c'?Char:Int));
    } else {
        tmp = makeNode(pos.type=='c'?Char:Int);
    }
    return tmp;
}

Node* getTypeFromSymbolTable(Node* node, SymbolTable* st, Role role, int f_number) {

    int i;
    /* printf("\n%d, %s, %d, %d\n", node->u.identifier[0], node->u.identifier, f_number, role); */
    for (i = 0; i < st->taille; i++) {
        VarFonct pos = st->ST[i];
        /* printf("%s, %d, %d, %c, %d, %d\n", pos.name, pos.num_fonct, pos.role, pos.type, pos.role, role); */
        /* Si je cherche une fonction et que je suis sur une fonction, 
         * ou que je suis une var globale
         * ou que je suis une var dans le scope de la fonction 
         * et que mon nom est le meme que celui cherché */
         if (strcmp(pos.name, node->u.identifier) == 0) {
             if ((role == Fonct && pos.role == Fonct) ||
                (pos.role != Fonct && (pos.num_fonct == 0 || pos.num_fonct == f_number))) {
                if (pos.type == 'v') return NULL;
                return varFonctToNode(pos);
            }
        }
    }
    return NULL;
}

void analyseFnct(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number) {
    Node* tmp = NULL, *tmp2;
    VarFonct* vf;
    int ret;
    analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
    tmp = getTypeFromSymbolTable(FIRSTCHILD(node), symbol_table, Fonct, f_number);
    if (tmp != NULL) {
        addChild(node, tmp); 
    }
    
    vf = symbol_table->ST + inSymbolTableVerifIdFonct(FIRSTCHILD(node), symbol_table, top) + 1;
    tmp2 = FIRSTCHILD(SECONDCHILD(node));
    
    /* Parcours des arguments de la fonction et test avec les types attendus */
    while (tmp2 != NULL && vf->role == Param) {
        tmp = varFonctToNode(*vf);
        if ((ret = typeCoherence(lastChild(tmp2), tmp)) != 0) {
            addTableOfProblems(node, top, ret);
        }
        tmp2 = tmp2->nextSibling;
        vf += 1;
    }
    /*printTree(node);*/
    if (tmp2 == NULL && vf->role == Param) {
        addTableOfProblems(FIRSTCHILD(node), top, NotEnoughArgs);
    }
    if (tmp2 != NULL && vf->role != Param) {
        addTableOfProblems(FIRSTCHILD(node), top, TooMuchArgs);
    }
}

void analyseBuilt_in(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number) {
    
    if (strcmp(FIRSTCHILD(node)->u.identifier, "print") == 0) {
        analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
        if (lastChild(SECONDCHILD(node))->kind == Pointeur) {
            addTableOfProblems(FIRSTCHILD(node), top, IntOrChar);
        }
    }
    else if (strcmp(FIRSTCHILD(node)->u.identifier, "reade") == 0) {
        initializeSymbolTable(SECONDCHILD(node), symbol_table, f_number, 1);
        analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
        if (lastChild(SECONDCHILD(node))->kind == Pointeur) {
            addTableOfProblems(FIRSTCHILD(node), top, ReadePointer);
        }
        else if (lastChild(SECONDCHILD(node))->kind == Char) {
            addTableOfProblems(FIRSTCHILD(node), top, ReadeChar);
        }
    }
    else if (strcmp(FIRSTCHILD(node)->u.identifier, "readc") == 0) {
        initializeSymbolTable(SECONDCHILD(node), symbol_table, f_number, 1);
        analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
        if (lastChild(SECONDCHILD(node))->kind == Pointeur) {
            addTableOfProblems(FIRSTCHILD(node), top, IntOrChar);
        }
    }
}

void testPointerOperation(Node* node, TableOfProblems* top) {
    if (lastChild(FIRSTCHILD(node))->kind == 0) {
        return;
    }
    if (lastChild(SECONDCHILD(node))->kind == 0) {
        return;
    }
    
    if (lastChild(FIRSTCHILD(node))->kind == Pointeur) {
        addTableOfProblems(FIRSTCHILD(node), top, PointerOperation);
    }
    if (lastChild(SECONDCHILD(node))->kind == Pointeur) {
        addTableOfProblems(SECONDCHILD(node), top, PointerOperation);
    }
}

void analyseExp(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number) {
    Node* tmp;
    int before;
    if (node == NULL) return;
    
    /*printf("Exp: %s (%d)\n", StringFromKind[node->kind], node->kind);*/
    switch (node->kind) {
        case Or: case And:
        case Egal: case Inegal:
        case Strict_Sup: case Strict_Inf:
        case Superieur: case Inferieur:
        case Mult: case Div:
        case Mod:
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
            testPointerOperation(node, top);
            addChild(node, makeNode(Int));
            break;
        case Plus: case Moins:
            if (SECONDCHILD(node) == NULL) {
                analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            } else {
                analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
                analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
                testPointerOperation(node, top);
            }
            addChild(node, makeNode(Int));
            break;
        case Not:
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            addChild(node, makeNode(Int));
            break;
        case Adr:
            before = isInitialized(FIRSTCHILD(node), symbol_table, f_number);
            initializeSymbolTable(FIRSTCHILD(node), symbol_table, f_number, 1);
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            initializeSymbolTable(FIRSTCHILD(node), symbol_table, f_number, before);
            tmp = makeNode(Pointeur);
            addChild(tmp, FIRSTCHILD(FIRSTCHILD(node)));
            addChild(node, tmp);
            break;
        case IntLitteral:
            addChild(node, makeNode(Int));
            break;
        case CharLitteral:
            addChild(node, makeNode(Char));
            break;
        case Built_in:
            analyseBuilt_in(node, symbol_table, top, f_number);
            break;
        case Fonction:
            if (!isInTOP(node->firstChild, top)) {
                analyseFnct(node, symbol_table, top, f_number);
            } else {
                addChild(node, makeNode(0));
            }
            break;
        case Identifieur:
            if (isInTOP(node, top)) {
                addChild(node, makeNode(0));
            } else {
                if (!isInitialized(node, symbol_table, f_number)) {
                    addTableOfProblems(node, top, Uninitialized);
                }
                addChild(node, getTypeFromSymbolTable(node, symbol_table, Var, f_number));
            }
            break;
        case Arguments:
            printTableOfProblems(top);
            for (tmp = FIRSTCHILD(node); tmp != NULL; tmp = tmp->nextSibling) {
                analyseExp(tmp,symbol_table,top, f_number);
            }
            break;
        case Pointeur:
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            tmp = lastChild(FIRSTCHILD(node));
            if (tmp == NULL || tmp->kind == Char) { /* if (tmp == NULL || tmp->kind != Pointer) { de base */
                addTableOfProblems(FIRSTCHILD(node), top, TypePointerChar);
                addChild(node, makeNode(0));
            }
            else if(tmp->kind == Int){
                addTableOfProblems(FIRSTCHILD(node), top, TypePointerInt);
                addChild(node, makeNode(0));
            }
            else {
                addChild(node, FIRSTCHILD(tmp));
            }
            break;
        default:
            fprintf(stderr, "analyseExp : Can't reach this point, kind=%d\n", node->kind);
    }
}

int isInTOP(Node* node, TableOfProblems* top) {
    int i;
    for (i = 0; i < top->taille; i++) {
        Problem pb = top->tab[i];
        //~ printf("%d : %d || %s : %s || %d --> %d\n", pb.line, node->lineno, pb.name, node->u.identifier, pb.tos, pb.line == node->lineno && strcmp(pb.name, node->u.identifier) == 0 && pb.tos != UsuallyAFunction);
        if (pb.line == node->lineno && strcmp(pb.name, node->u.identifier) == 0 && pb.tos != UsuallyAFunction) {
            return 1;
        }
    }
    return 0;
}

void analyseInstr(Node* node, SymbolTable* symbol_table, TableOfProblems* top, int f_number) {
    Node* tmp;
    int ret;
    if (node == NULL) return;
    /*printf("Ins: %s (%d)\n", StringFromKind[node->kind], node->kind);*/
    switch (node->kind) {
        case Assign:
            analyseExp(SECONDCHILD(node), symbol_table, top, f_number);
            if (isInTOP(node->firstChild, top)) {
                addChild(node, makeNode(0));
                break;
            }
            initializeSymbolTable(FIRSTCHILD(node), symbol_table, f_number, 1);
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            addChild(node, copyNode(lastChild(SECONDCHILD(node))));
            tmp = lastChild(FIRSTCHILD(node));
            if (tmp->kind == 0) /* Si le type de la left value ne peut pas être comprise on coupe (genre int a; *a = 3;)*/
                break;
            
            if ((ret = typeCoherence(lastChild(node), tmp)) != 0) {
                addTableOfProblems(FIRSTCHILD(node), top, ret);
            }
            break;
        case Built_in:
            analyseBuilt_in(node, symbol_table, top, f_number);
            break;
        case Fonction:
            if (!isInTOP(node->firstChild, top)) {
                analyseFnct(node, symbol_table, top, f_number);
            } else {
                addChild(node, makeNode(0));
            }
            break;
        case While:
        case If:
            analyseExp(FIRSTCHILD(node), symbol_table, top, f_number);
            analyseInstr(SECONDCHILD(node), symbol_table, top, f_number);
            break;
        case Else:
            analyseInstr(FIRSTCHILD(node), symbol_table, top, f_number);
            break;
        case Return:
            if (FIRSTCHILD(node) != NULL) {
                analyseExp(FIRSTCHILD(node),symbol_table, top, f_number);
                addChild(node, copyNode(lastChild(FIRSTCHILD(node))));
            }
            break;
        case SuiteInstr:
            analyseInstr(FIRSTCHILD(node), symbol_table, top, f_number);
            break;
        default:
            fprintf(stderr, "analyseInstr : Can't reach this point, kind=%d\n", node->kind);
    }
    analyseInstr(node->nextSibling, symbol_table, top, f_number);
}

void parcoursTree(Node *node, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top){
    /* parcours le node pour construire la table des symboles */
    if(node != NULL){
        switch(node->kind){
            case Prog: 
                parcoursTree(FIRSTCHILD(node), symbol_table, num_fonct, top); 
                parcoursTree(SECONDCHILD(node), symbol_table, num_fonct+1, top); 
                break;
            case DeclVars: 
                parcoursTree(FIRSTCHILD(node), symbol_table, num_fonct, top); 
                break;
            case Declarateurs:
                if(!inSymbolTableDecl(SECONDCHILD(node), symbol_table, num_fonct, top, Var))
                    addSymbolTable(node->firstChild, symbol_table, num_fonct, Var);
                parcoursTree(node->nextSibling, symbol_table, num_fonct, top); 
                break;
            case DeclFoncts:
                parcoursTree(FIRSTCHILD(node), symbol_table, num_fonct, top);
                break;
            case DeclFonct:
                /* 2 cas : fonction void et autres */
                if(FIRSTCHILD(node)->kind == Identifieur){ /* void */
                    if(!inSymbolTableDecl(FIRSTCHILD(node), symbol_table, num_fonct, top, Fonct)){
                        addSymbolTable(FIRSTCHILD(node), symbol_table, num_fonct, Fonct);
                        parcoursTree(SECONDCHILD(node), symbol_table, num_fonct, top);   /* ajout des param de la fonct */
                        parcoursTree(THIRDCHILD(node), symbol_table, num_fonct, top);    /* ajout des var de la fonct */
                    }
                }
                else{ /* autres */
                    if(!inSymbolTableDecl(SECONDCHILD(node), symbol_table, num_fonct, top, Fonct)){
                        addSymbolTable(FIRSTCHILD(node), symbol_table, num_fonct, Fonct);
                        parcoursTree(THIRDCHILD(node), symbol_table, num_fonct, top);    /* ajout des param de la fonct */
                        parcoursTree(FOURTHCHILD(node), symbol_table, num_fonct, top);   /* ajout des var de la fonct */
                    }
                }
                parcoursTree(node->nextSibling, symbol_table, num_fonct+1, top); /* fonction suivante */
                break;
            case Parametres:
                parcoursTree(FIRSTCHILD(node), symbol_table, num_fonct, top);
                break;
            case Parametre:
                if(!inSymbolTableDecl(SECONDCHILD(node), symbol_table, num_fonct, top, Param))
                    addSymbolTable(node->firstChild, symbol_table, num_fonct, Param);
                parcoursTree(node->nextSibling, symbol_table, num_fonct, top); 
                break;
            case Corps:
                parcoursTree(FIRSTCHILD(node), symbol_table, num_fonct, top);    /* ajout des var locales */
                break;
            default:;
        }
    }
}

int verifReturnType(Node* node, TableOfProblems* top, Node* type) {
    Node *tmp = NULL;
    int ret, foundReturn;
    if (node == NULL) return 0;
    switch (node->kind) {
        case Return:
            tmp = lastChild(node);
            if (tmp == NULL) {
                if (type->kind != 0) addTableOfProblems(node, top, ReturnVoid);
                break;
            }
            if (type->kind == 0) {
                if (tmp != NULL) addTableOfProblems(node, top, ReturnNotVoid);
                break;
            }
            if ((ret = typeCoherence(tmp, type)) != 0)
                addTableOfProblems(node, top, ret);
            foundReturn = 1;
            break;
        case Else: case SuiteInstr:
            foundReturn = verifReturnType(FIRSTCHILD(node), top, type);
            break;
        case If:
            foundReturn = verifReturnType(SECONDCHILD(node), top, type);
            break;
        default:;
    }
    return verifReturnType(node->nextSibling, top, type) || foundReturn;
}

void verifFonctionDef(Node* node, SymbolTable *symbol_table, int num_fonct, TableOfProblems *top, Node* type) {
    if(node != NULL){
        switch(node->kind){
            case Prog:
                verifFonctionDef(SECONDCHILD(node), symbol_table, num_fonct+1, top, type); 
                break;
            case DeclFoncts:
                verifFonctionDef(FIRSTCHILD(node), symbol_table, num_fonct, top, type);
                break;
            case DeclFonct:
                /* 2 cas : fonction void et autres */
                if(FIRSTCHILD(node)->kind == Identifieur){ /* void */
                    if(!isInTOP(FIRSTCHILD(node), top)){
                        verifFonctionDef(THIRDCHILD(node), symbol_table, num_fonct, top, makeNode(0));    /* ajout des var de la fonct */
                    }
                }
                else{ /* autres */
                    if(!isInTOP(SECONDCHILD(node), top)){
                        verifFonctionDef(FOURTHCHILD(node), symbol_table, num_fonct, top, FIRSTCHILD(node));   /* ajout des var de la fonct */
                    }
                }
                verifFonctionDef(node->nextSibling, symbol_table, num_fonct+1, top, type); /* fonction suivante */
                break;
            case Corps:
                verifFonctionDef(FIRSTCHILD(node), symbol_table, num_fonct, top, type);    /* ajout des var locales */
                verifId(SECONDCHILD(node), symbol_table, top, num_fonct);        /* vérifie les var/fonct utilisées dans SuiteInstr */
                analyseInstr(FIRSTCHILD(SECONDCHILD(node)), symbol_table, top, num_fonct);
                if (!verifReturnType(SECONDCHILD(node), top, type)) {
                    addTableOfProblems(node, top, NoReturn);
                }
                break;
            default:;
        }
    }
}


