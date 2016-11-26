//
// Created by Charvin on 23. 11. 2016.
//
#include "../ial.h"
#include "../instruction.h"
#include "../constants.h"
#include "../interpret.h"
#include <stdlib.h>
#include "stdbool.h"

#define createClass(name, output)                                                    \
 do {		                                                                     \
 	tHashTablePtr newTable = htabInit(31);                      \
  tSymbolPtr addedClass;\
 	tSymbolPtr classSymbol = symbolNew();											                 \
   classSymbol->Type = eCLASS;																                 \
   classSymbol->Const = true;																                 \
   classSymbol->Defined = true;															                 \
   classSymbol->Name = name;						                       \
   classSymbol->Next = NULL;						                                       \
   classSymbol->Data.ClassData.LocalSymbolTable = newTable;				                                 \
 	if ((addedClass = htabAddSymbol(globalScopeTable, classSymbol, false)) == NULL)            \
 		abort();\
 output = addedClass;\
newTable->Parent = addedClass;\
symbolFree(classSymbol);                                               \
 } while (0)
// TODO: ifj16 trieda nemoze byt



#define createFunction(type, defined, name, output)		                           \
 do{																										\
 	tHashTablePtr newTable = htabInit(31);\
 	tSymbolPtr functionSymbol = symbolNew();											\
  tSymbolPtr addedFunc;\
 	functionSymbol->Type = eFUNCTION;																\
 	functionSymbol->Name = name;						\
   functionSymbol->Defined = defined;\
 	functionSymbol->Data.FunctionData.ArgumentList = NULL;\
 	functionSymbol->Data.FunctionData.NumberOfArguments = 0;\
 	functionSymbol->Data.FunctionData.ReturnType = type;\
 	functionSymbol->Data.FunctionData.InstructionIndex = 0;\
 	functionSymbol->Data.FunctionData.LocalSymbolTable = newTable;\
  if ((addedFunc = htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, functionSymbol, false)) == NULL) \
	abort(); \
output = addedFunc;\
newTable->Parent = addedFunc;\
symbolFree(functionSymbol);\
 } while (0)

/*
 * Creates a new static variable symbol and stores it table of static variables and functions
 */
#define createStaticVariable(type, defined, name, output)		\
 do{																										\
 	tSymbolPtr currentVariable = symbolNew();						\
 	currentVariable->Type = type;																\
 	currentVariable->Const = true;																\
 	currentVariable->Defined = defined;															\
 	currentVariable->Name = name;\
 	if ((output = htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, currentVariable, false)) == NULL)\
		abort();\
symbolFree(currentVariable);\
 } while (0)


#define createConstant(type, value, output)		\
 do{																										\
 	tSymbolPtr currentVariable = symbolNew();						\
 	currentVariable->Type = type;																\
 	currentVariable->Const = true;																\
 	currentVariable->Defined = true;															\
 	currentVariable->Name = NULL;\
	switch(type){\
	case eINT:\
		currentVariable->Data.Integer = (long)value;break;\
	case eDOUBLE:\
		currentVariable->Data.Double = (double)value;break;\
	case eBOOL:\
		currentVariable->Data.Bool = (bool)value; break;\
	case eSTRING:\
		currentVariable->Data.String = (void*)value;break;\
default: break;\
}\
 	if ((output = constInsertSymbol(constContainer, *currentVariable)) == NULL)\
		abort();\
symbolFree(currentVariable);\
 } while (0)


/*
 * Creates a new local variable or parameter symbol and stores it table of function variables
 */
#define createFunctionVariable(type, defined, name, isArgument, output)                                        \
 do{                                                                                                     \
 	tSymbolPtr currentVariable = symbolNew();                                                             \
  tSymbolPtr addedVar;\
 	currentVariable->Type = type;																                                          \
 	currentVariable->Const = false;																                                        \
 	currentVariable->Defined = defined;															                                      \
 	currentVariable->Name = name;                                                                         \
 	if ((addedVar = htabAddSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, currentVariable, false)) == NULL)\
      abort();                                                   \
  if (isArgument)                                                                                      \
    symbolFuncAddArgument(currentFunction, addedVar);\
  output = addedVar;     \
symbolFree(currentVariable);                                  \
} while (0)

#define AI(type, dst, arg1, arg2) do{ tInstruction instr = {type, dst, arg1, arg2}; instrListInsertInstruction(list, instr); }while(0)


int main ()
{
	tHashTablePtr globalScopeTable = htabInit(13);
	tInstructionListPtr list = instrListNew();
	tConstContainerPtr  constContainer = constNew();

	tSymbolPtr currentFunction;
	tSymbolPtr currentClass;
	dtStrPtr string = strNewFromCStr("Main");
	createClass(string, currentClass);
	
	string = strNewFromCStr("static7");
	tSymbolPtr static7;
	createStaticVariable(eINT, true, string, static7);
	static7->Data.Integer = 7;
	
	string = strNewFromCStr("static666str");
	tSymbolPtr static666str;
	createStaticVariable(eSTRING, true, string, static666str);
	static666str->Data.String = strNewFromCStr("static666str");

	
	string = strNewFromCStr("run");
	createFunction(eNULL, true, string, currentFunction);
	currentFunction->Data.FunctionData.InstructionIndex = 1;

	string = strNewFromCStr(":tmp");
	tSymbolPtr runLocaltmpStr;
	createFunctionVariable(eSTRING, true, string, false, runLocaltmpStr);
	
	string = strNewFromCStr(":tmp2");
	tSymbolPtr runLocaltmpStr2;
	createFunctionVariable(eSTRING, true, string, false, runLocaltmpStr2);
	
	string = strNewFromCStr(":tmpBool");
	tSymbolPtr runLocaltmpBool;
	createFunctionVariable(eBOOL, true, string, false, runLocaltmpBool);


	tSymbolPtr constStr49;
	createConstant(eINT, 49, constStr49);

	
	string = strNewFromCStr("foo1");
	createFunction(eSTRING, true, string, currentFunction);
	tSymbolPtr foo1 =  currentFunction;
	
	string = strNewFromCStr(":foo1tmpStr");
	tSymbolPtr foo1LocaltmpStr3;
	createFunctionVariable(eSTRING, true, string, false, foo1LocaltmpStr3);




	
	string = strNewFromCStr("foo2");
	createFunction(eBOOL, true, string, currentFunction);
	tSymbolPtr foo2 =  currentFunction;
	
	string = strNewFromCStr(":foo2Arg");
	tSymbolPtr foo2Arg;
	createFunctionVariable(eINT, true, string, true, foo2Arg);
	
	string = strNewFromCStr(":foo2tmpBOol");
	tSymbolPtr foo2tmpBOOL;
	createFunctionVariable(eBOOL, true, string, false, foo2tmpBOOL);



	
	string = strNewFromCStr("Kokos");
	createClass(string, currentClass);
	
	string = strNewFromCStr("fool");
	createFunction(eSTRING, true, string, currentFunction);
	tSymbolPtr fool =  currentFunction;
	
	string = strNewFromCStr(":foolArg");
	tSymbolPtr foolArg;
	createFunctionVariable(eSTRING, true, string, true, foolArg);
	
	string = strNewFromCStr(":fooltmpStr");
	tSymbolPtr fooltmpStr;
	createFunctionVariable(eSTRING, true, string, false, fooltmpStr);


	AI(iMOV, runLocaltmpStr, static666str, NULL); //run 1
	AI(iCONV2STR, runLocaltmpStr2, static7, NULL);
	AI(iPRINT, NULL, runLocaltmpStr2, NULL);
	AI(iADD, runLocaltmpStr, runLocaltmpStr, runLocaltmpStr2);
	AI(iPRINT, NULL, runLocaltmpStr, NULL);
	AI(iFRAME, NULL, foo1, NULL);
	AI(iCALL, NULL, NULL, NULL);
	AI(iGETRETVAL, runLocaltmpStr2, NULL, NULL);
	AI(iPRINT, NULL, runLocaltmpStr2, NULL);
	AI(iFRAME, NULL, foo2, NULL);
	AI(iPUSH, NULL, static7, NULL);
	AI(iINC, static7, NULL, NULL );
	AI(iCALL, NULL, NULL, NULL);
	AI(iGETRETVAL, runLocaltmpBool, NULL, NULL);
	AI(iCONV2STR,runLocaltmpStr ,runLocaltmpBool, NULL );
	AI(iPRINT, NULL, runLocaltmpStr, NULL);
	AI(iREAD, runLocaltmpStr, NULL, NULL);
	AI(iFRAME, NULL, fool, NULL);
	AI(iPUSH, NULL, runLocaltmpStr, NULL);
	AI(iCALL, NULL, NULL, NULL);
	AI(iGETRETVAL, runLocaltmpStr2, NULL, NULL);
	AI(iCONV2STR,runLocaltmpStr ,static7, NULL );
	AI(iPRINT, NULL, runLocaltmpStr, NULL);
	AI(iPRINT, NULL, runLocaltmpStr2, NULL);
	AI(iRET, NULL, NULL, NULL);
	AI(iCONV2STR,foo1LocaltmpStr3 ,constStr49, NULL ); //foo1 26
	AI(iADD,foo1LocaltmpStr3 , foo1LocaltmpStr3, foo1LocaltmpStr3 );
	AI(iRET, NULL, foo1LocaltmpStr3, NULL);
	AI(iLE, foo2tmpBOOL, foo2Arg, constStr49 );//foo2 29
	AI(iRET, NULL, foo2tmpBOOL, NULL);
	AI(iREAD, fooltmpStr, NULL, NULL);  //foo3 31
	AI(iADD,fooltmpStr,foolArg ,fooltmpStr );
	AI(iRET, NULL, fooltmpStr, NULL); //out:766674949true*8*


	foo1->Data.FunctionData.InstructionIndex = 26;
	foo2->Data.FunctionData.InstructionIndex = 29;
	fool->Data.FunctionData.InstructionIndex = 31;


	Interpret(globalScopeTable, list);
	htabRecursiveFree(globalScopeTable);
	constFree(constContainer);
	instrListFree(list);

}