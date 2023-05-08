#include "ast.h"
//#include <cassert>
#include <cstring>
#include "semanticC.h"

int semanticAnalysis(astNode *node, vector<vector<char* >*> *vec){
	if (node == NULL){
		printf("node is NULL\n");
		return 0;
	}
	//assert(node != NULL);
	//printf("%d\n", node->type);
	switch(node->type){
		case ast_prog:{
						//create stack of vectors
						vector<vector<char* >*> *tableVector = new vector<vector<char* >*> ();
						//vector<char* > *paramSymbols = new vector<char* > ();
						//vec->push_back(paramSymbols);
						if (semanticAnalysis(node->prog.func, tableVector) == 1){
							delete (tableVector);
							return 1;
						} else {
							delete (tableVector);
							return 0;
						}
						//break;
					  }
		case ast_func:{
						//printf("%sFunc: %s\n",indent, node->func.name);
						vector<char* > *paramSymbols = new vector<char* > ();
						vec->push_back(paramSymbols);
						if (node->func.param != NULL){
							//printNode(node->func.param, n+1);
							//semanticAnalysis(node->func.param, vec);
							vec->back()->push_back(node->func.param->var.name);
							//printf("%s\n", node->func.param->var.name);
						}
						int a = semanticAnalysis(node->func.body, vec);
						vec->pop_back();
						delete (paramSymbols);
						if (a == 1){
							return 1;
						}
						return 0;
						//break;
					  }
		case ast_stmt:{
						//printf("%sStmt: \n",indent);
						astStmt stmt = node->stmt;
						if (semanticAnalysisStmt(&stmt, vec) == 1){
							return 1;
						}
						return 0;
						//break;
					  }
		case ast_extern:{
						//printf("%sExtern: %s\n", indent, node->ext.name);
						return 1;
						//break;
					  }
		case ast_var: {	
						//check to see if node->var.name is in stack
						//output error if not
						//iterate backwards through vectors
						bool existing = false;
						for (int i = vec->size()-1; 0 <= i; i--){
							for (int k = vec->at(i)->size()-1; 0 <= k ; k--){
								if (strcmp(vec->at(i)->at(k), node->var.name) == 0){
									existing = true;
								}
							}
						}
						if (!(existing)){
							fprintf(stderr, "The variable used here has not been declared (%s)\n", node->var.name);
						}
						//acts as an end of recursive path
						return 1;
						//break;
					  }
		case ast_cnst: {
						//printf("%sConst: %d\n", indent, node->cnst.value);
						return 1;
						//break;
					}
		case ast_rexpr: {
						//printf("%sRExpr: \n", indent);
						if (semanticAnalysis(node->rexpr.lhs, vec) == 1 && semanticAnalysis(node->rexpr.rhs, vec) == 1){
							return 1;
						}
						return 0;
						//break;
					  }
		case ast_bexpr: {
						if (semanticAnalysis(node->rexpr.lhs, vec) == 1 && semanticAnalysis(node->rexpr.rhs, vec) == 1){
							return 1;
						}
						return 0;
						//break;
					  }
		case ast_uexpr: {
						//printf("%sUExpr: \n", indent);
						if (semanticAnalysis(node->uexpr.expr, vec) == 1){
							return 1;
						}
						return 0;
						//break;
					  }
		default: {
					fprintf(stderr,"Incorrect node type\n");
				 	return 0;
					//exit(1);
				 }
	}
	/*printf("----------------------------\n");
	for (int e = vec->size()-1; 0 <= e; e--){
		for (int f = vec->at(e)->size()-1; 0 <= f ; f--){
			printf("%s\n", (vec->at(e)->at(f))); 
		}
		printf("---\n");
	}*/
	return 0;
}
int semanticAnalysisStmt(astStmt *stmt, vector<vector<char* >*> *vec){
	//assert(stmt != NULL);
	if (stmt == NULL){
		printf("stmt is NULL\n");
		return 0;
	}

	switch(stmt->type){
		case ast_call: { 
							if (stmt->call.param != NULL){
								//check if param is in any symbol tables on stack
								if (semanticAnalysis(stmt->call.param, vec) == 1){
									return 1;
								}
							}
							return 0;
							//break;
						}
		case ast_ret: {
							//printf("%sRet:\n", indent);
							//check if expr is in any symbol tables on stack
							if (semanticAnalysis(stmt->ret.expr, vec) == 1){
								return 1;
							}
							return 0;
							//break;
						}
		case ast_block: {
							//printf("%sBlock:\n", indent);
							vector<astNode*> slist = *(stmt->block.stmt_list);
							vector<astNode*>::iterator it = slist.begin();
							vector<char* > *symbolTable = new vector<char* > ();
							vec->push_back(symbolTable);
							int a = 1;
							while (it != slist.end()){
								//printNode(*it, n+1);
								//semanticAnalysis
								if (semanticAnalysis(*it, vec) == 0){
									a = 0;
								}
								it++;
							}
							vec->pop_back();
							delete (symbolTable);
							if (a == 0){
								return 0;
							}
							return 1;
							//break;
						}
		case ast_while: {
							if (semanticAnalysis(stmt->whilen.cond, vec) == 1 && semanticAnalysis(stmt->whilen.body, vec) == 1){
								return 1;
							}
							return 0;
							//break;
						}
		case ast_if: {
							int a = 1;
							if (semanticAnalysis(stmt->ifn.cond, vec) == 0 || semanticAnalysis(stmt->ifn.if_body, vec) == 0){
								a = 0;
							}
							if (stmt->ifn.else_body != NULL)
							{
								if (semanticAnalysis(stmt->ifn.else_body, vec) == 0){
									a = 0;
								}
							}
							if (a == 0){
								return 0;
							}
							return 1;
							//break;
					}
		case ast_asgn:	{
							if (semanticAnalysis(stmt->asgn.lhs, vec) == 1 && semanticAnalysis(stmt->asgn.rhs, vec) == 1){
								return 1;
							}
							return 0;
							//break;
						}
		case ast_decl:	{
							//printf("%sDecl: %s\n", indent, stmt->decl.name);
							vec->back()->push_back(stmt->decl.name);
							//printf("%s\n", stmt->decl.name);
							return 1;
							//break;
						}
		default: {
					fprintf(stderr,"Incorrect node type\n");
					return 0;
				 	//exit(1);
				 }
	}
}
