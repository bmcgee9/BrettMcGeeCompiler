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
//#include "optimizer.h"


void compute_liveness(LLVMBasicBlockRef bb, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses){
	//initialize all of the maps
	unordered_map<LLVMValueRef, int>* inst_index = new unordered_map<LLVMValueRef, int>();
    unordered_map<LLVMValueRef, int>* bb_uses = new unordered_map<LLVMValueRef, int>();
    unordered_map<LLVMValueRef, pair<int, int>>* live_range = new unordered_map<LLVMValueRef, pair<int, int>>();
	int index = 0;
    //int index2 = 0;
    //int lastuse = 0;
	for (LLVMValueRef instruction = LLVMGetFirstInstruction(bb); instruction; instruction = LLVMGetNextInstruction(instruction)) {
		LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
		if (opcode != LLVMAlloca) {
			//(*inst_index)[instruction] = index;
            int lastuse = index;
            int numuse = 0;
            int index2 = 0;
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
            pair<int, int> PAIR2Add = make_pair(index, lastuse);
            index = index + 1;
            (*live_range)[instruction] = PAIR2Add;
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
        LLVMOpcode opcode = LLVMGetInstructionOpcode(i.first);
        if (!(LLVMIsAArgument(i.first)) && opcode != LLVMAlloca){
            if ((*reg_map)[i.first] != -1){
                if (lowest_use == NULL){
                    lowest_use = i.first;
                } else {
                    if ((*bb_uses)[i.first] <= (*bb_uses)[lowest_use]){
                        lowest_use = i.first;
                    }
                }
            }
        }
    }
    return lowest_use;
}

int findLowAvlReg(set<int>* available_regs){
    int lowestReg = 4;
    int empty = 1;
    for (int reg : (*available_regs)){
        if (reg == 1 || reg == 2 || reg == 3){
            if (reg < lowestReg){
                lowestReg = reg;
            }
            empty = 0;
        }
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
        printf(" --> reg (%d) | \n ", itr.second); 
        fflush(stdout);
    }
    printf("}\n"); fflush(stdout);
} 

void print_lively(unordered_map<LLVMValueRef, pair<int, int>>* umap) {
    printf("{");  fflush(stdout);
    for (pair<LLVMValueRef, pair<int, int>> itr : (*umap)) {
        // LLVMDumpValue(itr.first);
        // printf(" --> (%d, %d) | ", itr.second[0], itr.second[1]); 
        // fflush(stdout);
        LLVMDumpValue(itr.first);
        printf(" --> lively (%d, %d) | ", itr.second.first, itr.second.second); 
        fflush(stdout);
    }
    printf("}\n"); fflush(stdout);
} 

int has_reg(unordered_map<LLVMValueRef, int>* reg_map, LLVMValueRef instruction){
    if ((*reg_map)[instruction] == 1 || (*reg_map)[instruction] == 2 || (*reg_map)[instruction] == 3){
        return 1;
    }
    return 0;
}

void reg_allocate(LLVMValueRef function, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses){
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
        unordered_map<LLVMValueRef, int>* reg_map = new unordered_map<LLVMValueRef, int>();
        set<int> available_regs = {1,2,3};
        int lowestAvlReg = 1;
        int instructionNumber = 0;
        for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
            LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
            if (opcode == LLVMAlloca){
                continue;
            }
            else if (opcode == LLVMCall && LLVMGetFirstUse(instruction) == NULL){
                int numOps = LLVMGetNumOperands(instruction);
                for (int i = 0; i < numOps; i++){
                    if ((*(*lively)[basicBlock])[LLVMGetOperand(instruction, i)].second == instructionNumber && has_reg(reg_map, LLVMGetOperand(instruction, i)) == 1){
                        available_regs.insert((*reg_map)[LLVMGetOperand(instruction, i)]);
                    }
                }
            }
            else if (opcode == LLVMBr || opcode == LLVMStore){
                int numOps = LLVMGetNumOperands(instruction);
                for (int i = 0; i < numOps; i++){
                    if ((*(*lively)[basicBlock])[LLVMGetOperand(instruction, i)].second == instructionNumber && has_reg(reg_map, LLVMGetOperand(instruction, i)) == 1){
                        available_regs.insert((*reg_map)[LLVMGetOperand(instruction, i)]);
                    }
                }
            }
            else {
                if ((opcode == LLVMAdd || opcode == LLVMSub || opcode == LLVMMul) && has_reg(reg_map, LLVMGetOperand(instruction,0)) == 1 && (*(*lively)[basicBlock])[LLVMGetOperand(instruction, 0)].second == instructionNumber){
                    (*reg_map)[instruction] = (*reg_map)[LLVMGetOperand(instruction, 0)];
                    if (has_reg(reg_map, LLVMGetOperand(instruction,1)) == 1 && (*(*lively)[basicBlock])[LLVMGetOperand(instruction, 1)].second == instructionNumber){
                        available_regs.insert((*reg_map)[LLVMGetOperand(instruction, 1)]);
                    }
                } else if ((findLowAvlReg(&available_regs) == 1 || findLowAvlReg(&available_regs) == 2 || findLowAvlReg(&available_regs) == 3)){
                    (*reg_map)[instruction] = findLowAvlReg(&available_regs);
                    // LLVMDumpValue(instruction);
                    // printf(" --> reg %d \n", (*reg_map)[instruction]);
                    // fflush(stdout);
                    available_regs.erase(findLowAvlReg(&available_regs));
                } else if (findLowAvlReg(&available_regs) == -1){
                    LLVMValueRef instructionToSpill = find_spill(reg_map, (*num_uses)[basicBlock]);
                    if ((*(*num_uses)[basicBlock])[instructionToSpill] >= (*(*num_uses)[basicBlock])[instruction]){
                        (*reg_map)[instruction] = -1;
                        // LLVMDumpValue(instruction);
                        // printf(" --> reg %d (spilled)\n", -1);
                        // fflush(stdout);
                    } else {
                        (*reg_map)[instruction] = (*reg_map)[instructionToSpill];
                        // LLVMDumpValue(instruction);
                        // printf(" --> reg %d (take spilled value)\n", (*reg_map)[instructionToSpill]);
                        // fflush(stdout);
                        (*reg_map)[instructionToSpill] = -1;
                        // LLVMDumpValue(instructionToSpill);
                        // printf(" --> reg %d (spilled)\n", -1);
                        // fflush(stdout);
                    }
                    int numOps = LLVMGetNumOperands(instruction);
                    for (int i = 0; i < numOps; i++){
                        if ((*(*lively)[basicBlock])[LLVMGetOperand(instruction, i)].second == instructionNumber && has_reg(reg_map, LLVMGetOperand(instruction, i)) == 1){
                            available_regs.insert((*reg_map)[LLVMGetOperand(instruction, i)]);
                        }
                    }
                }
            }

            instructionNumber = instructionNumber + 1;
            // for (int i : available_regs){
            //     printf("%d ", i);
            // }
            // printf("\n");
        }
        (*bbReg)[basicBlock] = reg_map;
        //print_umap((*bbReg)[basicBlock]);
    }
}

//createBBLabels : This function populates a map where key is a LLVMBasicBlockRef and 
//associated value is a char * that you can use as label when generating code.
unordered_map<LLVMBasicBlockRef, string>* createBBLabels(LLVMValueRef function){
    unordered_map<LLVMBasicBlockRef, string>* BBLabels = new unordered_map<LLVMBasicBlockRef, string>();
    int BBnum = 0;
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
        string label = ".LF";
        string name = label + to_string(BBnum);
        (*BBLabels)[basicBlock] = name;
        BBnum++;
    }
    if (BBLabels == NULL){
        fprintf(stderr, "BBLabels is null\n");
    }
    return (BBLabels);
}

/*
2) printDirectives: This function emits the required directives for your function 
and also assembly instructions to push callers %ebp and update the %ebp to current value of %esp.
*/
void printDirectives(LLVMModuleRef m, unordered_map<LLVMBasicBlockRef, string>* BBLabels){
    size_t size = 0;
    const char* filename = LLVMGetSourceFileName(m, &size);
    fprintf(stdout, "\t.file\t\"%s\"\n", filename);
    fprintf(stdout, "\t.text\n\t.globl\tfunc\n\t.type\tfunc, @function\nfunc:\n");
    fprintf(stdout, "%s:\n", (*BBLabels)[LLVMGetFirstBasicBlock(LLVMGetFirstFunction(m))].c_str());
    //LLVMGetValueName 
    fprintf(stdout, "\tpushl\t%%ebp\n");
    fprintf(stdout, "\tmovl\t%%esp, %%ebp\n");

}

/*
printFunctionEnd: This function emits the assembly instructions 
to restore the value of %esp and %ebp, and the ret instruction. 
*/
void printFunctionEnd(){
    fprintf(stdout, "\tpopl\t%%ebx\n");
    fprintf(stdout, "\tleave\n");
    fprintf(stdout, "\tret\n");
}

/*
getOffsetMap: This function will populate the global map offset_map. 
This map associates each value(instruction) to the memory offset of that value from %ebp.
The keys in this map are LLVMValueRef and values are integers. 
This function will also initialize an integer variable localMem that indicates the number of 
bytes required to store the local values.
*/
unordered_map<LLVMValueRef, int>* getOffsetMap(LLVMModuleRef m, int* localMem){
    LLVMValueRef function = LLVMGetFirstFunction(m);
    if (m == NULL){
        return NULL;
    }
    LLVMValueRef parameter = LLVMGetParam(function, 0);
    int offset = 0; 
    int numAlloc = 0;
    unordered_map<LLVMValueRef, int>* offsetMap = new unordered_map<LLVMValueRef, int>();
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
        for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
            LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
            if (opcode == LLVMAlloca){
                //if (numAlloc != 0){
                offset = offset - 4;
                *localMem = *localMem + 4;
                (*offsetMap)[instruction] = offset;
                // fprintf(stdout, "%d: ", offset);
                // LLVMDumpValue(instruction);
                // printf("\n");
                //}
                //numAlloc++;
            }
            if (opcode == LLVMLoad){
                if (!(LLVMIsConstant(LLVMGetOperand(instruction, 0)))){
                    int off = (*offsetMap)[LLVMGetOperand(instruction, 0)];
                    (*offsetMap)[instruction] = off;
                    // fprintf(stdout, "%d: ", off);
                    // LLVMDumpValue(instruction);
                    // printf("\n");
                }
            }
            // if opcode == add or subtract or something you can just choose the offset of one of the operands
            // if (opcode == LLVMAdd || opcode == LLVMSub || opcode == LLVMMul || opcode == LLVMUDiv){ 
            //     int off = (*offsetMap)[LLVMGetOperand(instruction, 0)];
            //     (*offsetMap)[instruction] = off;
                // fprintf(stdout, "%d: ", off);
                // LLVMDumpValue(instruction);
                // printf("\n");
            //}
            if (opcode == LLVMStore){
                // check if first param is argument and then make offset of second operand 8
                if (LLVMIsAArgument(LLVMGetOperand(instruction, 0))){
                    int off = 8;
                    (*offsetMap)[LLVMGetOperand(instruction, 1)] = off;
                } else if (!(LLVMIsConstant(LLVMGetOperand(instruction, 0)))){
                    int off = (*offsetMap)[LLVMGetOperand(instruction, 1)];
                    (*offsetMap)[LLVMGetOperand(instruction, 0)] = off; 
                    // fprintf(stdout, "%d: ", off);
                    // LLVMDumpValue(LLVMGetOperand(instruction, 1));
                    // printf("\n");
                }
            }

        }
    }
    if (offsetMap == NULL){
        return NULL;
    }
    return (offsetMap);
}

//get reg char
char getReg(int regNum){
    if (regNum == 1){
        return ('b');
    } else if (regNum == 2){
        return ('c');
    } else if (regNum == 3){
        return ('d');
    }
    return 'z';
}

//codegen algorithm from the pseudocode
void codegen(LLVMModuleRef m, unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg){
    if (m == NULL){
        fprintf(stderr, "moduleRef is NULL\n");
    }
    if (bbReg == NULL){
        fprintf(stderr, "bbReg is NULL\n");
    }
    //for (LLVMValueRef function =  LLVMGetFirstFunction(m); function; function = LLVMGetNextFunction(function)) {
        LLVMValueRef function =  LLVMGetFirstFunction(m);
        unordered_map<LLVMBasicBlockRef, string>* BBLabels = createBBLabels(function);
        printDirectives(m, BBLabels);
        int localMem = 0;
        int firstBB = 1;
        LLVMValueRef param1 = LLVMGetParam(function, 0);
        unordered_map<LLVMValueRef, int>* offsetMap = getOffsetMap(m, &localMem);
        //print_umap(offsetMap);
        fprintf(stdout, "\tsubl\t$%d, %%esp\n", localMem);
        fprintf(stdout, "\tpushl\t%%ebx\n");
        // fprintf(stdout, "\tpushl\t%%ebx\n");
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
            if (firstBB == 0){
                fprintf(stdout, "%s:\n", (*BBLabels)[basicBlock].c_str());
            }
            firstBB = 0;
            for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction)) {
                LLVMOpcode opcode = LLVMGetInstructionOpcode(instruction);
                if (opcode == LLVMRet){
                    if (LLVMIsConstant(LLVMGetOperand(instruction, 0))){
                        fprintf(stdout, "\tmovl\t$%lld, %%eax\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 0)));
                    } else {
                        if ((*(*bbReg)[basicBlock])[LLVMGetOperand(instruction, 0)] != -1){
                            int regNum = (*(*bbReg)[basicBlock])[LLVMGetOperand(instruction, 0)];
                            fprintf(stdout, "\tmovl\t%%e%cx, %%eax\n", getReg(regNum));
                        } else {
                            fprintf(stdout, "\tmovl\t%d(%%ebp), %%eax\n", (*offsetMap)[LLVMGetOperand(instruction, 0)]);
                        }
                    }
                    printFunctionEnd();
                }
                if (opcode == LLVMLoad){
                    //print_umap((*bbReg)[basicBlock]);
                    //LLVMDumpValue(LLVMBasicBlockAsValue(basicBlock));
                    //unordered_map<LLVMValueRef, int>* test = (*bbReg)[basicBlock];
                    if ((*(*bbReg)[basicBlock])[instruction] != -1){
                        int regNum = (*(*bbReg)[basicBlock])[instruction];
                        fprintf(stdout, "\tmovl\t%d(%%ebp), %%e%cx\n", (*offsetMap)[LLVMGetOperand(instruction, 0)], getReg(regNum));
                    }
                }
                if (opcode == LLVMStore){
                    if (param1 == LLVMGetOperand(instruction, 0)){
                        continue;
                    } else if (LLVMIsConstant(LLVMGetOperand(instruction, 0))){
                        int offset = (*offsetMap)[LLVMGetOperand(instruction,1)];
                        fprintf(stdout, "\tmovl\t$%lld, %d(%%ebp)\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 0)), offset);
                    } else {
                        if ((*(*bbReg)[basicBlock])[LLVMGetOperand(instruction, 0)] != -1){
                            int offset = (*offsetMap)[LLVMGetOperand(instruction, 1)];
                            int regNum = (*((*bbReg)[basicBlock]))[LLVMGetOperand(instruction, 0)];
                            fprintf(stdout, "\tmovl\t%%e%cx, %d(%%ebp)\n", getReg(regNum), offset);
                        } else {
                            int c1 = (*offsetMap)[LLVMGetOperand(instruction, 0)];
                            fprintf(stdout, "\tmovl\t%d(%%ebp), %%eax\n", c1);
                            int c2 = (*offsetMap)[LLVMGetOperand(instruction, 1)];
                            fprintf(stdout, "\tmovl\t%%eax, %d(%%ebp)\n", c2);
                        }
                    }
                }
                if (opcode == LLVMCall){
                    fprintf(stdout, "\tpushl\t%%ebx\n");
                    fprintf(stdout, "\tpushl\t%%ecx\n");
                    fprintf(stdout, "\tpushl\t%%edx\n");
                    //LLVMValueRef func = LLVMGetValueName(LLVMGetCalledValue(instruction));
                    //LLVMValueRef param2 = LLVMGetParam(LLVMValueRef func, 0);
                    if (LLVMGetNumOperands(instruction) == 2){
                        LLVMValueRef param2 = LLVMGetOperand(instruction, 0);
                        if (LLVMIsConstant(param2)){
                            fprintf(stdout, "\tpushl\t$%lld\n", LLVMConstIntGetSExtValue(param2));
                        } else {
                            if ((*((*bbReg)[basicBlock]))[param2] != -1){
                                int regNum = (*((*bbReg)[basicBlock]))[param2];
                                fprintf(stdout, "\tpushl\t%%e%cx\n", getReg(regNum));
                            } else {
                                int offset = (*offsetMap)[param2];
                                fprintf(stdout, "\tpushl\t%d(%%ebp)\n", offset);
                            }
                        }
                    }
                    fprintf(stdout, "\tcall\t%s\n", LLVMGetValueName(LLVMGetCalledValue(instruction)));
                    if (LLVMGetNumOperands(instruction) == 2){
                        fprintf(stdout, "\taddl\t$4, %%esp\n");
                    }
                    fprintf(stdout, "\tpopl\t%%edx\n");
                    fprintf(stdout, "\tpopl\t%%ecx\n");
                    fprintf(stdout, "\tpopl\t%%ebx\n");
                    if (LLVMGetNumOperands(instruction) == 1) {
                        if ((*((*bbReg)[basicBlock]))[instruction] != -1) {
                            int regNum = (*((*bbReg)[basicBlock]))[instruction];
                            fprintf(stdout, "\tmovl\t%%eax, %%e%cx\n", getReg(regNum));
                        } else {
                            int k = (*offsetMap)[instruction];
                            fprintf(stdout, "\tmovl\t%%eax, %d(%%ebp)\n", k);
                        }
                    }
                }
                if (opcode == LLVMBr){
                    if (!(LLVMIsConditional(instruction))){
                        //LLVMGetNumOperands(instruction) == 1
                        //LLVMDumpValue(LLVMGetOperand(instruction, 0));
                        string label = (*BBLabels)[LLVMValueAsBasicBlock(LLVMGetOperand(instruction, 0))];
                        fprintf(stdout, "\tjmp\t%s\n", label.c_str());
                    } else {
                        //LLVMDumpValue(LLVMGetOperand(instruction, 1));
                        //LLVMDumpValue(LLVMGetOperand(instruction, 2));
                        // char* label1 = (*BBLabels)[LLVMGetOperand(instruction, 1)];
                        // char* label2 = (*BBLabels)[LLVMGetOperand(instruction, 2)];
                        string label1 = (*BBLabels)[LLVMValueAsBasicBlock(LLVMGetOperand(instruction, 1))];
                        //printf("%s\n", label1.c_str());
                        string label2 = (*BBLabels)[LLVMValueAsBasicBlock(LLVMGetOperand(instruction, 2))];
                       // printf("%s\n", label2.c_str());
                        LLVMIntPredicate pred = LLVMGetICmpPredicate(LLVMGetOperand(instruction, 0));
                        //printf("%u\n", pred);
                        if (pred == LLVMIntEQ){
                            fprintf(stdout, "\tje\t%s\n", label1.c_str());
                        }
                        if (pred == LLVMIntNE){
                            fprintf(stdout, "\tjne\t%s\n", label1.c_str());
                        }
                        if (pred == LLVMIntSGT || pred == LLVMIntUGT){
                            fprintf(stdout, "\tjle\t%s\n", label1.c_str());
                        }
                        if (pred == LLVMIntSGE || pred == LLVMIntUGE){
                            fprintf(stdout, "\tjl\t%s\n", label1.c_str());
                        }
                        if (pred == LLVMIntSLT || pred == LLVMIntULT){
                            fprintf(stdout, "\tjge\t%s\n", label1.c_str());
                        }
                        if (pred == LLVMIntSLE || pred == LLVMIntULE){
                            fprintf(stdout, "\tjg\t%s\n", label1.c_str());
                        }
                        fprintf(stdout, "\tjmp\t%s\n", label2.c_str());
                    }
                }
                if (opcode == LLVMAlloca){
                    continue;
                }
                if (opcode == LLVMAdd || opcode == LLVMICmp || opcode == LLVMMul || opcode == LLVMSub){
                    string* X = new string();
                    //fprintf(stdout, "%s\n", X->c_str());
                    if ((*((*bbReg)[basicBlock]))[instruction] != -1){
                        int regNum = (*((*bbReg)[basicBlock]))[instruction];
                        char reg = getReg(regNum);
                        char endingX = 'x';
                        *X = "%e";
                        //fprintf(stdout, "%s\n", X->c_str());
                        X->push_back(reg);
                        //fprintf(stdout, "%s\n", X->c_str());
                        X->push_back(endingX);
                        //*X = "%e + reg + endingX;
                        //fprintf(stdout, "%s\n", X->c_str());
                        //strncat(X, &reg, 1);
                        //strncat(X, &endingX, 1);
                    } else {
                        *X = "%eax";
                    }
                    //fprintf(stdout, "%s\n", X->c_str());
                    //fflush(stdout);
                    if (LLVMIsConstant(LLVMGetOperand(instruction, 0))){
                        fprintf(stdout, "\tmovl\t$%lld, %s\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 0)), X->c_str());
                    } else {
                        if ((*((*bbReg)[basicBlock]))[LLVMGetOperand(instruction, 0)] != -1){
                            int regNum = (*((*bbReg)[basicBlock]))[LLVMGetOperand(instruction, 0)];
                            char reg = getReg(regNum);
                            char endingX = 'x';
                            string* AReg = new string("%e");
                            //*AReg = *AReg + reg + endingX;
                            AReg->push_back(reg);
                            AReg->push_back(endingX);
                            //strncat(AReg, &reg, 1);
                            //strncat(AReg, &endingX, 1);
                            if (AReg->compare(*X) != 0){
                                //fprintf(stdout, "%s\n", X->c_str());
                                //fflush(stdout);
                                fprintf(stdout, "\tmovl\t%s, %s\n", AReg->c_str(), X->c_str());
                            }
                        } else {
                            int n = (*offsetMap)[LLVMGetOperand(instruction, 0)];
                            //fprintf(stdout, "%s\n", X->c_str());
                            //fflush(stdout);
                            fprintf(stdout, "\tmovl\t%d(%%ebp), %s\n", n, X->c_str());
                        }
                    }
                    //check if second operand is constant
                    if (LLVMIsConstant(LLVMGetOperand(instruction, 1))){
                        if (opcode == LLVMAdd){
                            fprintf(stdout, "\taddl\t$%lld, %s\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 1)), X->c_str());
                        } else if (opcode == LLVMICmp){
                            fprintf(stdout, "\tcmpl\t$%lld, %s\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 1)), X->c_str());
                        } else if (opcode == LLVMSub){
                            fprintf(stdout, "\tsubl\t$%lld, %s\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 1)), X->c_str());
                        } else if (opcode == LLVMMul){
                            fprintf(stdout, "\timull\t$%lld, %s\n", LLVMConstIntGetSExtValue(LLVMGetOperand(instruction, 1)), X->c_str());
                        }
                    } else {
                        if ((*((*bbReg)[basicBlock]))[LLVMGetOperand(instruction, 1)] != -1){
                            int regNum = (*((*bbReg)[basicBlock]))[LLVMGetOperand(instruction, 1)];
                            if (opcode == LLVMAdd){
                                fprintf(stdout, "\taddl\t%%e%cx, %s\n", getReg(regNum), X->c_str());
                            } else if (opcode == LLVMICmp){
                                fprintf(stdout, "\tcmpl\t%%e%cx, %s\n", getReg(regNum), X->c_str());
                            } else if (opcode == LLVMSub){
                                fprintf(stdout, "\tsubl\t%%e%cx, %s\n", getReg(regNum), X->c_str());
                            } else if (opcode == LLVMMul){
                                fprintf(stdout, "\timull\t%%e%cx, %s\n", getReg(regNum), X->c_str());
                            }
                        } else {
                            int off = (*offsetMap)[LLVMGetOperand(instruction, 1)];
                            if (opcode == LLVMAdd){
                                fprintf(stdout, "\taddl\t%d(%%ebp), %s\n", off, X->c_str());
                            } else if (opcode == LLVMICmp){
                                fprintf(stdout, "\tcmpl\t%d(%%ebp), %s\n", off, X->c_str());
                            } else if (opcode == LLVMSub){
                                fprintf(stdout, "\tsubl\t%d(%%ebp), %s\n", off, X->c_str());
                            } else if (opcode == LLVMMul){
                                fprintf(stdout, "\timull\t%d(%%ebp), %s\n", off, X->c_str());
                            }
                        }
                    }
                    if ((*((*bbReg)[basicBlock]))[instruction] == -1){
                        int off = (*offsetMap)[instruction];
                        fprintf(stdout, "\tmovl\t%%eax, %d(%%ebp)\n", off);
                    }

                }
            }
        }
    //}
}

int runCodegen(LLVMModuleRef m){
    unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>();
    unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
    unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
    for (LLVMValueRef function =  LLVMGetFirstFunction(m); function; function = LLVMGetNextFunction(function)) {

        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
            compute_liveness(basicBlock, lively, num_uses);
            //print_lively((*lively)[basicBlock]);
        }
        reg_allocate(function, lively, bbReg, num_uses);
        // for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
        //     print_umap((*bbReg)[basicBlock]);
        // }

    }
    codegen(m, bbReg);

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
    return 1;
}

// void runna(LLVMValueRef function){
// int main(int argc, char** argv){
// 	LLVMModuleRef m = NULL;
//     setvbuf(stdout, NULL, _IONBF, 0);
// 	if (argc == 2){
// 		char *err = 0;

//         LLVMMemoryBufferRef ll_f = 0;

//         LLVMCreateMemoryBufferWithContentsOfFile(argv[1], &ll_f, &err);

//         if (err != NULL) { 
//             printf("error!");
//             return 0;
//         }
        
//         LLVMParseIRInContext(LLVMGetGlobalContext(), ll_f, &m, &err);
//         if (m == NULL){
//             printf("module is NULL\n");
//         }
//         // LLVMDumpModule(m);
//         if (err != NULL) {
//             printf("error!");
//         }
//         unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>* lively = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>>*>();
//         unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
//         unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
//         for (LLVMValueRef function =  LLVMGetFirstFunction(m); function; function = LLVMGetNextFunction(function)) {

//             // unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>* lively = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, pair<int, int>*>*>();
//             // unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* bbReg = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();
//             // unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>* num_uses = new unordered_map<LLVMBasicBlockRef, unordered_map<LLVMValueRef, int>*>();

//             for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
//                 compute_liveness(basicBlock, lively, num_uses);
//                 //print_lively((*lively)[basicBlock]);
//             }
//             reg_allocate(function, lively, bbReg, num_uses);
//             // for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
//             //     print_umap((*bbReg)[basicBlock]);
//             // }

//         }
//         codegen(m, bbReg);

//         for (auto level1 : (*lively)){
//             //level1.second->clear();
//             // for (auto level2 : *level1.second){
//             //     delete(level2.second);
//             // }
//             delete(level1.second);
//         }
//         //lively->clear();
//         delete(lively);
//         for (auto level1 : (*bbReg)){
//             delete(level1.second);
//             //level1.second->clear();
//         }
//         //bbReg->clear();
//         delete(bbReg);
//         for (auto level1 : (*num_uses)){
//             //level1.second->clear();
//             delete(level1.second);
//         }
//         //num_uses->clear();
//         delete(num_uses);
//         // LLVMDisposeModule(m);
//         // LLVMDisposeMemoryBuffer(ll_f);
// 	}
// }