#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include "optimizer.h"

#define prt(x) if(x) { printf("%s\n", x); }

LLVMModuleRef createLLVMModel(char * filename){
	char *err = 0;

	LLVMMemoryBufferRef ll_f = 0;
	LLVMModuleRef m = 0;

	LLVMCreateMemoryBufferWithContentsOfFile(filename, &ll_f, &err);

	if (err != NULL) { 
		prt(err);
		return NULL;
	}
	
	LLVMParseIRInContext(LLVMGetGlobalContext(), ll_f, &m, &err);

	if (err != NULL) {
		prt(err);
	}

	return m;
}


int commonSubexprElim (LLVMBasicBlockRef bb){
	for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
        LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
        int numop = LLVMGetNumOperands(instruction);
		//printf("topLevel\n");
        for (LLVMValueRef instruction2 = LLVMGetFirstInstruction(bb); instruction2; instruction2 = LLVMGetNextInstruction(instruction2)){
            LLVMOpcode opcode2 = LLVMGetInstructionOpcode(instruction2);
			if (instruction != instruction2 && (opcode != LLVMAlloca)){
				//printf("middleLevel\n");
				if ((opcode2 == opcode) && (LLVMGetFirstUse(instruction2) != NULL)){
					int numop2 = LLVMGetNumOperands(instruction2);
					int same = 0;
					if (numop2 == numop){
						for (int i = 0; i < numop; i++){
							//printf("in numop loop\n");
							if (LLVMGetOperand(instruction, i) == LLVMGetOperand(instruction2, i)){
								same++;
							}
						}
						if (same == numop){
							if (opcode != LLVMLoad){
								LLVMReplaceAllUsesWith(instruction2, instruction);
								printf("returned1SubExpr\n");
								return 1;
							} else {
								int between = 0;
								int canReplace = 1;
								for (LLVMValueRef instructionBtw = LLVMGetFirstInstruction(bb); instructionBtw; instructionBtw = LLVMGetNextInstruction(instructionBtw)){
									//printf("between\n");
									if (instructionBtw == instruction){
										between = 1;
										continue;
									}
									if (instructionBtw == instruction2){
										between = 0;
										continue;
									}
									if (between == 1){
										LLVMOpcode opcodeBtw = LLVMGetInstructionOpcode(instructionBtw);
										if (opcodeBtw == LLVMStore){
											if (LLVMGetOperand(instructionBtw, 1) == LLVMGetOperand(instruction2, 0)){
												canReplace = 0;
											}
										}
									}
								}
								if (canReplace == 1){
									LLVMReplaceAllUsesWith(instruction2, instruction);
									printf("returned1SubExpr\n");
									return 1;
								}
							}
						}
					}
				}
			}
        }
    }
	printf("returned0SubExpr\n");
	return 0;
}

int deleteDeadCode(LLVMBasicBlockRef bb){
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
        LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		//printf("toplevelDeadCode\n");
        if (opcode != LLVMStore && opcode != LLVMRet && opcode != LLVMBr && opcode != LLVMSwitch && opcode != LLVMIndirectBr && opcode != LLVMInvoke && opcode != LLVMUnreachable && opcode != LLVMCallBr && opcode != LLVMAlloca){
            if (LLVMGetFirstUse(instruction) == NULL){
                LLVMInstructionEraseFromParent(instruction);
				printf("returned1DeadCode\n");
				return 1;
            }
        }
    }
	printf("returned0DeadCode\n");
	return 0;
}

int constantFolding(LLVMBasicBlockRef bb){
	for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
		LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		if (opcode == LLVMAdd){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstAdd(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				printf("returned1Const\n");
				return 1;
			}
		} else if (opcode == LLVMSub){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstSub(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				printf("returned1Const\n");
				return 1;
			}
		} else if (opcode == LLVMMul){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstMul(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				printf("returned1Const\n");
				return 1;
			}
		}
	}
	printf("returned0Const\n");
	return 0;
}

void makeOptimizations(LLVMModuleRef module){
	for (LLVMValueRef function =  LLVMGetFirstFunction(module); function; function = LLVMGetNextFunction(function)) {
		for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
			printf("new BB just dropped\n");
			int done = 1;
			while (done != 0){
				int changed = 0;
				printf("changing again\n");
				if (commonSubexprElim(basicBlock) == 1){
					changed++;
				}
				if (deleteDeadCode(basicBlock) == 1){
					changed++;
				} 
				if (constantFolding(basicBlock) == 1){
					changed++;
				}
				if (changed == 0){
					printf("set done to zero\n");
					done = 0;
				} else {
					printf("%d\n", changed);
				}
			}
		}
 	}
}

int optimize(char* filename)
{
	LLVMModuleRef m;

	m = createLLVMModel(filename);

	if (m != NULL){
		//LLVMDumpModule(m);
		makeOptimizations(m);
		LLVMPrintModuleToFile(m, "optTest1.txt", NULL);
	}
	else {
	    printf("module is NULL\n");
	}
	
	return 0;
}

/*int main(int argc, char** argv)
{
	LLVMModuleRef m;

	if (argc == 2){
		m = createLLVMModel(argv[1]);
	}
	else{
		m = NULL;
		return 1;
	}

	if (m != NULL){
		//LLVMDumpModule(m);
		optimize(m);
		LLVMPrintModuleToFile(m, "optTest.txt", NULL);
	}
	else {
	    printf("m is NULL\n");
	}
	
	return 0;
}*/
