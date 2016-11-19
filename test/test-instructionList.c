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

	tInstruction instr = {iLE, NULL, NULL, NULL};
	instrListInsertInstruction(iList, instr); //iSTOP, iLE
	instr.type = iADD;
	instr.dst = (void*)1;
	instr.arg1 = (void*)2;
	instr.arg2 = (void*)3;
	instrListInsertInstruction(iList, instr); //iSTOP, iLE, iADD
	instr.type = iMUL;
	instrListInsertInstruction(iList, instr); //iSTOP, iLE, iADD, iMUL

	for(int i = 0; i < 10; i++)
		instrListInsertInstruction(iList, instr); //iSTOP, iLE, iADD, iMUL, 10xiMUL

	instr.type = iSTOP;
	instrListInsertInstruction(iList, instr); //iSTOP, iLE, iADD, iMUL, 10xiMUL, iSTOP

	tInstructionPtr instrPtr = instrListGetInstruction(iList, 13);
	SHOULD_NOT_EQUAL("Get instruction by index", instrPtr, NULL);
	SHOULD_EQUAL("Get instruction by index", instrPtr->type, iMUL);

	SHOULD_EQUAL("get first instruction when not active", instrListGetActiveInstruction(iList), NULL);
	SHOULD_EQUAL("get next instruction when not active", instrListGetNextInstruction(iList), NULL);
	instrListSetFirstInstruction(iList, 0);

	SHOULD_EQUAL("get active instruction", instrListGetActiveInstruction(iList), NULL);
	SHOULD_NOT_EQUAL("get next instruction", instrListGetNextInstruction(iList), NULL);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction", instrPtr->type, iLE);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get active instruction", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction", instrPtr->type, iADD);
	SHOULD_EQUAL("get instruciton argument", instrPtr->dst, (void*)1);
	SHOULD_EQUAL("get instruciton argument", instrPtr->arg1, (void*)2);
	SHOULD_EQUAL("get instruciton argument", instrPtr->arg2, (void*)3);

	instrListGoto(iList, 1);
	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get active instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get active instruction after goto", instrPtr->type, iLE);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iADD);

	for(int i = 0; i < 11; i++) {
		instrPtr = instrListGetNextInstruction(iList);
		SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
		SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iMUL);
	}

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iSTOP);

	instrListGoto(iList, 2);

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iADD);

	for(int i = 0; i < 11; i++) {
		instrPtr = instrListGetNextInstruction(iList);
		SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
		SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iMUL);
	}

	instrPtr = instrListGetNextInstruction(iList);
	SHOULD_NOT_EQUAL("Get next instruction after goto", instrPtr, NULL);
	SHOULD_EQUAL("Get next instruction after goto", instrPtr->type, iSTOP);

	instrListFree(iList);

TEST_SUITE_END
