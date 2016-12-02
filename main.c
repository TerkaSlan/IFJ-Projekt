/*
 * Project: IFJ
 * Implementace interpretu imperativního jazyka IFJ16.
 *
 * Description:
 * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2016.pdf
 *
 * Team:
 * Michal Charvát          (xcharv16)
 * Terézia Slanináková     (xslani06)
 * Katarína Grešová        (xgreso00)
 * Marek Schauer           (xschau00)
 * Jakub Handzuš           (xhandz00)
 */

#include <stdio.h>
#include "parser_second.h"
#include "parser.h"
#include "scanner.h"
#include "interpret.h"
#include "ial.h"

static tHashTablePtr helperSymbolTable;
tHashTablePtr globalScopeTable;
tInstructionListPtr instructionList;
tInstructionListPtr preInstructionList;
tConstContainerPtr constTable;


#define CHECK_ERRCODE(jmpto) if(errCode != ERR_OK) goto jmpto
#define INIT_ERRJUMP(jmpto) do{printError(ERR_INTERN, "Error allocating new space. Out of memory.\n"); errCode = ERR_INTERN; goto jmpto;}while(0)

static eError initializeGlobalVariables(){
	eError errCode = ERR_OK;

	if((globalScopeTable = htabInit(HTAB_DEFAULT_SIZE)) == NULL)
        INIT_ERRJUMP(lFailedGlobalTableInit);

	if((constTable = constNew()) == NULL)
        INIT_ERRJUMP(lFailedConstTableInit);

	if((preInstructionList = instrListNew()) == NULL)
		INIT_ERRJUMP(lFailedPreInstructionListInit);

	if ((instructionList = instrListNew()) == NULL)
		INIT_ERRJUMP(lFailedInstructionListInit);


	//create fake main and run
	if((helperSymbolTable = htabInit(3)) == NULL)
		INIT_ERRJUMP(lFailedHelperSymbolTableInit);

	tHashTablePtr mainClassTable;
	if((mainClassTable = htabInit(3)) == NULL)
		INIT_ERRJUMP(lFailedHelperClassTableInit);

	tSymbolPtr symbol;
	if((symbol = symbolNew()) == NULL)
		INIT_ERRJUMP(lFailedSymbolInit);

	symbol->Type = eCLASS;
	symbol->Const = true;
	symbol->Defined = true;
	symbol->Data.ClassData.LocalSymbolTable = mainClassTable;
	if((symbol->Name = strNewFromCStr("Main")) == NULL)
		INIT_ERRJUMP(lFailedClassNameInit);


	if((mainClassTable->Parent = htabAddSymbol(helperSymbolTable, symbol, false)) == NULL)
		INIT_ERRJUMP(lFailedClassNameInit);

	tHashTablePtr runFunctionTable;
	if((runFunctionTable = htabInit(3)) == NULL)
		INIT_ERRJUMP(lFailedClassNameInit);

	symbol->Type = eFUNCTION;
	symbol->Data.FunctionData.ArgumentList = NULL;
	symbol->Data.FunctionData.InstructionIndex = 1;
	symbol->Data.FunctionData.NumberOfArguments = 0;
	symbol->Data.FunctionData.ReturnType = eNULL;
	symbol->Data.FunctionData.LocalSymbolTable = runFunctionTable;
	strClear(symbol->Name);
	if(strAddCStr(symbol->Name, "run") == STR_ERROR)
		INIT_ERRJUMP(lFailedNameReSet);

	if((runFunctionTable->Parent = htabAddSymbol(mainClassTable, symbol, false)) == NULL)
		INIT_ERRJUMP(lFailedNameReSet);

	symbolFree(symbol);
	return ERR_OK;

lFailedNameReSet:
	htabFree(runFunctionTable);
lFailedClassNameInit:
	symbolFree(symbol);
lFailedSymbolInit:
	htabFree(mainClassTable);
lFailedHelperClassTableInit:
	htabFree(helperSymbolTable);
lFailedHelperSymbolTableInit:
	instrListFree(instructionList);
lFailedInstructionListInit:
	instrListFree(preInstructionList);
lFailedPreInstructionListInit:
	constFree(constTable);
lFailedConstTableInit:
	htabFree(globalScopeTable);
lFailedGlobalTableInit:
  return errCode;
}

static void freeGlobalVariables(){
	constFree(constTable);
	htabRecursiveFree(globalScopeTable);
	htabRecursiveFree(helperSymbolTable);
	instrListFree(instructionList);
	instrListFree(preInstructionList);
}

int main(int argc, char const *argv[]) {
	if(argc < 2) {
		printError(ERR_INTERN, "Parameter with filename is required for the interpret to run.\n");
		return ERR_INTERN;
	}

	eError errCode;
	errCode = openFile(argv[1]);
	CHECK_ERRCODE(lOpenFileFailed);

	errCode = initializeGlobalVariables();
	CHECK_ERRCODE(lInitializationFailed);

	errCode = fillSymbolTable();
	CHECK_ERRCODE(lParserFailed);

	errCode = generateInstructions();
	CHECK_ERRCODE(lParserFailed);

	errCode = Interpret(globalScopeTable, preInstructionList);
	CHECK_ERRCODE(lParserFailed);

	errCode = Interpret(globalScopeTable, instructionList);

lParserFailed:
	freeGlobalVariables();

lInitializationFailed:
	closeFile();

lOpenFileFailed:
	return (int)errCode;
}
