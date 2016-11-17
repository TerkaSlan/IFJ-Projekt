//
// Created by Charvin on 14. 11. 2016.
//
#include "test.h"
#include "../interpret.h"
#include "../ial.h"
#include "../instruction.h"
#include "../constants.h"

TEST_SUITE_START(InterpretTEST)

	tHashTablePtr globalTable = htabInit(13);
	tHashTablePtr MainTable = htabInit(31);

	tSymbolPtr sym = symbolNew();
	sym->Type = eCLASS;
	sym->Const = true;
	sym->Defined = true;
	sym->Name = strNewFromCStr("Main");
	sym->Data.ClassData.LocalSymbolTable = MainTable;

	tSymbolPtr mainClass = htabAddSymbol(globalTable, sym, true);
	SHOULD_NOT_EQUAL("Alloc ok", mainClass, NULL);
	SHOULD_EQUAL("Add fine", sym->Type, eCLASS);
	SHOULD_EQUAL("Add fine", sym->Const, true);
	SHOULD_EQUAL("Add fine", sym->Defined, true);
	SHOULD_EQUAL("Add fine", sym->Data.ClassData.LocalSymbolTable, MainTable);

	sym->Type = eINT;
	sym->Const = true;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "Static7");
	sym->Data.Integer = 7;

	tSymbolPtr Static7 = htabAddSymbol(MainTable, sym, true);
	SHOULD_EQUAL("Add fine", sym->Data.Integer, 7);


	sym->Type = eSTRING;
	sym->Const = true;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "StaticASD");
	sym->Data.String = strNewFromCStr("ASD");

	tSymbolPtr StaticASD = htabAddSymbol(MainTable, sym, true);
	SHOULD_EQUAL("Add fine", strcmp(strGetCStr(sym->Data.String), "ASD"), 0);


	tHashTablePtr RunTable = htabInit(31);


	sym->Type = eFUNCTION;
	sym->Const = true;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "run");
	sym->Data.FunctionData.ReturnType = eNULL;
	sym->Data.FunctionData.ArgumentList = NULL;
	sym->Data.FunctionData.InstructionIndex = 1;
	sym->Data.FunctionData.LocalSymbolTable = RunTable;

	tSymbolPtr Run = htabAddSymbol(MainTable, sym, true);
	RunTable->Parent = Run;

	tHashTablePtr CatTable = htabInit(31);

	sym->Type = eFUNCTION;
	sym->Const = true;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "Cat");
	sym->Data.FunctionData.ReturnType = eSTRING;
	sym->Data.FunctionData.InstructionIndex = 13;
	sym->Data.FunctionData.ArgumentList = NULL;
	sym->Data.FunctionData.LocalSymbolTable = CatTable;

	tSymbolPtr Cat = htabAddSymbol(MainTable, sym, true);
	CatTable->Parent = Cat;

	sym->Type = eINT;
	sym->Const = false;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "x");
	sym->Data.Integer = 1337;

	tSymbolPtr x = htabAddSymbol(RunTable, sym, true);

	sym->Type = eINT;
	sym->Const = false;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "y");
	sym->Data.Integer = 42;

	tSymbolPtr y = htabAddSymbol(RunTable, sym, true);

	sym->Type = eSTRING;
	sym->Const = false;
	sym->Defined = false;
	strClear(sym->Name);
	strAddCStr(sym->Name, ":tmpStr");
	sym->Data.String = NULL;


	tSymbolPtr tmp = htabAddSymbol(RunTable, sym, true);

	sym->Type = eSTRING;
	sym->Const = false;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "str");
	sym->Data.String = NULL;

	tSymbolPtr CatLocalStr = htabAddSymbol(CatTable, sym, true);

	sym->Type = eSTRING;
	sym->Const = false;
	sym->Defined = true;
	strClear(sym->Name);
	strAddCStr(sym->Name, "str2");
	sym->Data.String = NULL;

	tSymbolPtr CatLocalStr2 = htabAddSymbol(CatTable, sym, true);


	tConstContainerPtr constants = constNew();

	sym->Type = eINT;
	sym->Const = true;
	sym->Defined = true;
	sym->Data.Integer = 1337;
	sym->Name = NULL;

	tSymbolPtr leetconst = constInsertSymbol(constants, *sym);

	sym->Type = eINT;
	sym->Const = true;
	sym->Data.Integer = 42;

	tSymbolPtr hellconst = constInsertSymbol(constants, *sym);



	tInstructionListPtr  list = instrListNew();

	//MOV FUNC
	{tInstruction instr = {iMOV, x, leetconst, NULL}; instrListInsertInstruction(list, instr);}

	tInstructionPtr instrPtr = instrListGetInstruction(list, 1);
	SHOULD_EQUAL("Get active instruction", instrPtr->type, iMOV);
	SHOULD_EQUAL("get instruciton argument", instrPtr->dst, x);
	SHOULD_EQUAL("get instruciton argument", instrPtr->arg1, leetconst);
	SHOULD_NOT_EQUAL("get instruciton argument", instrPtr->arg1, NULL);
	SHOULD_EQUAL("get instruciton argument", instrPtr->arg2, NULL);

	{tInstruction instr = {iMOV, y, hellconst, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iSUB, x, x, Static7}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iADD, x, x, y}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iADD, Static7, Static7, y}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iCONV2STR, tmp, x, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iPRINT, NULL, tmp, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iFRAME, Cat, NULL, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iCALL, NULL, NULL, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iGETRETVAL, tmp, NULL, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iPRINT, NULL , tmp, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iRET, NULL, NULL, NULL}; instrListInsertInstruction(list, instr);}
	//CAT FUNC
	{tInstruction instr = {iMOV, CatLocalStr, StaticASD, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iCONV2STR, CatLocalStr2, Static7, NULL}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iADD, CatLocalStr, CatLocalStr, CatLocalStr2}; instrListInsertInstruction(list, instr);}
	{tInstruction instr = {iRET, NULL , CatLocalStr, NULL}; instrListInsertInstruction(list, instr);}

	instrListSetFirstInstruction(list ,1);
	Interpret(globalTable, list);

	htabFree(globalTable);
	htabFree(MainTable);
	htabFree(CatTable);
	htabFree(RunTable);
	constFree(constants);
	instrListFree(list);


TEST_SUITE_END