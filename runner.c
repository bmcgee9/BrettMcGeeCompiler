#include <stdio.h>
#include "ast.h"
#include "semanticC.h"
#include "optimizer.h"
#include "runner.h"
#include "y.tab.h"
//#include "globals.h"
void yyerror(const char *);
extern int yylex();
extern int yylex_destroy();
extern FILE *yyin;
extern int yylineno;
//extern int yyparse();
extern astNode *root;

int main(int argc, char** argv){
	if (argc >= 2){
		yyin = fopen(argv[1], "r");
	}

	yyparse();
	if (root == NULL){
		printf("NULL Root\n");
	}
	printf("made it past parsing");
	fflush(stdout);
	//extern astNode* root;
	//call semantic analysis function here
	int i = semanticAnalysis(root, NULL);
	printf("made it past semantic analysis");
	fflush(stdout);
	if (i == 1 && argv[2] != NULL){
		optimize(argv[2]);
	}
	if (yyin != stdin)
		fclose(yyin);

	yylex_destroy();
	freeNode(root);
	return 0;
}


void yyerror(const char *){
	fprintf(stdout, "Syntax error %d\n", yylineno);
}