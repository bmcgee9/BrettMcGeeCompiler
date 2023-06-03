#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <set>
using namespace std;
#include <iterator>
using namespace std;
#include <unordered_map>
using namespace std;
#include <utility>
using namespace std;
#include <string>
using namespace std;
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>

/*
codegen.h
*/

void compute_liveness(LLVMBasicBlockRef bb, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses);
LLVMValueRef find_spill(unordered_map<LLVMValueRef, int>* reg_map, unordered_map<LLVMValueRef, int>* bb_uses);
int findLowAvlReg(set<int>* available_regs);
void print_umap(unordered_map<LLVMValueRef, int>* umap);
void print_lively(unordered_map<LLVMValueRef, pair<int, int>>* umap);
int has_reg(unordered_map<LLVMValueRef, int>* reg_map, LLVMValueRef instruction);
void reg_allocate(LLVMValueRef function, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses);
unordered_map<LLVMBasicBlockRef, string>* createBBLabels(LLVMValueRef function);
void printDirectives(LLVMModuleRef m, unordered_map<LLVMBasicBlockRef, string>* BBLabels);
void printFunctionEnd();
unordered_map<LLVMValueRef, int>* getOffsetMap(LLVMModuleRef m, int* localMem);
char getReg(int regNum);
void codegen(LLVMModuleRef m, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg);
int runCodegen(LLVMModuleRef m);