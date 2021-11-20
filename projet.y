%{

/* projet.y */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "src/abstract-tree.h"
#include "src/symbol-table.h"
#include "src/trad-nasm.h"

extern int lineno;
extern int character;
extern const char *StringFromKind[];
int code_error = 0;
Node* root;
extern char type[];

int yylex();
int yyleng();
void yyerror(const char *);
Kind strToKind(char* str);
Node* makeIdNode(char*);
Node* assembly(Kind, Kind, Node*, Kind, Node*);
Node* fonction(Kind, char*, char*, int, Node*);

%}

%error-verbose
%union{
    char ident[64];
    int num;
    char character;
    struct Node* node;
}


%token <ident> IDENT TYPE OR AND EQ ORDER
%token VOID READE READC PRINT IF ELSE WHILE RETURN
%token <character> ADDSUB CHARACTER
%token <num> NUM


%type <node> DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar
%type <node> Corps SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp
%%

Prog:  DeclVars DeclFoncts                      { root = assembly(Prog, DeclVars, $1, DeclFoncts, $2); }
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'           { if ($1 == NULL) {$$ = $3;} else {$$ = $1; addSibling($$, $3); } }
    |                                           { $$ = NULL; }
    ;
Declarateurs:
       Declarateurs ',' IDENT                   { $$ = fonction(Declarateurs, type, $3, 0, $1); }
    |  Declarateurs ',' '*' IDENT               { $$ = fonction(Declarateurs, type, $4, 1, $1); }
    |  IDENT                                    { $$ = fonction(Declarateurs, type, $1, 0, NULL); }
    |  '*' IDENT                                { $$ = fonction(Declarateurs, type, $2, 1, NULL); }
    ;
DeclFoncts:
       DeclFoncts DeclFonct                     { $$ = $1; addSibling($$, $2); }
    |  DeclFonct                                { $$ = $1; }
    ;
DeclFonct:
       EnTeteFonct Corps                        { $$ = makeNode(DeclFonct); addChild($$, $1); addChild($$, $2); }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'            { $$ = fonction(-1, $1, $2, 0, NULL); addSibling($$, $4); }
    |  TYPE '*' IDENT '(' Parametres ')'        { $$ = fonction(-1, $1, $3, 1, NULL); addSibling($$, $5); }
    |  VOID IDENT '(' Parametres ')'            { $$ = fonction(-1, "void", $2, 0, NULL); addSibling($$, $4); }
    ;
Parametres:
       VOID                                     { $$ = makeNode(Parametres); };
    |  ListTypVar                               { $$ = makeNode(Parametres); addChild($$, $1); };
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT                { $$ = fonction(Parametre, $3, $4, 0, $1); }
    |  ListTypVar ',' TYPE '*' IDENT            { $$ = fonction(Parametre, $3, $5, 1, $1); }
    |  TYPE IDENT                               { $$ = fonction(Parametre, $1, $2, 0, NULL); }
    |  TYPE '*' IDENT                           { $$ = fonction(Parametre, $1, $3, 1, NULL); }
    ;
Corps: '{' DeclVars SuiteInstr '}'              { $$ = assembly(Corps, DeclVars, $2, SuiteInstr, $3); }
    ;
SuiteInstr:
       SuiteInstr Instr                         { if ($1 == NULL) {$$ = $2;} else {$$ = $1; addSibling($$, $2); } }
    |                                           { $$ = NULL; }
    ;
Instr:
       LValue '=' Exp ';'                       { $$ = makeNode(Assign); addChild($$, $1); addChild($$, $3);  }
    |  READE '(' IDENT ')' ';'                  { $$ = makeNode(Built_in); addChild($$, makeIdNode("reade")); addChild($$, makeIdNode($3)); }
    |  READC '(' IDENT ')' ';'                  { $$ = makeNode(Built_in); addChild($$, makeIdNode("readc")); addChild($$, makeIdNode($3)); }
    |  PRINT '(' Exp ')' ';'                    { $$ = makeNode(Built_in); addChild($$, makeIdNode("print")); addChild($$, $3); }
    |  IF '(' Exp ')' Instr                     { $$ = makeNode(If); addChild($$, $3); addChild($$, $5); }
    |  IF '(' Exp ')' Instr ELSE Instr          { $$ = makeNode(If); addChild($$, $3); addChild($$, $5); addSibling($$, makeNode(Else)); addChild($$->nextSibling, $7); }
    |  WHILE '(' Exp ')' Instr                  { $$ = makeNode(While); addChild($$, $3); addChild($$, $5); }
    |  IDENT '(' Arguments  ')' ';'             { $$ = fonction(Fonction, "void", $1, 0, NULL); addChild($$, $3); }
    |  RETURN Exp ';'                           { $$ = makeNode(Return); addChild($$, $2); }
    |  RETURN ';'                               { $$ = makeNode(Return); }
    |  '{' SuiteInstr '}'                       { $$ = makeNode(SuiteInstr); addChild($$, $2); }
    |  ';'                                      { $$ = NULL; }
    ;
Exp :  Exp OR TB                                { $$ = makeNode(Or); addChild($$, $1); addChild($$, $3); }
    |  TB                                       { $$ = $1; }
    ;
TB  :  TB AND FB                                { $$ = makeNode(And); addChild($$, $1); addChild($$, $3); }
    |  FB                                       { $$ = $1; }
    ;
FB  :  FB EQ M                                  { $$ = makeNode(strToKind($2)); addChild($$, $1); addChild($$, $3); }
    |  M                                        { $$ = $1; }
    ;
M   :  M ORDER E                                { $$ = makeNode(strToKind($2)); addChild($$, $1); addChild($$, $3); }
    |  E                                        { $$ = $1; }
    ;
E   :  E ADDSUB T                               { $$ = makeNode($2 == '+'? Plus: Moins); addChild($$, $1); addChild($$, $3); }
    |  T                                        { $$ = $1; }
    ;    
T   :  T '*' F                                  { $$ = makeNode(Mult); addChild($$, $1); addChild($$, $3); }
    |  T '/' F                                  { $$ = makeNode(Div); addChild($$, $1); addChild($$, $3); }
    |  T '%' F                                  { $$ = makeNode(Mod); addChild($$, $1); addChild($$, $3); }
    |  F                                        { $$ = $1; }
    ;
F   :  ADDSUB F                                 { $$ = makeNode($1 == '+'? Plus: Moins); addChild($$, $2); }
    |  '!' F                                    { $$ = makeNode(Not); addChild($$, $2); }
    |  '&' IDENT                                { $$ = makeNode(Adr); addChild($$, makeIdNode($2)); }
    |  '(' Exp ')'                              { $$ = $2; }
    |  NUM                                      { $$ = makeNode(IntLitteral); $$->u.integer = $1; }
    |  CHARACTER                                { $$ = makeNode(CharLitteral); $$->u.character = $1; }
    |  LValue                                   { $$ = $1; }
    |  IDENT '(' Arguments  ')'                 { $$ = fonction(Fonction, "void", $1, 0, NULL); addChild($$, $3); }; 
    |  '*' IDENT '(' Arguments  ')'             { $$ = makeNode(Pointeur); addChild($$, fonction(Fonction, "void", $2, 0, NULL)); addChild(FIRSTCHILD($$), $4); }
    ;
LValue:
       IDENT                                    { $$ = makeIdNode($1); }
    |  '*' IDENT                                { $$ = makeNode(Pointeur); addChild($$, makeIdNode($2)); }
    ;
Arguments:
       ListExp                                  { $$ = makeNode(Arguments); addChild($$, $1); }
    |                                           { $$ = NULL; }
    ;
ListExp:
       ListExp ',' Exp                          { if ($1 == NULL) {$$ = $3;} else {$$ = $1; addSibling($$, $3); } }
    |  Exp                                      { $$ = $1; }
    ;


%%

int main(int argc, char** argv) {
    SymbolTable symbol_table;
    TableOfProblems top;
    int error;
    FILE *out = NULL;
    
    yyparse();
    
    if(code_error) {
        printf("The tested file is incorrect.\n\n");
        return 1;
    }
    initSymbolTable(&symbol_table);
    initTableOfProblems(&top);
    
    parcoursTree(root, &symbol_table, 0, &top);
    verifFonctionDef(root, &symbol_table, 0, &top, NULL);
    
    /*printTree(root);*/
    error = SymbolTableDontHaveMain(&symbol_table) + printTableOfProblems(&top);
    freeTableOfProblems(&top);
    
    if(error) {
        printf("\nThe tested file is incorrect.\n\n\n");
        freeSymbolTable(&symbol_table);
        return 1;
    }
    if(argc > 1 && strcmp(argv[1], "-o") == 0)
        out = fopen(argv[2], "w");
    if(out == NULL){
        if((out = fopen("tmp.asm", "w")) == NULL){
            fprintf(stderr, "Error : Can't open file !\n");
            exit(1);
        }
    }
    writeNasm(root, &symbol_table, out);
    freeSymbolTable(&symbol_table);
    printf("\nThe tested file is correct.\n\n\n");
    
    return 0;
}


void yyerror(const char *s){
    /* Lorsqu'il y a une erreur, on reparcours le fichier
     * pour pouvoir afficher toute la ligne ainsi que le numéro de la ligne,
     * le numéro du caractère et une flèche. */
    
    int cpt = 1;
    
    code_error = 1;
    fprintf(stderr, "%s line %d column %d\n", s, lineno, character);
    printLine(lineno);
    
    /* affichage de la flèche */
    for(cpt=1; cpt<character; cpt++){
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
    for(cpt=1; cpt<character; cpt++){
        fprintf(stderr, " ");
    }
    fprintf(stderr, "|\n");
}

Kind strToKind(char* str) {
    if (strcmp(str, "char") == 0) {
        return Char;
    }
    if (strcmp(str, "int") == 0) {
        return Int;
    }
    if (strcmp(str, "==") == 0) {
        return Egal;
    }
    if (strcmp(str, "!=") == 0) {
        return Inegal;
    }
    if (strcmp(str, "<") == 0) {
        return Strict_Inf;
    }
    if (strcmp(str, ">") == 0) {
        return Strict_Sup;
    }
    if (strcmp(str, "<=") == 0) {
        return Inferieur;
    }
    if (strcmp(str, ">=") == 0) {
        return Superieur;
    }
    fprintf(stderr,"strToKind : can't reach that point\n");
    exit(1);
}

Node* assembly(Kind rootkind, Kind child1kind, Node* subchild1, Kind child2kind, Node* subchild2) {
    Node* root = makeNode(rootkind);
    
    addChild(root, makeNode(child1kind));
    addChild( FIRSTCHILD(root), subchild1);
    
    addChild(root, makeNode(child2kind));
    addChild(SECONDCHILD(root), subchild2);
    return root;
}

Node* makeTypeNode(char* type, int isptr) {
    Node* node = makeNode(strToKind(type));
    if (isptr) {
        Node* tmp = makeNode(Pointeur);
        addChild(tmp, node);
        node = tmp;
    }
    return node;
}

Node* makeIdNode(char* id) {
    Node* node = makeNode(Identifieur);
    strcpy(node->u.identifier, id);
    return node;
}

Node* fonction(Kind kind, char* type, char* id, int isptr, Node* sibling) {
    Node* node, *tmp;
    node = makeIdNode(id);
    
    if (strcmp(type, "void") != 0) {
        tmp = makeTypeNode(type, isptr);
        addSibling(tmp, node);
        node = tmp;
    }
    if (kind != -1) {
        tmp = makeNode(kind);
        addChild(tmp, node);
        node = tmp;
    }
    if (sibling == NULL) {
        return node;
    }
    addSibling(sibling, node);
    return sibling;
}
