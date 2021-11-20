
#ifndef __ABSTRACT_TREE__

    #define __ABSTRACT_TREE__
    #define FIRSTCHILD(node) node->firstChild
    #define SECONDCHILD(node) node->firstChild->nextSibling
    #define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
    #define FOURTHCHILD(node) node->firstChild->nextSibling->nextSibling->nextSibling
    typedef enum {
        Prog,           /* 0 */
        DeclVars,
        Declarateurs,
        Pointeur,
        Int,
        Char,           /* 5 */
        Identifieur,
        DeclFoncts,
        DeclFonct,
        Parametres,
        Parametre,      /* 10 */
        Corps,
        SuiteInstr,
        Instr,
        Assign,
        Fonction,       /* 15 */
        Built_in,
        If,
        Else,
        While,
        Return,         /* 20 */
        Or,
        And,
        Egal,
        Inegal,
        Strict_Sup,     /* 25 */
        Strict_Inf,
        Superieur,
        Inferieur,
        Plus,
        Moins,          /* 30 */
        Mult,
        Div,
        Mod,
        Not,
        Adr,            /* 35 */
        IntLitteral,
        CharLitteral,
        Arguments,
    } Kind;

    typedef struct Node {
        Kind kind;
        union {
            int integer;
            char character;
            char identifier[64];
        } u;
        struct Node *firstChild, *nextSibling;
        int lineno;
    } Node;

    Node *makeNode(Kind kind);
    void addSibling(Node *node, Node *sibling);
    void addChild(Node *parent, Node *child);
    void deleteTree(Node*node);
    void printTree(Node *node);

#endif

