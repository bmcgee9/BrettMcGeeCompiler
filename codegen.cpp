#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <set>
using namespace std;
#include <iterator>
using namespace std;
#include <unordered_map>
using namespace std;
#include <utility>
using namespace std;
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
//#include "optimizer.h"


void compute_liveness(LLVMBasicBlockRef bb, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses){
	//initialize all of the maps
	unordered_map<LLVMValueRef, int>* inst_index = new unordered_map<LLVMValueRef, int>();
    unordered_map<LLVMValueRef, int>* bb_uses = new unordered_map<LLVMValueRef, int>();
    unordered_map<LLVMValueRef, pair<int, int>*>* live_range = new unordered_map<LLVMValueRef, pair<int, int>*>();
	int index = 0;
    int index2 = 0;
    int lastuse = 0;
	for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
		LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		if (opcode != LLVMAlloca) {
			(*inst_index)[instruction] = index;
            lastuse = index;
            int numuse = 0;
            for (LLVMValueRef instruction2 = LLVMGetFirstInstruction(bb); instruction2; instruction2 = LLVMGetNextInstruction(instruction2)) {
                LLVMOpcode opcode2 = LLVMGetInstructionOpcode(instruction2);
                if (opcode2 != LLVMAlloca) {
                    int numop2 = LLVMGetNumOperands(instruction2);
                    for (int i = 0; i < numop2; i++){
                        if (LLVMGetOperand(instruction2, i) == instruction){
                            lastuse = index2;
                            numuse = numuse + 1;
                        }
                    }
                    index2 = index2 + 1;
                }
            }
            (*bb_uses)[instruction] = numuse;
            index = index + 1;
            pair<int, int> PAIR2Add(index, lastuse);
            (*live_range)[instruction] = &PAIR2Add;
        }
	}
    (*lively)[bb] = live_range;
    (*num_uses)[bb] = bb_uses;
    //inst_index->clear();
    delete(inst_index);
}

LLVMValueRef find_spill(unordered_map<LLVMValueRef, int>* reg_map, unordered_map<LLVMValueRef, int>* bb_uses){
    LLVMValueRef lowest_use = NULL;
    for (pair<LLVMValueRef, int> i : (*reg_map)){
        if ((*reg_map)[i.first] != -1){
            if (lowest_use == NULL){
                lowest_use = i.first;
            } else {
                if ((*bb_uses)[i.first] < (*bb_uses)[lowest_use]){
                    lowest_use = i.first;
                }
            }
        }
    }
    return lowest_use;
}

int findLowAvlReg(set<int>* available_regs){
    int lowestReg = 100;
    int empty = 1;
    for (int reg : (*available_regs)){
        if (reg < lowestReg){
            lowestReg = reg;
        }
        empty = 0;
    }
    if (empty == 1){
        return (-1);
    } else {
        return lowestReg;
    }
}

void print_umap(unordered_map<LLVMValueRef, int>* umap) {
    printf("{");  fflush(stdout);
    for (pair<LLVMValueRef, int> itr : (*umap)) {
        // LLVMDumpValue(itr.first);
        // printf(" --> (%d, %d) | ", itr.second[0], itr.second[1]); 
        // fflush(stdout);
        LLVMDumpValue(itr.first);
        printf(" --> reg (%d) | ", itr.second); 
        fflush(stdout);
    }
    printf("}\n"); fflush(stdout);
} 

void reg_allocate(LLVMValueRef function, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses){
    //for (LLVMValueRef function = LLVMGetFirstFunction(module); function; function = LLVMGetNextFunction(function)) {
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
            unordered_map<LLVMValueRef, int>* reg_map = new unordered_map<LLVMValueRef, int>();
            set<int> available_regs = {0,1,2};
            int lowestAvlReg = 0;
            int instructionNumber = 0;
            for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
                LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
                if (opcode != LLVMAlloca && opcode != LLVMStore && opcode != LLVMBr) {
                    //lowestAvlReg = findLowAvlReg(available_regs);
                    if (lowestAvlReg != -1){
                        (*reg_map)[instruction] = lowestAvlReg;
                        available_regs.erase(lowestAvlReg);
                        lowestAvlReg = findLowAvlReg(&available_regs);
                    } else {
                        LLVMValueRef instructionToSpill = find_spill( reg_map, (*num_uses)[basicBlock]);
                        if ((*(*num_uses)[basicBlock])[instructionToSpill] < (*(*num_uses)[basicBlock])[instruction]){
                            (*reg_map)[instruction] = (*reg_map)[instructionToSpill];
                            (*reg_map)[instructionToSpill] = -1;
                        } else {
                            (*reg_map)[instruction] = -1;
                        }
                    }
                }
                if (opcode != LLVMAlloca){
                    for (LLVMValueRef instruction2 = LLVMGetFirstInstruction(basicBlock); instruction2; instruction2 = LLVMGetNextInstruction(instruction2)) {
                        LLVMOpcode opcode2 = LLVMGetInstructionOpcode(instruction2);
                        if (opcode2 != LLVMAlloca) {
                            if ((*(*lively)[basicBlock])[instruction2]->second == instructionNumber){
                                available_regs.insert((*reg_map)[instruction2]);
                                lowestAvlReg = findLowAvlReg(&available_regs);
                            }
                        }
                    }
                    instructionNumber = instructionNumber + 1;
                }
            }
            (*bbReg)[basicBlock] = reg_map;
            //print_umap((*bbReg)[basicBlock]);
        }
    //}
}

// void runna(LLVMValueRef function){
int main(int argc, char** argv){
	LLVMModuleRef m = 0;

	if (argc == 2){
		char *err = 0;

        LLVMMemoryBufferRef ll_f = 0;

        LLVMCreateMemoryBufferWithContentsOfFile(argv[1], &ll_f, &err);

        if (err != NULL) { 
            printf("error!");
            return 0;
        }
        
        LLVMParseIRInContext(LLVMGetGlobalContext(), ll_f, &m, &err);

        if (err != NULL) {
            printf("error!");
        }
        for (LLVMValueRef function =  LLVMGetFirstFunction(m); function; function = LLVMGetNextFunction(function)) {

            unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>* lively = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>();
            unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
            unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();

            for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
                compute_liveness(basicBlock, lively, num_uses);
            }
            reg_allocate(function, lively, bbReg, num_uses);

            for (auto level1 : (*lively)){
                //level1.second->clear();
                // for (auto level2 : *level1.second){
                //     delete(level2.second);
                // }
                delete(level1.second);
            }
            //lively->clear();
            delete(lively);
            for (auto level1 : (*bbReg)){
                delete(level1.second);
                //level1.second->clear();
            }
            //bbReg->clear();
            delete(bbReg);
            for (auto level1 : (*num_uses)){
                //level1.second->clear();
                delete(level1.second);
            }
            //num_uses->clear();
            delete(num_uses);
        }
        // LLVMDisposeModule(m);
        // LLVMDisposeMemoryBuffer(ll_f);
	}
}