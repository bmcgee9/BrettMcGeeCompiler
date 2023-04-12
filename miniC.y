%token IF FUNC ELSE WHILE RETURN INT VOID EXTERN VAR EQ LE GE NUM PRINT READ

%{
#include <stdio.h>
void yyerror(const char *);
extern int yylex();
extern int yylex_destroy();
extern FILE *yyin;
extern int yylineno;
extern char* yytext;
%}

%start program
%nonassoc IFX
%nonassoc ELSE
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%%
term: VAR | NUM

expr:
 term
 | '-' expr %prec UMINUS 
 | expr '+' expr 
 | expr '-' expr 
 | expr '*' expr 
 | expr '/' expr 
 | '(' expr ')'

condition: 
	expr '>' expr
	| expr '<' expr
	| expr EQ expr
	| expr GE expr
	| expr LE expr
	| '(' condition ')'

asgn_stmt: VAR '=' expr ';' | VAR '=' READ '(' ')' ';'

if_stmt: IF condition stmt %prec IFX
if_else_stmt: IF condition stmt  ELSE stmt
while_loop: WHILE condition stmt
call_statement: PRINT expr ';'
return_statement: RETURN expr ';'

stmt: asgn_stmt
	| if_stmt
	| if_else_stmt
	| while_loop
	| block_stmt
	| call_statement
	| return_statement

stmts: stmts stmt | stmt

decl: INT VAR ';'

var_decls: var_decls decl
	| /* empty */

block_stmt: '{' var_decls stmts '}'

func_header: INT FUNC '(' INT VAR ')'

function_def: func_header block_stmt

extern_read: EXTERN INT READ '(' ')' ';'

extern_print: EXTERN VOID PRINT '(' INT ')' ';'

extern: extern_print extern_read | extern_read extern_print

program: extern function_def
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
