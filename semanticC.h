#include <stdio.h>
#include <stdlib.h>

int semanticAnalysis(astNode *node, vector<vector<char* >*> *vec);

int semanticAnalysisStmt(astStmt *stmt, vector<vector<char* >*> *vec);
