
%{
#include <stdio.h>
#include "ast.h"
#include <cassert>
#include <cstring>
void yyerror(const char *);
void semanticAnalysis(astNode *node, vector<vector<char* >*> *vec);
void semanticAnalysisStmt(astStmt *stmt, vector<vector<char* >*> *vec);
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
	| '(' expr ')' {$$ = $2;}

condition: 
	term '>' term {$$ = createRExpr($1, $3, gt);}
	| term '<' term {$$ = createRExpr($1, $3, lt);}
	| term EQ term {$$ = createRExpr($1, $3, eq);}
	| term GE term {$$ = createRExpr($1, $3, ge);}
	| term LE term {$$ = createRExpr($1, $3, le);}
	| term NEQ term {$$ = createRExpr($1, $3, neq);}
	| '(' condition ')' {$$ = $2;}

asgn_stmt: VAR '=' expr ';' {astNode* tnptr = createVar($1);
							 $$ = createAsgn(tnptr, $3);} 
		   | VAR '=' call_statement {astNode* tnptr = createVar($1);
									 $$ = createAsgn(tnptr, $3);}

if_stmt: IF condition stmt %prec IFX {$$ = createIf($2, $3, NULL);}
if_else_stmt: IF condition stmt ELSE stmt {$$ = createIf($2, $3, $5);}
while_loop: WHILE condition stmt {$$ = createWhile($2, $3);}
call_statement: PRINT expr ';' {$$ = createCall("print", $2);}
				| READ '(' ')' ';' {$$ = createCall("read", NULL);}

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

function_def: INT FUNC '(' INT VAR ')' block_stmt {astNode* tmpvar = createVar($5);
												  $$ = createFunc("func", tmpvar, $7);}

extern_read: EXTERN INT READ '(' ')' ';' {$$ = createExtern("read");}

extern_print: EXTERN VOID PRINT '(' INT ')' ';' {$$ = createExtern("print");}

extern: extern_print {$$ = $1;}
	| extern_read {$$ = $1;}

program: extern extern function_def {$$ = createProg($1, $2, $3);
									 semanticAnalysis($$, NULL);}
%%

int main(int argc, char** argv){
	if (argc == 2){
		yyin = fopen(argv[1], "r");
	}

	yyparse();
	//call semantic analysis function here
	//semanticAnalysis()

	if (yyin != stdin)
		fclose(yyin);

	yylex_destroy();
	
	return 0;
}


void yyerror(const char *){
	fprintf(stdout, "Syntax error %d\n", yylineno);
}

void semanticAnalysis(astNode *node, vector<vector<char* >*> *vec){
	assert(node != NULL);
	
	switch(node->type){
		case ast_prog:{
						//create stack of vectors
						vector<vector<char* >*> *tableVector = new vector<vector<char* >*> ();
						vector<char* > *paramSymbols = new vector<char* > ();
						tableVector->push_back(paramSymbols);
						semanticAnalysis(node->prog.func, tableVector);
						break;
					  }
		case ast_func:{
						//printf("%sFunc: %s\n",indent, node->func.name);
						if (node->func.param != NULL){
							//printNode(node->func.param, n+1);
							semanticAnalysis(node->func.param, vec);
						}
						//printNode(node->func.body, n+1);
						break;
					  }
		case ast_stmt:{
						//printf("%sStmt: \n",indent);
						astStmt stmt = node->stmt;
						semanticAnalysisStmt(&stmt, vec);
						break;
					  }
		case ast_extern:{
						//printf("%sExtern: %s\n", indent, node->ext.name);
						break;
					  }
		case ast_var: {	
						//check to see if node->var.name is in stack
						//output error if not
						//iterate backwards through vectors
						for (int i = vec->size()-1; 0 <= i; i--){
							for (int k = vec->at(i)->size()-1; 0 <= k ; k++){
								if (strcmp(vec->at(i)->at(k), node->var.name) != 0){
									fprintf(stderr, "the variable used here has not been declared");
								}
							}
						}
						//acts as an end of recursive path
						break;
					  }
		case ast_cnst: {
						//printf("%sConst: %d\n", indent, node->cnst.value);
						 break;
					}
		case ast_rexpr: {
						//printf("%sRExpr: \n", indent);
						semanticAnalysis(node->rexpr.lhs, vec);
						semanticAnalysis(node->rexpr.rhs, vec);
						break;
					  }
		case ast_bexpr: {
						semanticAnalysis(node->rexpr.lhs, vec);
						semanticAnalysis(node->rexpr.rhs, vec);
						break;
					  }
		case ast_uexpr: {
						//printf("%sUExpr: \n", indent);
						semanticAnalysis(node->uexpr.expr, vec);
						break;
					  }
		default: {
					fprintf(stderr,"Incorrect node type\n");
				 	exit(1);
				 }
	}
}
void semanticAnalysisStmt(astStmt *stmt, vector<vector<char* >*> *vec){
	assert(stmt != NULL);

	switch(stmt->type){
		case ast_call: { 
							if (stmt->call.param != NULL){
								//check if param is in any symbol tables on stack
								semanticAnalysis(stmt->call.param, vec);
							}
							break;
						}
		case ast_ret: {
							//printf("%sRet:\n", indent);
							//check if expr is in any symbol tables on stack
							semanticAnalysis(stmt->ret.expr, vec);
							break;
						}
		case ast_block: {
							//printf("%sBlock:\n", indent);
							vector<astNode*> slist = *(stmt->block.stmt_list);
							vector<astNode*>::iterator it = slist.begin();
							vector<char* > *symbolTable = new vector<char* > ();
							vec->push_back(symbolTable);
							while (it != slist.end()){
								//printNode(*it, n+1);
								//semanticAnalysis
								semanticAnalysis(*it, vec);
								it++;
							}
							break;
						}
		case ast_while: {
							semanticAnalysis(stmt->whilen.cond, vec);
							semanticAnalysis(stmt->whilen.body, vec);
							break;
						}
		case ast_if: {
							semanticAnalysis(stmt->ifn.cond, vec);
							semanticAnalysis(stmt->ifn.if_body, vec);
							if (stmt->ifn.else_body != NULL)
							{
								semanticAnalysis(stmt->ifn.else_body, vec);
							}
							break;
					}
		case ast_asgn:	{
							semanticAnalysis(stmt->asgn.lhs, vec);
							semanticAnalysis(stmt->asgn.rhs, vec);
							break;
						}
		case ast_decl:	{
							//printf("%sDecl: %s\n", indent, stmt->decl.name);
							vec->back()->push_back(stmt->decl.name);
							break;
						}
		default: {
					fprintf(stderr,"Incorrect node type\n");
				 	exit(1);
				 }
	}
}
