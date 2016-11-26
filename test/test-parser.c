//
// Created by Terka on 25.11.2016
//

#include "test.h"
#include "../parser.h"
#include "../parser_second.h"
#include "../interpret.h"
#include "../scanner.h"
#include "../error.h"
TEST_SUITE_START(ParserTest);

extern tHashTablePtr globalScopeTable;
extern tInstructionListPtr instructionList;
extern tInstructionListPtr preInstructionList;

static char* enumStrings[] = {"ERR_OK", "ERR_LEX", "ERR_SYNTAX", "ERR_SEM", "ERR_SEM_TYPE",
															"No 5th error", "ERR_SEM_OTHER", "ERR_RUN_INPUT", "ERR_RUN_UNINITIALIZED",
															"ERR_RUN_ZERODIV", "ERR_OTHER"};

void testParser(char *sourceFile, char *message, eError expectedError, int testNumber){
	printf("\nTest n. %d: '%s' expecting error: %d (%s)\n", testNumber, message, expectedError, enumStrings[expectedError]);
	openFile(sourceFile);
	eError errCode = fillSymbolTable();
	SHOULD_EQUAL("1. Run of parsing errCode", errCode, expectedError);
	if (errCode == ERR_OK){
		errCode = generateInstructions();
		SHOULD_EQUAL("2. Run of parsing errCode", errCode, expectedError);
	}
	if (errCode == ERR_OK){
		instrListPrint(preInstructionList);
		Interpret(globalScopeTable, preInstructionList);
		SHOULD_EQUAL("Preinterpretation errCode", errCode, expectedError);
	}
	if (errCode == ERR_OK){
		instrListPrint(instructionList);
		Interpret(globalScopeTable, instructionList);
		SHOULD_EQUAL("Interpretation errCode", errCode, expectedError);
	}
}

testParser("0", "Empty file", ERR_SEM, 0);
htabClear(globalScopeTable);
testParser("1", "Most minimalistic program possible", ERR_OK, 1);
htabClear(globalScopeTable);
testParser("2", "Id confict - function and variable in another function", ERR_SEM, 2);
htabClear(globalScopeTable);
testParser("3", "Static variable initialization - use of preinterpretation", ERR_OK, 3);
htabClear(globalScopeTable);
testParser("4", "2 classes, 2 functions", ERR_OK, 4);
closeFile();

TEST_SUITE_END
