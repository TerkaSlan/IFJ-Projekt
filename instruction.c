//
// Created by Charvin on 16. 10. 2016.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "instruction.h"


#define INSTRUCTION_LIST_DEFAULT_SIZE 4096


tInstructionListPtr instrListNew() {
	tInstructionListPtr ret = malloc(sizeof(tInstructionList));
	if(!ret)
		return NULL;

	//allocate array of instructions
	ret->instructionArray = calloc(INSTRUCTION_LIST_DEFAULT_SIZE, sizeof(tInstruction));
	if(!ret->instructionArray) {
		free(ret);
		return NULL;
	}

	//set to default values
	ret->activeInstruction = -1;
	ret->firstInstruction  = -1;
	ret->usedSize          = 0;
	ret->allocatedSize     = INSTRUCTION_LIST_DEFAULT_SIZE;

	//add iStop instruction on index 0
	tInstruction stop = {iSTOP, NULL, NULL, NULL};
	if(instrListInsertInstruction(ret, stop) != 0) {
		free(ret->instructionArray);
		free(ret);
	}

	return ret;

}

tInstructionPtr instrListGetNextInstruction(tInstructionListPtr list) {
	if(!list || list->firstInstruction <
	            0) //not checking for array != null, interpreting a corrupted instruction list should not happen
		return NULL;

	//increases active instruction index and returns pointer to the instruction
	return &(list->instructionArray[++(list->activeInstruction)]);
}


tInstructionPtr instrListGetActiveInstruction(tInstructionListPtr list) {
	if(!list || list->activeInstruction < 0 || !list->instructionArray)
		return NULL;

	return &(list->instructionArray[list->activeInstruction]);
}


int64_t instrListInsertInstruction(tInstructionListPtr list, tInstruction instr) {
	if(!list || !list->instructionArray)
		return -1;

	//add instruction
	int64_t index = list->usedSize;

	memcpy(&(list->instructionArray[index]), &instr, sizeof(tInstruction));


	//check if still in the array - if not, realloc
	if(++(list->usedSize) >= list->allocatedSize) {
		uint32_t        newAllocSize        = list->allocatedSize * 2;
		tInstructionPtr newInstructionArray = realloc(list->instructionArray, sizeof(tInstruction) * newAllocSize);
		if(!newInstructionArray)
			return -1;

		list->instructionArray = newInstructionArray;
		list->allocatedSize    = newAllocSize;
	}

	return index;
}

void instrListSetFirstInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->allocatedSize)
		return;

	list->firstInstruction  = (int64_t) index;
	list->activeInstruction = (int64_t) index - 1;

}


tInstructionPtr instrListGetInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->usedSize || !list->instructionArray)
		return NULL;

	return &(list->instructionArray[index]);
}


void instrListGoto(tInstructionListPtr list, uint32_t instructionIndex) {
	if(!list)
		return;

	list->activeInstruction = (int64_t) instructionIndex - 1;
}

void instrListFree(tInstructionListPtr list) {
	free(list->instructionArray);
	free(list);

}

void instrListPrint(tInstructionListPtr list) {
	if (!list) {
		printf("Pointer to instruction list is NULL\n");
		return;
	}
	
	char *types[] = {"iSTOP", "iMOV", "iFRAME", "iPUSH", "iCALL", "iRET", "iGETRETVAL", "iINC", "iDEC", "iADD", "iSUB", "iMUL", "iDIV", "iNEG", "iLE", "iLT", "iGE", "iGT", "iEQ", "iNEQ", "iLAND", "iLOR", "iLNOT", "iGOTO", "iIFGOTO", "iIFNGOTO", "iCONV2STR", "iCONV2INT", "iCONV2BOOL", "iCONV2DOUBLE", "iPRINT", "iREAD", "iLEN", "iCOMPARE", "iFIND", "iSORT", "iSUBSTR", "Unknown instr"};

	uint32_t index = 0;
	printf(" _____________________________________________________________________________ \n");
	printf("|    No.        Instr             Dst               Arg1              Arg2    |\n");
	printf("|-----------------------------------------------------------------------------|\n");
	while (index < list->usedSize) {
		printf("|%4d .: ", index);
		printf("%13s ", types[list->instructionArray[index].type]);
		printf("%17p ", list->instructionArray[index].dst);
		printf("%17p ", list->instructionArray[index].arg1);
		printf("%17p  |\n", list->instructionArray[index].arg2);
		printf("|-----------------------------------------------------------------------------|\n");
		index++;
	}
}