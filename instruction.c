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

	//add iStop instruction on index 0
	tInstruction stop = {iSTOP, NULL, NULL, NULL};
	if(instrListInsertInstruction(ret, stop) != 0) {
		free(ret->instructionArray);
		free(ret);
		return NULL;
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
int64_t instrListGetActiveInstructionIndex(tInstructionListPtr list)
{
	if(!list)
		return -2;
	return list->activeInstruction;
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
	list->activeInstruction = ((int64_t) index) - 1;

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
