%{
/* projet.lex */
#include "projet.tab.h"
#include <string.h>
int lineno = 1;
int character = 0;
char type[8];

void add_line(char *dest, char *src);
void clean_line(char *line);
void strToChar(void);

%}

%option noyywrap
%option nounput
%option noinput

%x COMMENTAIRE1


%%
\/\*                                {
                                        character += yyleng;
                                        BEGIN COMMENTAIRE1;
                                    }
<COMMENTAIRE1>\/\*                  {
                                        printf("Line %d : \"/*\" No nested comments.\n", lineno);
                                        return 1;
                                    }
<COMMENTAIRE1>.                     {
                                        character += yyleng;
                                    }
<COMMENTAIRE1>[\n]                  {
                                        character = 0;
                                        lineno += 1;
                                    }
<COMMENTAIRE1>\*\/                  {
                                        character += yyleng;
                                        BEGIN INITIAL;
                                    }



[\n]                                {
                                        character = 0;
                                        lineno += 1;
                                    }
[0-9]+                              {
                                        character += yyleng;
                                        yylval.num = atoi(yytext);
                                        return NUM;
                                    }
"void"                              {
                                        character += yyleng;
                                        return VOID;
                                    }
"reade"                             {
                                        character += yyleng;
                                        return READE;
                                    }
"readc"                             {
                                        character += yyleng;
                                        return READC;
                                    }
"print"                             {
                                        character += yyleng;
                                        return PRINT;
                                    }
"if"                                {
                                        character += yyleng;
                                        return IF;
                                    }
"else"                              {
                                        character += yyleng;
                                        return ELSE;
                                    }
"while"                             {
                                        character += yyleng;
                                        return WHILE;
                                    }
"return"                            {
                                        character += yyleng;
                                        return RETURN;
                                    }
"int"|"char"                        {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext);
                                        strcpy(type, yytext);
                                        return TYPE;
                                    }
[a-zA-Z_][a-zA-Z_0-9]*              {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext);
                                        return IDENT;
                                    }
"||"                                {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext); 
                                        return OR;
                                    }
"&&"                                {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext); 
                                        return AND;
                                    }
"!="|"=="                           {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext); 
                                        return EQ;
                                    }
"<"|">"|"<="|">="                   {
                                        character += yyleng;
                                        strcpy(yylval.ident, yytext); 
                                        return ORDER;
                                    }
"+"|"-"                             {
                                        character += yyleng;
                                        yylval.character = yytext[0]; 
                                        return ADDSUB;
                                    }
'[^']'|'\\a'|'\\b'|'\\f'|'\\r'|'\\n'|'\\t'|'\\v'|'\\''|'\\\\'|'\\0'    {
                                        character += yyleng;
                                        strToChar();
                                        return CHARACTER;
                                    }
[;,*(){}=/%!&]                      {
                                        character += yyleng;
                                        return yytext[0];
                                    }
[\t]                                {
                                        character += 4;
                                    }
" "                                 {
                                        character += yyleng;
                                    }
.                                   {
                                        return yytext[0];
                                    }
<<EOF>>                             {
                                        return 0;
                                    }


%%

void add_line(char *dest, char *src){
    strcat(dest, src);
}

void clean_line(char *line){
    line[0] = '\0';
}

void strToChar(void){
    if(yytext[1] == '\\'){
        switch(yytext[2]){
            case 'a': yylval.character = '\a'; break;
            case 'b': yylval.character = '\b'; break;
            case 'f': yylval.character = '\f'; break;
            case 'r': yylval.character = '\r'; break;
            case 'n': yylval.character = '\n'; break;
            case 't': yylval.character = '\t'; break;
            case 'v': yylval.character = '\v'; break;
            case '\'': yylval.character = '\''; break;
            case '\\': yylval.character = '\\'; break;
        }
    }
    else
        yylval.character = yytext[1];
}





