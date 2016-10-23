//
// Created by Charvin on 16. 10. 2016.
//

#include <stdlib.h>
#include <string.h>
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

	return ret;

}

tInstructionPtr instrListGetNextInstruction(tInstructionListPtr list) {
	if(!list)
		return NULL;

	if(list->activeInstruction < 0) {
		if(list->firstInstruction < 0)
			return NULL;

		//set active instruction to the first instruction
		list->activeInstruction = list->firstInstruction - 1;
	}

	return &(list->instructionArray[++(list->activeInstruction)]);
}


tInstructionPtr instrListGetActiveInstruction(tInstructionListPtr list) {
	if(!list || list->activeInstruction < 0)
		return NULL;

	return &(list->instructionArray[list->activeInstruction]);
}


tInstructionPtr instrListInsertInstruction(tInstructionListPtr list, tInstruction instr) {
	if(!list)
		return NULL;

	//add instruction
	tInstructionPtr ret = &(list->instructionArray[list->usedSize]);
	memcpy(ret, &instr, sizeof(tInstruction));


	//check if still in the array - if not, realloc
	if(++(list->usedSize) >= list->allocatedSize) {
		uint32_t        newAllocSize        = list->allocatedSize * 2;
		tInstructionPtr newInstructionArray = realloc(list->instructionArray, sizeof(tInstruction) * newAllocSize);
		if(!newInstructionArray)
			return NULL;

		list->instructionArray = newInstructionArray;
		list->allocatedSize    = newAllocSize;
	}

	return ret;
}

void instrListSetFirstInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->allocatedSize)
		return;

	list->firstInstruction  = (int64_t) index;
	list->activeInstruction = (int64_t) index;

}


tInstructionPtr instrListGetInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->usedSize)
		return NULL;

	return &(list->instructionArray[index]);
}



void instrListGoto(tInstructionListPtr list, uint32_t instructionIndex) {
	if(!list)
		return;

	list->activeInstruction = (int64_t) instructionIndex;
}

void instrListFree(tInstructionListPtr list) {
	free(list->instructionArray);
	free(list);

}