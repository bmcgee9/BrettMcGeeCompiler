
%{
#include <stdio.h>
#include "ast.h"
void yyerror(const char *);
extern int yylex();
extern int yylex_destroy();
extern FILE *yyin;
extern int yylineno;
extern char* yytext;
%}

%union {
	int ival;
	char character;
	char* sname;
	astNode* npl;
	vector<astNode*> *svec_ptr;
}

%token <ival> NUM
%token <sname> VAR
%token IF FUNC ELSE WHILE RETURN INT VOID EXTERN EQ LE GE PRINT READ NEQ

%type <svec_ptr> stmts
%type <npl> term expr stmt block_stmt condition asgn_stmt if_stmt if_else_stmt while_loop call_statement return_statement declare_statement function_def extern_print extern_read extern program

%start program
%nonassoc IFX
%nonassoc ELSE
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%%
term: VAR {$$ = createVar($1);} | NUM {$$ = createCnst($1);}

expr: '-' term %prec UMINUS {$$ = createUExpr($2, uminus);}
 | term '+' term {$$ = createBExpr($1, $3, add);}
 | term '-' term {$$ = createBExpr($1, $3, sub);}
 | term '*' term {$$ = createBExpr($1, $3, mul);}
 | term '/' term {$$ = createBExpr($1, $3, divide);}
 | term

condition: 
	term '>' term {$$ = createRExpr($1, $3, gt);}
	| term '<' term {$$ = createRExpr($1, $3, lt);}
	| term EQ term {$$ = createRExpr($1, $3, eq);}
	| term GE term {$$ = createRExpr($1, $3, ge);}
	| term LE term {$$ = createRExpr($1, $3, le);}
	| term NEQ term {$$ = createRExpr($1, $3, neq);}

asgn_stmt: VAR '=' expr ';' {astNode* tnptr = createVar($1);
							 $$ = createAsgn(tnptr, $3);} 
		   | VAR '=' READ '(' ')' ';' {$$ = createAsgn($1, $3);}

if_stmt: IF condition stmt %prec IFX {$$ = createIF($2, $3, NULL);}
if_else_stmt: IF condition stmt ELSE stmt {$$ = createIF($2, $3, $5);}
while_loop: WHILE condition stmt {$$ = createWhile($2, $3);}
call_statement: PRINT expr ';' {$$ = createCall($1, $2);}
return_statement: RETURN expr ';' {$$ = createRet($2);}

stmt: asgn_stmt {$$ = $1;}
	| if_stmt {$$ = $1;}
	| if_else_stmt {$$ = $1;}
	| while_loop {$$ = $1;}
	| block_stmt {$$ = $1;}
	| call_statement {$$ = $1;}
	| return_statement {$$ = $1;}
	| declare_statement {$$ = $1;}

stmts: stmts stmt {$$ = $1;
				   $$->push_back($2);}
	| stmt {$$ = new vector<astNode*> ();
			$$->push_back($1);}

declare_statement: INT VAR ';' {$$ = createDecl($2);}

block_stmt: '{' stmts '}' {$$ = createBlock($2);}

function_def: INT VAR '(' INT VAR ')' block_stmt {$$ = createFunc($2, $5, $7);}

extern_read: EXTERN INT READ '(' ')' ';' {$$ = createExtern($3);}

extern_print: EXTERN VOID PRINT '(' INT ')' ';' {$$ = createExtern($3);}

extern: extern_print {$$ = $1;}
	| extern_read {$$ = $1;}

program: extern extern function_def {$$ = createProg($1, $2, $3);
									 printNode($$);}
%%

int main(int argc, char** argv){
	if (argc == 2){
		yyin = fopen(argv[1], "r");
	}

	yyparse();
	//call semantic analysis function here

	if (yyin != stdin)
		fclose(yyin);

	yylex_destroy();
	
	return 0;
}


void yyerror(const char *){
	fprintf(stdout, "Syntax error %d\n", yylineno);
}
