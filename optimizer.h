#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>

LLVMModuleRef createLLVMModel(char * filename);
int commonSubexprElim (LLVMBasicBlockRef bb);
int deleteDeadCode(LLVMBasicBlockRef bb);
int constantFolding(LLVMBasicBlockRef bb);
int constProp(LLVMValueRef function);
void makeOptimizations(LLVMModuleRef module);
int optimize(char* filename);
