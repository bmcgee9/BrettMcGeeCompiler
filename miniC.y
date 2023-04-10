%token IF FUNC ELSE WHILE RETURN INT VOID EXTERN WORD OP NUMP PRINT READ

%{
#include <stdio.h>
void yyerror(const char *);
extern int yylex();
extern int yylex_destroy();
extern FILE *yyin;
extern int yylineno;
extern char* yytext;
%}

%start CODE
%nonassoc TAG
%nonassoc ELSE
%nonassoc UMINUS
%nonassoc UNEG


%%
VarDec : INT WORD ';'
ARITH : '+' | '*' | '/' | '-' %prec UMINUS
NUM : NUMP | '-' NUMP %prec UNEG | '(' NUM ')'
ExFunc : EXTERN VOID PRINT '(' INT ')' ';' | EXTERN INT READ '(' ')' ';'
AROP : WORD ARITH WORD | WORD ARITH NUM | NUM ARITH WORD | NUM ARITH NUM
BOP : WORD OP WORD | WORD OP NUM | NUM OP WORD | NUM OP NUM
Ret : RETURN '(' AROP ')' ';' | RETURN '(' WORD ')' ';' | RETURN '(' NUM ')' ';'
ASSIGN : WORD '=' AROP ';' | WORD '=' WORD ';' | WORD '=' NUM ';' | WORD '=' READFunc ';'
Conditional : IF '(' BOP ')' '{' CODE '}' ELSE '{' CODE '}' | IF '(' BOP ')' ASSIGN | IF '(' BOP ')' Ret | IF '(' BOP ')' PRINTFunc | IF '(' BOP ')' READFunc | IF '(' BOP ')' '{' CODE '}' %prec TAG
PRINTFunc : PRINT '(' WORD ')' ';'
READFunc : READ '(' ')'
CreateFunc : INT FUNC '(' INT WORD ')' '{' CODE '}'
WhileLoop : WHILE '(' BOP ')' '{' CODE '}'
CODE : CODE WhileLoop | CODE ASSIGN | CODE Conditional | CODE Ret | CODE PRINTFunc | CODE READFunc | CODE ExFunc | CODE CreateFunc | CODE VarDec
		| VarDec | ASSIGN | Conditional | Ret | PRINTFunc | READFunc | ExFunc | WhileLoop

%%

int main(int argc, char** argv){
	if (argc == 2){
		yyin = fopen(argv[1], "r");
	}

	yyparse();

	if (yyin != stdin)
		fclose(yyin);

	yylex_destroy();
	
	return 0;
}


void yyerror(const char *){
	fprintf(stdout, "Syntax error %d\n", yylineno);
}
