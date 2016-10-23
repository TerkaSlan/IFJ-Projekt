//
// Created by Charvin on 23. 10. 2016.
//

#include "test.h"
#include "../instruction.h"


TEST_SUITE_START(InstructionListTest)

	tInstructionListPtr iList = instrListNew();
	SHOULD_NOT_EQUAL("Creation", iList, NULL);

	SHOULD_EQUAL("Next instruction on empty list", instrListGetNextInstruction(iList), NULL);
	SHOULD_EQUAL("Active instruction on empty list", instrListGetActiveInstruction(iList), NULL);

	tInstruction instr = {iLABEL, NULL, NULL, NULL};
	instrListInsertInstruction(iList, instr);
	instr.type = iADD;
	instrListInsertInstruction(iList, instr);
	instr.type = iXOR;
	instrListInsertInstruction(iList, instr);
	instr.type = iAND;

	for(int i = 0; i < 10; i++)
		instrListInsertInstruction(iList, instr);

	instr.type = iSTOP;
	instrListInsertInstruction(iList, instr);

	tInstructionPtr instrPtr = instrListGetInstruction(iList, 13);
	SHOULD_NOT_EQUAL("Get instruction by index", instrPtr, NULL);
	SHOULD_EQUAL("Get instruction by index", instrPtr->type, iSTOP);

	SHOULD_EQUAL("get first instruction when not active", instrListGetActiveInstruction(iList), NULL);
	SHOULD_EQUAL("get next instruction when not active", instrListGetNextInstruction(iList), NULL);
	instrListSetFirstInstruction(iList, 0);

	SHOULD_NOT_EQUAL("get active instruction", instrListGetActiveInstruction(iList), NULL);
	SHOULD_NOT_EQUAL("get next instruction", instrListGetNextInstruction(iList), NULL);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction", instrPtr->type, iXOR);

	instrPtr = instrListGetActiveInstruction(iList);
	SHOULD_NOT_EQUAL("Get active instruction", instrPtr, NULL);
	SHOULD_EQUAL("Get active instruction", instrPtr->type, iXOR);

	instrListGoto(iList, 0);
	instrPtr = instrListGetActiveInstruction(iList);
	SHOULD_NOT_EQUAL("Get active instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get active instruction after goto", instrPtr->type, iLABEL);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iADD);

	instrListFree(iList);

TEST_SUITE_END