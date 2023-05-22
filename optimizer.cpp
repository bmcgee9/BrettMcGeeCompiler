#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <set>
using namespace std;
#include <iterator>
using namespace std;
#include <unordered_map>
using namespace std;
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
								//printf("returned1SubExpr\n");
								return 1;
							} else {
								int between = 0;
								int canReplace = 1;
								for (LLVMValueRef instructionBtw = LLVMGetFirstInstruction(bb); instructionBtw; instructionBtw = LLVMGetNextInstruction(instructionBtw)){
									//printf("between\n");
									if ((instructionBtw == instruction) || (instructionBtw == instruction2)){
										between++;
										continue;
									}
									/*if (instructionBtw == instruction2){
										between = 0;
										continue;
									}*/
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
									//printf("returned1SubExpr\n");
									return 1;
								}
							}
						}
					}
				}
			}
        }
    }
	//printf("returned0SubExpr\n");
	return 0;
}

int deleteDeadCode(LLVMBasicBlockRef bb){
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
        LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		//printf("toplevelDeadCode\n");
        if (opcode != LLVMStore && opcode != LLVMRet && opcode != LLVMBr && opcode != LLVMSwitch && opcode != LLVMIndirectBr && opcode != LLVMInvoke && opcode != LLVMUnreachable && opcode != LLVMCallBr && opcode != LLVMAlloca){
            if (LLVMGetFirstUse(instruction) == NULL){
                LLVMInstructionEraseFromParent(instruction);
				//printf("returned1DeadCode\n");
				return 1;
            }
        }
    }
	//printf("returned0DeadCode\n");
	return 0;
}

int constantFolding(LLVMBasicBlockRef bb){
	for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
		LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		if (opcode == LLVMAdd){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstAdd(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				//printf("returned1Const\n");
				return 1;
			}
		} else if (opcode == LLVMSub){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstSub(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				//printf("returned1Const\n");
				return 1;
			}
		} else if (opcode == LLVMMul){
			if (LLVMIsConstant(LLVMGetOperand(instruction, 0)) && LLVMIsConstant(LLVMGetOperand(instruction, 1))){
				LLVMValueRef newConst = LLVMConstMul(LLVMGetOperand(instruction, 0), LLVMGetOperand(instruction, 1));
				LLVMReplaceAllUsesWith(instruction, newConst);
				//printf("returned1Const\n");
				return 1;
			}
		}
	}
	//printf("returned0Const\n");
	return 0;
}

int constProp(LLVMValueRef function){
	int changed = 0;
	//initialize all of the maps
	unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>* GEN = new unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>();
	unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>* KILL = new unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>();
	unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>* IN = new unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>();
	unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>* OUT = new unordered_map<LLVMBasicBlockRef, set<LLVMValueRef>*>();
	// create predecessors map
	unordered_map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>*>* previousBlocks = new unordered_map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>*>();
	// initializes a set for each BB key in the map
	for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
		set<LLVMBasicBlockRef>* preds = new set<LLVMBasicBlockRef>();
		(*previousBlocks)[basicBlock] = preds;
	}
	for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
		LLVMValueRef terminator = LLVMGetBasicBlockTerminator(basicBlock);
		int numSucc = LLVMGetNumSuccessors(terminator);
		for (int i = 0; i < numSucc; i++){
			LLVMBasicBlockRef tempBB = LLVMGetSuccessor(terminator, i);
			//preds->insert(tempBB);
			(*previousBlocks)[tempBB]->insert(basicBlock);
		}
	}	
	set<LLVMValueRef>* S = new set<LLVMValueRef>();
	for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
		// creates set of all store instructions in a function
		for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
			LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
			if (opcode == LLVMStore){
				S->insert(instruction);
			}
		}
	}
	for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
		set<LLVMValueRef>* GEN_BB = new set<LLVMValueRef>();
		set<LLVMValueRef>* KILL_BB = new set<LLVMValueRef>();
		for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
			LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
			if (opcode == LLVMStore){
				// creating and managing GEN
				GEN_BB->insert(instruction);
				set<LLVMValueRef>* tempSet = new set<LLVMValueRef>();
				for (LLVMValueRef i : (*GEN_BB)){
					if (LLVMGetOperand(instruction, 1) == LLVMGetOperand(i, 1)){
						if (i != instruction){
							tempSet->insert(i);
						}
					}
				}
				for (LLVMValueRef j : (*tempSet)){
					GEN_BB->erase(j);
				}
				// creating KILL
				for (LLVMValueRef s : (*S)){
					if (LLVMGetOperand(instruction, 1) == LLVMGetOperand(s, 1) && (instruction != s)){
						KILL_BB->insert(s);
					}
				}
			}
		}
		//add this basic block's gen and kill sets to maps
		(*GEN)[basicBlock] = GEN_BB;
		(*KILL)[basicBlock] = KILL_BB;
		
	}
	//sets OUT to GEN for each BB
	for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
		(*OUT)[basicBlock] = new set<LLVMValueRef>((*(*GEN)[basicBlock]));

	}
	int change = 1;
	while (change == 1){
		change = 0;
		for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
			//adding elements to IN from the unions of the OUTs of predecessors
			//set<LLVMValueRef>* IN_BB = new set<LLVMValueRef>();
			(*IN)[basicBlock] = new set<LLVMValueRef>();
			if (previousBlocks->count(basicBlock) != 0){
				if ((*previousBlocks)[basicBlock]->size() != 0){
					for (LLVMBasicBlockRef BB2 : (*(*previousBlocks)[basicBlock])){
						(*IN)[basicBlock]->insert((*OUT)[BB2]->begin(), (*OUT)[BB2]->end());
					}
				}
			}
			
			//oldOut = OUT[B]
			set<LLVMValueRef>* oldOut = new set<LLVMValueRef>((*(*OUT)[basicBlock]));
			//building new OUT[B]
			(*OUT)[basicBlock] = new set<LLVMValueRef>((*(*GEN)[basicBlock]));
			for (auto itr = (*IN)[basicBlock]->begin(); itr != (*IN)[basicBlock]->end(); ++itr){
				//IN[basicBlock].insert(*itr);
				if ((*KILL)[basicBlock]->count(*itr) == 0){
					(*OUT)[basicBlock]->insert(*itr);
				}
			}
			if ((*oldOut) != (*(*OUT)[basicBlock])){
				change = 1;
			}
		}
		
	}
	//the actual constant propagation
	for(LLVMBasicBlockRef BB = LLVMGetFirstBasicBlock(function); BB; BB = LLVMGetNextBasicBlock(BB)){
		// printf("IN is %lu\n", (*IN)[BB]->size());
		// fflush(stdout);
		set<LLVMValueRef>* R = new set<LLVMValueRef>((*(*IN)[BB]));
		//printf("R is %lu\n", R->size());
		set<LLVMValueRef>* toDelete = new set<LLVMValueRef>();
		for (LLVMValueRef instruction = LLVMGetFirstInstruction(BB); instruction; instruction = LLVMGetNextInstruction(instruction)) {
			LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
			//manages the store instructions that are from IN or are in the current BB
			if (opcode == LLVMStore){
				R->insert(instruction);
				set<LLVMValueRef>* tempSet1 = new set<LLVMValueRef>();
				for (LLVMValueRef i : (*R)){
					if ((LLVMGetOperand(instruction, 1) == LLVMGetOperand(i, 1)) && instruction != i){
						tempSet1->insert(i);
					}
				}
				for (LLVMValueRef k : (*tempSet1)){
					R->erase(k);
				}
			}
			if (opcode == LLVMLoad){
				//creates set of stores that store to the ptr in the load instruction
				set<LLVMValueRef>* storeToT = new set<LLVMValueRef>();
				LLVMValueRef t = LLVMGetOperand(instruction, 0);
				for (LLVMValueRef i : (*R)){
					if (LLVMGetOperand(i, 1) == t){
						storeToT->insert(i);
					}
				}
				int storeSameConstant = 1;
				LLVMValueRef constant = NULL; //not initializing this to NULL cost me hours of my life
				// finds which stores with same ptr to load are storing constants
				for (LLVMValueRef Tstore : (*storeToT)){
					if (LLVMIsConstant(LLVMGetOperand(Tstore, 0))){
						for (LLVMValueRef Tstore2 : (*storeToT)){
							if (LLVMGetOperand(Tstore, 0) != LLVMGetOperand(Tstore2, 0)){
								storeSameConstant = 0;
							}
						}
						constant = LLVMGetOperand(Tstore, 0);
						//printf("constant gets set\n");
					}
				}
				if (storeSameConstant == 1){
					if (constant == NULL){
						//printf("NULL constant\n");
						//fflush(stdout);
					} else {
						//LLVMDumpValue(constant);
						long long te = LLVMConstIntGetSExtValue(constant);
						LLVMValueRef constInstruc = LLVMConstInt(LLVMInt32Type(), te, 1);
						//replaces all uses of load instruction with constant and delete load instruction
						LLVMReplaceAllUsesWith(instruction, constInstruc);
						toDelete->insert(instruction);
					}
				}
			}

		}
		if (toDelete->size() != 0){
			for (LLVMValueRef deleteMe : (*toDelete)){
				LLVMInstructionEraseFromParent(deleteMe);
				changed = 1;
			}
		}
	}
	// printf("______________________\n");
	// fflush(stdout);
	if (changed == 1){
		//printf("Const Prop returned 1\n");
		return 1;
	}
	//printf("Const Prop returned 0\n");
	return 0;
}

void makeOptimizations(LLVMModuleRef module){
	for (LLVMValueRef function =  LLVMGetFirstFunction(module); function; function = LLVMGetNextFunction(function)) {
		for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
			// for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
			// 	LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
			// 	if (opcode == LLVMBr){
			// 		int a = LLVMGetNumOperands(instruction);
			// 		int b = 0;
			// 		for (int i = 0; i < a; i++){
			// 			if (LLVMGetValueKind(LLVMGetOperand(instruction, i)) == LLVMBasicBlockValueKind){
			// 				b++;
			// 			}
			// 		}
			// 		printf("%d out of %d\n", b, a);
			// 	}
			// }
			//printf("new BB just dropped\n");
			int done = 1;
			while (done != 0){
				int changed = 0;
				//printf("changing again\n");
				if (constProp(function) == 1){
					changed++;
				}
				//LLVMDumpModule(module);
				if (constantFolding(basicBlock) == 1){
					changed++;
				}
				//LLVMDumpModule(module);
				if (commonSubexprElim(basicBlock) == 1){
					changed++;
				}
				if (deleteDeadCode(basicBlock) == 1){
					changed++;
				} 
				if (changed == 0){
					//printf("set done to zero\n");
					done = 0;
				} else {
					//printf("%d\n", changed);
				}
				//LLVMDumpModule(module);
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
		LLVMPrintModuleToFile(m, "optTestFINAL.txt", NULL);
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
