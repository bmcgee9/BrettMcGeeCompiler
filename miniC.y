
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
%type <npl> term expr stmt block_stmt

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

stmts: stmts stmt {$$ = $1;
				   $$->push_back($2);}
	| stmt {$$ = new vector<astNode*> ();
			$$->push_back($1);}

decl: INT VAR ';'

var_decls: var_decls decl
	| /* empty */

block_stmt: '{' var_decls stmts '}' {$$ = createBlock($2); printNode($$);}

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
