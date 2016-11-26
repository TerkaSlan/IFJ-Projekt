#include <stdio.h>
#include "parser_second.h"
#include "parser.h"
#include "scanner.h"
#include "interpret.h"

tHashTablePtr globalScopeTable;
tInstructionListPtr instructionList;
tInstructionListPtr preInstructionList;
tConstContainerPtr constTable;


#define CHECK_ERRCODE(jmpto) if(errCode != ERR_OK) goto jmpto

static eError initializeGlobalVariables(){
  if((globalScopeTable = htabInit(HTAB_DEFAULT_SIZE)) == NULL){
    return ERR_INTERN;
  }
  if((constTable = constNew()) == NULL) {
    htabFree(globalScopeTable);
    return ERR_INTERN;
  }
  if ((instructionList = instrListNew()) == NULL){
    htabFree(globalScopeTable);
    constFree(constTable);
    return ERR_INTERN;
  }
  return ERR_OK;
}

static void freeGlobalVariables(){
  constFree(constTable);
  htabRecursiveFree(globalScopeTable);
  instrListFree(instructionList);
  instrListFree(preInstructionList);
}

int main(int argc, char const *argv[]) {
	if(argc < 2) {
		printError(ERR_OTHER, "Parameter with filename is required for the interpret to run.\n");
		return ERR_OTHER;
	}

	eError errCode;
	errCode = openFile(argv[1]);
	CHECK_ERRCODE(lOpenFileFailed);

	errCode = initializeGlobalVariables();
	CHECK_ERRCODE(lInitializationFailed);

	errCode = fillSymbolTable();
	CHECK_ERRCODE(lParserFailed);
	printf("Retcode 1: %d \n", errCode);

	errCode = generateInstructions();
	CHECK_ERRCODE(lParserFailed);
	printf("Retcode 2: %d \n", errCode);

	Interpret(globalScopeTable, preInstructionList);
	CHECK_ERRCODE(lParserFailed);

	Interpret(globalScopeTable, instructionList);

lParserFailed:
	freeGlobalVariables();

lInitializationFailed:
	closeFile();

lOpenFileFailed:
	return (int)errCode;
}
