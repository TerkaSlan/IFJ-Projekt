/*
 *  \file expr.c
 *  \brief Contains implementation of functions regarding expression handling
 *  \author Katka
 *
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

#include "expr.h"
#include "scanner.h"
#include "error.h"
#include "builtin.h"
#include "constants.h"
#include <stdlib.h>
#include <string.h>

#define PRECEDENCE_STACK_DEFAULT_SIZE 32
#define SYMBOL_STACK_DEFAULT_SIZE 32
#define NONTERMINALBORDER 23

// global variables from parser.c
extern Token* token;
extern tSymbolPtr result;
extern tHashTablePtr globalScopeTable;
extern tInstructionListPtr instructionList;
extern tConstContainerPtr constTable;
extern tSymbolPtr currentClass;
extern tSymbolPtr currentFunction;
extern tInstructionListPtr preInstructionList;
extern bool preinterpretation;

// global variables for unique temporary variable names
dtStrPtr tmpString;
dtStrPtr tmpInt;
dtStrPtr tmpDouble;
dtStrPtr tmpBool;
dtStrPtr tmpNull;

/*
 * Creates instruction for converting operand to given type
 */
#define convert(table, operand, instr, symbolTmp, dType)		\
do{																\
	dtStrPtr name;												\
	if (dType == eDOUBLE) {										\
		name = strNewFromStr(tmpDouble);						\
		strAddChar(tmpDouble, '.');								\
		instr.type = iCONV2DOUBLE;								\
	} else { 													\
		name = strNewFromStr(tmpString);						\
		strAddChar(tmpString, '@');								\
		instr.type = iCONV2STR;									\
	}															\
	tSymbolPtr tmp = symbolNew();								\
	tmp->Defined = true;										\
	tmp->Const = preinterpretation;								\
	tmp->Type = dType;											\
	tmp->Name = strNewFromStr(name);							\
	symbolTmp = htabAddSymbol(table, tmp, true);				\
	symbolFree(tmp);											\
	strFree(name);												\
	instr.dst = symbolTmp;										\
	instr.arg1 = operand;										\
	instr.arg2 = NULL;											\
} while (0);

/*
 * Creates new variable with given type, stores it into given table and returns pointer to it in symbolExprTmp
 */
#define tmpVariable(table, symbolExprTmp, dType)				\
do {															\
	tSymbolPtr exprTmp = symbolNew();							\
	exprTmp->Defined = true;									\
	exprTmp->Const = preinterpretation;							\
	exprTmp->Type = dType;										\
	switch(dType) {												\
		case eSTRING:											\
			exprTmp->Name = strNewFromStr(tmpString);			\
			strAddChar(tmpString, '@');							\
			break;												\
		case eDOUBLE:											\
			exprTmp->Name = strNewFromStr(tmpDouble);			\
			strAddChar(tmpDouble, '.');							\
			break;												\
		case eINT:												\
			exprTmp->Name = strNewFromStr(tmpInt);				\
			strAddChar(tmpInt, '#');							\
			break;												\
		case eBOOL:												\
			exprTmp->Name = strNewFromStr(tmpBool);				\
			strAddChar(tmpBool, '0');							\
			break;												\
		default:												\
			return ERR_INTERN;									\
	}															\
	symbolExprTmp = htabAddSymbol(table, exprTmp, true);		\
	symbolFree(exprTmp);										\
} while (0);

#define createInstruction(Instr, Type, Dst, Arg1, Arg2)	\
do {													\
	Instr.type = Type;									\
	Instr.dst = Dst;									\
	Instr.arg1 = Arg1;									\
	Instr.arg2 = Arg2;									\
} while(0);												\

#define insertInstruction(instr) preinterpretation ? (insertErrCode = instrListInsertInstruction(preInstructionList, instr)) : (insertErrCode = instrListInsertInstruction(instructionList, instr));

const uint32_t precedenceTable[24][24] =
{
// columns represent token->type
//    +   -   *   /   <   >  <=  >=  ==  !=  id f.id str  d   i   b    not and or   (   )   ,   ;   $
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_plus,
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_minus,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_multiply,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_divide,
	{'<','<','<','<','x','x','x','x','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_less,
	{'<','<','<','<','x','x','x','x','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_greater,
	{'<','<','<','<','x','x','x','x','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_lessEqual,
	{'<','<','<','<','x','x','x','x','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_greaterEqual,
	{'<','<','<','<','<','<','<','<','x','x','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_equal,
	{'<','<','<','<','<','<','<','<','x','x','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_notEqual,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','>','>','f','>','>','>','x'}, //TT_identifier,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','>','>','f','>','>','>','x'}, //TT_fullIdentifier,
	{'>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_string,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_double,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_number,
	{'>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','x','>','>','x','>','>','>','x'}, //TT_boolean,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_not,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_and,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','>','>','<','>','>','>','x'}, //TT_or,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','=','x','>','x'}, //TT_leftRoundBracket,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','>','>','>','x','>','x','>','x'}, //TT_rightRoundBracket,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','x','>','x','>','x'}, //TT_comma,
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','x'}, //TT_semicolon
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','e','e','e','x'}  //TT_dolar
																											   //rows represent stackTop
};

tPrecedenceStackPtr precedenceStackNew() {

	tPrecedenceStackPtr stack = malloc(sizeof(tPrecedenceStack));
	if (stack == NULL) {
		return NULL;
	}

	stack->data = calloc(PRECEDENCE_STACK_DEFAULT_SIZE, sizeof(uint32_t));
	if (stack->data == NULL) {
		free(stack);
		return NULL;
	}

	stack->top = -1;
	stack->capacity = PRECEDENCE_STACK_DEFAULT_SIZE;

	return stack;

}

void precedenceStackFree(tPrecedenceStackPtr stack) {

	free(stack->data);
	free(stack);

}

uint32_t precedenceStackTopTerminal(tPrecedenceStackPtr stack) {

	if (stack == NULL || stack->data == NULL || stack->top == -1) {
		return ERR_INTERN;
	}

	for (int32_t i = stack->top; i > -1; i--) {
		if (stack->data[i] <= NONTERMINALBORDER) {
			return stack->data[i];
		}
	}

	return ERR_INTERN;

}

eError precedenceStackPush(tPrecedenceStackPtr stack, uint32_t item) {

	if (stack == NULL || stack->data == NULL) {
		return ERR_INTERN;
	}

	if (stack->top == stack->capacity -1) {
		if ((stack->data = realloc(stack->data, stack->capacity * 2)) == NULL) {
			return ERR_INTERN;
		}
		stack->capacity = stack->capacity * 2;
	}

	stack->top = stack->top + 1;
	stack->data[stack->top] = item;

	return ERR_OK;

}

int64_t precedenceStackPop(tPrecedenceStackPtr stack) {

	uint32_t topValue = -1;

	if (stack->top > -1) {

		topValue = stack->data[stack->top];
		stack->top = stack->top - 1;

	}

	return topValue;

}

eError precedenceStackShift(tPrecedenceStackPtr stack) {

	if (stack->top == stack->capacity - 1) {
		if ((stack->data = realloc(stack->data, stack->capacity * 2)) == NULL) {
			return ERR_INTERN;
		}
		stack->capacity = stack->capacity * 2;
	}

	stack->top = stack->top + 1;

	int32_t i;
	for (i = stack->top - 1; i > -1; i--) {
		if (stack->data[i] > NONTERMINALBORDER) {
			stack->data[i + 1] = stack->data[i];
		} else {
			break;
		}
	}

	stack->data[i + 1] = TT_start;
	return ERR_OK;

}


tPrecedenceSymbolPtr precedenceSymbolNew() {

	tPrecedenceSymbolPtr symbol = calloc(1, sizeof(tPrecedenceSymbol));
	return symbol;

}

void precedenceSymbolFree(tPrecedenceSymbolPtr symbol) {
	if (symbol != NULL) {
		precedenceSymbolClean(symbol);
		free(symbol);
	}
}


void precedenceSymbolClean(tPrecedenceSymbolPtr symbol) {

	if ((symbol->type == TT_string)
	 || (symbol->type == TT_identifier
	 || (symbol->type == TT_fullIdentifier))) {
		strFree(symbol->stringOrId);
	}

	symbol->type = TT_empty;

}

tSymbolStackPtr symbolStackNew() {

	tSymbolStackPtr stack = malloc(sizeof(tSymbolStack));
	if (stack == NULL) {
		return NULL;
	}

	stack->data = calloc(SYMBOL_STACK_DEFAULT_SIZE, sizeof(tPrecedenceSymbol));
	if (stack->data == NULL) {
		free(stack);
		return NULL;
	}

	stack->top = -1;
	stack->capacity = SYMBOL_STACK_DEFAULT_SIZE;

	return stack;

}

void symbolStackFree(tSymbolStackPtr stack) {

	for(int32_t i = stack->top; i > -1; i--) {
		precedenceSymbolClean(&(stack->data[stack->top]));
	}

	free(stack->data);
	free(stack);

}

eError symbolStackPush(tSymbolStackPtr stack, tPrecedenceSymbolPtr item) {

	if (stack == NULL || stack->data == NULL) {
		return ERR_INTERN;
	}

	if (stack->top == stack->capacity -1) {
		if ((stack->data = realloc(stack->data, sizeof(tPrecedenceSymbol) * stack->capacity * 2)) == NULL) {
			return ERR_INTERN;
		}
		stack->capacity = stack->capacity * 2;
	}

	stack->top = stack->top + 1;

	stack->data[stack->top].type = item->type;

	if ((item->type == TT_string)
	 || (item->type == TT_identifier
	 || (item->type == TT_fullIdentifier))) {

		stack->data[stack->top].stringOrId = strNew();
		strCopyStr(stack->data[stack->top].stringOrId, item->stringOrId);

	} else if(item->type == TT_E) {

		stack->data[stack->top].symbol = item->symbol;

	} else {

		stack->data[stack->top].stringOrId = item->stringOrId;

	}

	return ERR_OK;

}

tPrecedenceSymbolPtr symbolStackPop(tSymbolStackPtr stack) {

	if (stack->top > -1) {

		tPrecedenceSymbolPtr symbol = precedenceSymbolNew();
		if (symbol == NULL) {
			return NULL;
		}

		symbol->type = stack->data[stack->top].type;

		if ((symbol->type == TT_string)
		 || (symbol->type == TT_identifier
		 || (symbol->type == TT_fullIdentifier))) {

			symbol->stringOrId = strNew();
			strCopyStr(symbol->stringOrId, stack->data[stack->top].stringOrId);

		} else if(symbol->type == TT_E) {

			symbol->symbol = stack->data[stack->top].symbol;

		} else {

			symbol->stringOrId = stack->data[stack->top].stringOrId;

		}

		precedenceSymbolClean(&(stack->data[stack->top]));
		stack->top = stack->top -1;
		return symbol;

	}

	return NULL;

}

eError functionParse(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	if (preinterpretation) {
		printError(ERR_SEM, "Line: %lu - Function call in static variable initialization isn't supported\n", (unsigned long)LineCounter);
		return ERR_SEM;
	}

	eError errCode;
	int32_t insertErrCode;
	tSymbolPtr symbolExprTmp;
	dtStrPtr func;

	int64_t funcId = precedenceStackPop(stack);
	if (funcId != TT_identifier && funcId != TT_fullIdentifier) {
		printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in function call\n", (unsigned long)LineCounter);
		return ERR_SYNTAX;
	}
	if (precedenceStackPop(stack) != TT_start) {
		printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in function call\n", (unsigned long)LineCounter);
		return ERR_SYNTAX;
	}

	tPrecedenceSymbolPtr funcName = symbolStackPop(symbolStack);
	if (funcName == NULL) {
		printError(ERR_INTERN, "Line: %lu - Unexpected symbol on symbol stack\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}

	tSymbolPtr funcSymbol;
	if (funcId == TT_fullIdentifier) {


		//get class name from fullIdentifier
		dtStrPtr className;
		errCode = substr(funcName->stringOrId, 0, strCharPos(funcName->stringOrId, '.'), &className);
		if (errCode != ERR_OK) {
			precedenceSymbolFree(funcName);
			return errCode;
		}

		//function call with full identifier might me builtin
		if(strCmpCStr(className, "ifj16") == 0) {
			errCode = builtinCall(funcName->stringOrId, stack, symbolStack);
			if (errCode != ERR_OK) {
				return errCode;
			}
			precedenceSymbolFree(funcName);

			cleanToken(&token);
			errCode = getToken(token);
			if (errCode != ERR_OK) {
				return errCode;
			}

			strFree(className);
			return errCode;
		}

		//look up class table in global scope table
		tSymbolPtr classSymbol = htabGetSymbol(globalScopeTable, className);
		strFree(className);
		if (classSymbol == NULL) {
			precedenceSymbolFree(funcName);
			printError(ERR_SEM, "Line: %lu - Using undefined class\n", (unsigned long)LineCounter);
			return ERR_SEM;
		}

		//get pointer to classTable from classSymbol
		tHashTablePtr classTable = classSymbol->Data.ClassData.LocalSymbolTable;
		if (classTable == NULL) {
			precedenceSymbolFree(funcName);
			printError(ERR_INTERN, "Line: %lu - No symbol table assigned to class\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		//get function name from fullIdentifier		
		errCode = substr(funcName->stringOrId, strCharPos(funcName->stringOrId, '.') + 1, strGetLength(funcName->stringOrId) - (strCharPos(funcName->stringOrId, '.') + 1), &func);
		precedenceSymbolFree(funcName);
		if (errCode != ERR_OK) {
			return errCode;
		}

		//look up function in current class table
		funcSymbol = htabGetSymbol(classTable, func);
		if (funcSymbol == NULL) {
			printError(ERR_SEM, "Line: %lu - Using undefined function\n", (unsigned long)LineCounter);
			return ERR_SEM;
		}

	} else {

		//function have to be in current class
		funcSymbol = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, funcName->stringOrId);
		if (funcSymbol == NULL) {
			printError(ERR_SEM, "Line: %lu - Using undefined function\n", (unsigned long)LineCounter);
			return ERR_SEM;
		}
		precedenceSymbolFree(funcName);

	}

	tInstruction instr = {iFRAME, NULL, funcSymbol, NULL};
	insertInstruction(instr);
	if (insertErrCode == -1) {
		printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}

	int32_t paramCount = funcSymbol->Data.FunctionData.NumberOfArguments;
	tArgumentListItem* argument = funcSymbol->Data.FunctionData.ArgumentList;

	errCode = parsing(NULL);
	if (errCode != ERR_OK) {
		return errCode;
	}

	if (paramCount == 0) {
		if (result == NULL) {
			goto afterParams;
		} else {
			printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in function call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}
	}

	while (paramCount > 0) {

		if (result == NULL) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in function call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		if (result->Type == eINT && argument->Symbol->Type == eDOUBLE) {
			//convert result from expressions parsing to double
			tSymbolPtr symbolTmp;
			convert(funcSymbol->Data.FunctionData.LocalSymbolTable, result, instr, symbolTmp, eDOUBLE);
			//add instr to instructionList
			insertInstruction(instr);
			if (insertErrCode == -1) {
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			result = symbolTmp;
		}

		if (result->Type == argument->Symbol->Type) {

			createInstruction(instr, iPUSH, NULL, result, NULL);
			insertInstruction(instr);
			if (insertErrCode == -1) {
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			paramCount--;
			argument = argument->Next;

		} else {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible argument type in function call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
		}

		if (paramCount > 0) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
		}

	}

	if (token->type != TT_rightRoundBracket) {
		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}
		if (result != NULL) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in function call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		} else {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}
	}

	afterParams:

	createInstruction(instr, iCALL, NULL, NULL, NULL);
	insertInstruction(instr);
	if (insertErrCode == -1) {
		printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}

	tPrecedenceSymbolPtr funcPrecedenceSymbol = precedenceSymbolNew();

	if (funcSymbol->Data.FunctionData.ReturnType != eNULL) {

		precedenceStackPush(stack, TT_E);

		funcPrecedenceSymbol->type = TT_E;

		tSymbolPtr symbolExprTmp;
		tmpVariable(currentFunction->Data.FunctionData.LocalSymbolTable, symbolExprTmp, funcSymbol->Data.FunctionData.ReturnType);

		createInstruction(instr, iGETRETVAL, symbolExprTmp, NULL, NULL);
		insertInstruction(instr);
		if (insertErrCode == -1) {
			precedenceSymbolFree(funcPrecedenceSymbol);
			printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		funcPrecedenceSymbol->symbol = symbolExprTmp;

		if (symbolStackPush(symbolStack, funcPrecedenceSymbol) == ERR_INTERN) {
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			precedenceSymbolFree(funcPrecedenceSymbol);
			return ERR_INTERN;
		}

	} else {

		precedenceStackPush(stack, TT_void);

		tSymbolPtr exprTmp = symbolNew();
		exprTmp->Defined = true;
		exprTmp->Const = false;
		exprTmp->Type = eNULL;
		exprTmp->Name = strNewFromStr(tmpNull);
		strAddChar(tmpNull, '*');
		symbolExprTmp = htabAddSymbol(funcSymbol->Data.FunctionData.LocalSymbolTable, exprTmp, true);
		symbolFree(exprTmp);

		funcPrecedenceSymbol->type = TT_void;
		funcPrecedenceSymbol->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, funcPrecedenceSymbol) == ERR_INTERN) {
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			precedenceSymbolFree(funcPrecedenceSymbol);
			return ERR_INTERN;
		}

	}

	precedenceSymbolFree(funcPrecedenceSymbol);

	cleanToken(&token);
	errCode = getToken(token);
	if (errCode != ERR_OK) {
		return errCode;
	}
	return ERR_OK;

}

eError builtinCall(dtStrPtr builtin, tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	tSymbolPtr symbolTmp;
	tSymbolPtr symbolExprTmp;
	tInstruction instr;
	eError errCode;
	tHashTablePtr currentFuncTable;
	if (preinterpretation) {
		currentFuncTable = currentClass->Data.ClassData.LocalSymbolTable;
	} else {
		currentFuncTable = currentFunction->Data.FunctionData.LocalSymbolTable;
	}

		if (strCmpCStr(builtin, "ifj16.substr") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type == TT_rightRoundBracket) {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			} else {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
		}

		if (token->type == TT_rightRoundBracket) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		//this builtin has three parameters - first parameter have to be followed by comma
		if (token->type != TT_comma) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr s = result;

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		//this builtin has three parameters - second parameter have to be followed by comma
		if (token->type == TT_rightRoundBracket) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}
		if (token->type != TT_comma) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eINT) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr i = result;

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		//this builtin has three parameters - precedence parsing have to stop on right round bracket after third parameter
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}
		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eINT) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr n = result;

        createInstruction(instr, iSUBSTR, s, i, n);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		tmpVariable(currentFuncTable, symbolExprTmp, eSTRING);

		createInstruction(instr, iGETRETVAL, symbolExprTmp, NULL, NULL);
		if (instrListInsertInstruction(instructionList, instr) == -1 ) {
			printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}
	if (strCmpCStr(builtin, "ifj16.readDouble") == 0 || (strCmpCStr(builtin, "ifj16.readInt") == 0) || (strCmpCStr(builtin, "ifj16.readString") == 0)) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result != NULL) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}
		//this builtin has no parameters - next token should be right round bracket
		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		eSymbolType symbolType;
		if ((strCmpCStr(builtin, "ifj16.readDouble") == 0))
			symbolType = eDOUBLE;
		if ((strCmpCStr(builtin, "ifj16.readInt") == 0))
			symbolType = eINT;
		if ((strCmpCStr(builtin, "ifj16.readString") == 0))
			symbolType = eSTRING;
		tmpVariable(currentFuncTable, symbolExprTmp, symbolType);

        createInstruction(instr, iREAD, symbolExprTmp, NULL, NULL);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}

	if (strCmpCStr(builtin, "ifj16.print") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type == TT_rightRoundBracket) {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			} else {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
		}

		//this builtin has only one parameter - precedence parsing have to stop on right round bracket
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}
		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			convert(currentFuncTable, result, instr, symbolTmp, eSTRING);
			if (instrListInsertInstruction(instructionList, instr) == -1) {
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
		} else {
			symbolTmp = result;
		}

        createInstruction(instr, iPRINT, NULL, symbolTmp, NULL);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_void);

		tSymbolPtr exprTmp = symbolNew();
		exprTmp->Defined = true;
		exprTmp->Const = false;
		exprTmp->Type = eNULL;
		exprTmp->Name = strNewFromStr(tmpNull);
		strAddChar(tmpNull, '*');
		symbolExprTmp = htabAddSymbol(currentFuncTable, exprTmp, true);
		symbolFree(exprTmp);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}
	if (strCmpCStr(builtin, "ifj16.length") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type != TT_rightRoundBracket) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}

		//this builtin has only one parameter - precedence parsing have to stop on right round bracket
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}

		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tmpVariable(currentFuncTable, symbolExprTmp, eINT);

        createInstruction(instr, iLEN, symbolExprTmp, result, NULL);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}
	if (strCmpCStr(builtin, "ifj16.compare") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type == TT_rightRoundBracket) {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			} else {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
		}

		if (token->type == TT_rightRoundBracket) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		//this builtin has two parameters - first parameter have to be followed by comma
		if (token->type != TT_comma) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr s1 = result;

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		//this builtin has two parameters - precedence parsing have to stop on right round bracket after second parameter
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}

		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr s2 = result;

		tmpVariable(currentFuncTable, symbolExprTmp, eINT);

        createInstruction(instr, iCOMPARE, symbolExprTmp, s1, s2);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}
	if (strCmpCStr(builtin, "ifj16.find") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type == TT_rightRoundBracket) {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			} else {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
		}

		if (token->type == TT_rightRoundBracket) {
			printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		//this builtin has two parameters - first parameter have to be followed by comma
		if (token->type != TT_comma) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr s = result;

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		//this builtin has two parameters - precedence parsing have to stop on right round bracket after second parameter
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}
		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tSymbolPtr search = result;

		tmpVariable(currentFuncTable, symbolExprTmp, eINT);

        createInstruction(instr, iFIND, symbolExprTmp, s, search);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;

	}
	if (strCmpCStr(builtin, "ifj16.sort") == 0) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (result == NULL) {
			if (token->type != TT_rightRoundBracket) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too few paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}

		//this builtin has only one parameter - precedence parsing have to stop on right round bracket
		if (token->type == TT_comma) {
			errCode = parsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (result == NULL) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			} else {
				printError(ERR_SEM_TYPE, "Line: %lu - Too much paramaters in builtin call\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}
		}

		if (token->type != TT_rightRoundBracket) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in builtin call\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
		}

		if (result->Type != eSTRING) {
			printError(ERR_SEM_TYPE, "Line: %lu - Incompatible type of parameter in builtin call\n", (unsigned long)LineCounter);
			return ERR_SEM_TYPE;
		}

		tmpVariable(currentFuncTable, symbolExprTmp, eSTRING);

        createInstruction(instr, iSORT, symbolExprTmp, result, NULL);
        if (instrListInsertInstruction(instructionList, instr) == -1) {
        	printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}

		precedenceStackPush(stack, TT_E);

		tPrecedenceSymbolPtr returnVal = precedenceSymbolNew();
		returnVal->type = TT_E;
		returnVal->symbol = symbolExprTmp;
		if (symbolStackPush(symbolStack, returnVal) == ERR_INTERN) {
			precedenceSymbolFree(returnVal);
			printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
			return ERR_INTERN;
		}
		precedenceSymbolFree(returnVal);

		return ERR_OK;
	}

	return ERR_SEM;

}

eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	eError errCode;
	int32_t insertErrCode;

	tHashTablePtr currentTable;
	if (preinterpretation) {
		currentTable = currentClass->Data.ClassData.LocalSymbolTable;
	} else {
		currentTable = currentFunction->Data.FunctionData.LocalSymbolTable;
	}

	switch(precedenceStackTopTerminal(stack)) {

		//E -> ID
		case TT_identifier: {

			if (precedenceStackPop(stack) != TT_identifier) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			//get precedence symbol representing current id from symbol stack
			tPrecedenceSymbolPtr id = symbolStackPop(symbolStack);

			tSymbolPtr symbolId;

			if (preinterpretation) {
				symbolId = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, id->stringOrId);
				if (symbolId == NULL) {
					precedenceSymbolFree(id);
					printError(ERR_SEM, "Line: %lu - Using undefined identfier\n", (unsigned long)LineCounter);
					return ERR_SEM;
				}
			} else {
				//look up id in symbol table of current function scope
				symbolId = htabGetSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, id->stringOrId);
				if (symbolId == NULL) {

					//id isn't in current function symbol table - but it still could be static id in current class symbol table
					symbolId = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, id->stringOrId);
					 if (symbolId == NULL || symbolId->Data.FunctionData.LocalSymbolTable != NULL) {
						precedenceSymbolFree(id);
						printError(ERR_SEM, "Line: %lu - Using undefined identfier\n", (unsigned long)LineCounter);
						return ERR_SEM;
					}
				}
			}

			//precedence symbol containing pointer to current id in symbol table - push on symbol stack
			precedenceSymbolClean(id);
			id->type = TT_E;
			id->symbol = symbolId;
			if (symbolStackPush(symbolStack, id) == ERR_INTERN) {
				precedenceSymbolFree(id);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(id);

			//push type representing operand on precedence stack
			precedenceStackPush(stack, TT_E);
			break;
		}
		case TT_fullIdentifier: {

			if (precedenceStackPop(stack) != TT_fullIdentifier) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr fullId = symbolStackPop(symbolStack);

			//get class name from fullIdentifier
			dtStrPtr className;
			errCode = substr(fullId->stringOrId, 0, strCharPos(fullId->stringOrId, '.'), &className);
			if (errCode != ERR_OK) {
				precedenceSymbolFree(fullId);
				return errCode;
			}

			//look up class table in global scope table
			tSymbolPtr classSymbol = htabGetSymbol(globalScopeTable, className);
			strFree(className);
			if (classSymbol == NULL) {
				precedenceSymbolFree(fullId);
				printError(ERR_SEM, "Line: %lu - Using undefined class\n", (unsigned long)LineCounter);
				return ERR_SEM;
			}

			//get pointer to classTable from classSymbol
			tHashTablePtr classTable = classSymbol->Data.ClassData.LocalSymbolTable;
			if (classTable == NULL) {
				precedenceSymbolFree(fullId);
				return ERR_INTERN;
			}

			dtStrPtr id;
			errCode = substr(fullId->stringOrId, strCharPos(fullId->stringOrId, '.') + 1, strGetLength(fullId->stringOrId) - (strCharPos(fullId->stringOrId, '.') + 1), &id);
			if (errCode != ERR_OK) {
				precedenceSymbolFree(fullId);
				strFree(className);
				return errCode;
			}

			tSymbolPtr symbolId;
			symbolId = htabGetSymbol(classTable, id);
			strFree(id);
			if (symbolId == NULL || symbolId->Data.FunctionData.LocalSymbolTable != NULL) {
				precedenceSymbolFree(fullId);
				printError(ERR_SEM, "Line: %lu - Using undefined identifier\n", (unsigned long)LineCounter);
				return ERR_SEM;
			}

			precedenceSymbolClean(fullId);
			fullId->type = TT_E;
			fullId->symbol = symbolId;
			if (symbolStackPush(symbolStack, fullId) == ERR_INTERN) {
				precedenceSymbolFree(fullId);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}

			precedenceStackPush(stack, TT_E);

			precedenceSymbolFree(fullId);
			return ERR_OK;
		}

		// E -> const
		case TT_string:
		case TT_double:
		case TT_number:
		case TT_boolean:

			precedenceStackPop(stack);
			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr constant = symbolStackPop(symbolStack);

			//create new constant adn fill it
			tSymbolPtr symbolConst = symbolNew();
			symbolConst->Const = true;
			symbolConst->Name = NULL;
			symbolConst->Defined = true;
			switch(constant->type) {
				case TT_string:
					symbolConst->Type = eSTRING;
					symbolConst->Data.String = strNewFromStr(constant->stringOrId);
					break;
				case TT_double:
					symbolConst->Type = eDOUBLE;
					symbolConst->Data.Double = constant->dNum;
					break;
				case TT_number:
					symbolConst->Type = eINT;
					symbolConst->Data.Integer = constant->iNum;
					break;
				case TT_boolean:
					symbolConst->Type = eBOOL;
					symbolConst->Data.Bool = constant->iNum;
					break;
				default:
					precedenceSymbolFree(constant);
					symbolFree(symbolConst);
					printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
					return ERR_SYNTAX;
			}

			precedenceSymbolClean(constant);

			//store created constant to constTable
			constant->symbol = constInsertSymbol(constTable, *symbolConst);
			constant->type = TT_E;

			free(symbolConst);
			if (symbolStackPush(symbolStack, constant) == ERR_INTERN) {
				precedenceSymbolFree(constant);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(constant);

			precedenceStackPush(stack, TT_E);
			break;

		// E -> ( E )
		case TT_rightRoundBracket:

			if (precedenceStackPop(stack) != TT_rightRoundBracket) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_leftRoundBracket) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			precedenceStackPush(stack, TT_E);
			break;

		case TT_less:
		case TT_lessEqual:
		case TT_greater:
		case TT_greaterEqual:
		case TT_equal:
		case TT_notEqual: {

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			int64_t operator = precedenceStackPop(stack);
			if (operator == -1 ) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand2 = symbolStackPop(symbolStack);
			tPrecedenceSymbolPtr operand1 = symbolStackPop(symbolStack);
			tSymbolPtr symbolExprTmp;
			tSymbolPtr symbolTmp;
			tInstruction instr;

			if (operand1->symbol->Type == eINT) {
				if (operand2->symbol->Type == eINT) {

					goto generateBoolInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {

					//convert first operand to double
					convert(currentTable, operand1->symbol, instr, symbolTmp, eDOUBLE);

					insertInstruction(instr);
					if (insertErrCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
						return ERR_INTERN;
					}
					operand1->symbol = symbolTmp;

					goto generateBoolInstruction;
				} else {

					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			} else if (operand1->symbol->Type == eDOUBLE) {
				if (operand2->symbol->Type == eINT) {

					//convert second operand to double
					convert(currentTable, operand2->symbol, instr, symbolTmp, eDOUBLE);

					insertInstruction(instr);
					if (insertErrCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
						return ERR_INTERN;
					}
					operand2->symbol = symbolTmp;

					goto generateBoolInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {

					goto generateBoolInstruction;
				} else {

					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			} else {

				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}

			generateBoolInstruction: {

			tmpVariable(currentTable, symbolExprTmp, eBOOL);
			eInstructionType instruction;
			switch(operator) {
				case TT_less:
					instruction = iLT;

					break;
				case TT_lessEqual:
					instruction = iLE;

					break;
				case TT_greater:
					instruction = iGT;

					break;
				case TT_greaterEqual:
					instruction = iGE;

					break;
				case TT_equal:
					instruction = iEQ;

					break;
				case TT_notEqual:
					instruction = iNEQ;

					break;
				default:
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
					return ERR_SYNTAX;
			}

			instr.type = instruction;
			instr.dst = symbolExprTmp;
			instr.arg1 = operand1->symbol;
			instr.arg2 = operand2->symbol;

			insertInstruction(instr);
			if (insertErrCode == -1) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand1);
			precedenceSymbolFree(operand2);

			//push pointer to result on symbol stack
			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			if (symbolStackPush(symbolStack, result) == ERR_INTERN) {
				precedenceSymbolFree(result);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;
			}

		}
		case TT_minus:
		case TT_plus:
		case TT_multiply:
		case TT_divide: {

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			int64_t operator = -1;
			operator = precedenceStackPop(stack);
			if (operator == -1 ) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand2 = symbolStackPop(symbolStack);
			tPrecedenceSymbolPtr operand1 = symbolStackPop(symbolStack);
			tSymbolPtr symbolExprTmp;
			tSymbolPtr symbolTmp;
			tInstruction instr;

			if (operand1->symbol->Type == eSTRING) {
				if (operator == TT_plus) {
					if (operand2->symbol->Type != eSTRING) {

						//convert second operand to string
						convert(currentTable, operand2->symbol, instr, symbolTmp, eSTRING);

						insertInstruction(instr);
						if (insertErrCode == -1) {
							precedenceSymbolFree(operand1);
							precedenceSymbolFree(operand2);
							printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
							return ERR_INTERN;
						}
						operand2->symbol = symbolTmp;
					}

					tmpVariable(currentTable, symbolExprTmp, eSTRING)
					goto generateInstruction;

				} else {
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			}
			if (operand2->symbol->Type == eSTRING) {
				if (operator == TT_plus) {
					if (operand1->symbol->Type != eSTRING) {

						//convert first operand to string
						convert(currentTable, operand1->symbol, instr, symbolTmp, eSTRING);

						insertInstruction(instr);
						if (insertErrCode == -1) {
							precedenceSymbolFree(operand1);
							precedenceSymbolFree(operand2);
							printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
							return ERR_INTERN;
						}
						operand1->symbol = symbolTmp;
					}

					tmpVariable(currentTable, symbolExprTmp, eSTRING)

					goto generateInstruction;
				} else {
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			}

			if (operand1->symbol->Type == eINT) {
				if (operand2->symbol->Type == eINT) {

					tmpVariable(currentTable, symbolExprTmp, eINT)

					goto generateInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {

					//convert first operand to double
					convert(currentTable, operand1->symbol, instr, symbolTmp, eDOUBLE);

					insertInstruction(instr);
					if (insertErrCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
						return ERR_INTERN;
					}
					operand1->symbol = symbolTmp;

					tmpVariable(currentTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else {

					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			} else if (operand1->symbol->Type == eDOUBLE) {
				if (operand2->symbol->Type == eINT) {

					//convert second operand to double
					convert(currentTable, operand2->symbol, instr, symbolTmp, eDOUBLE);

					insertInstruction(instr);
					if (insertErrCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						printError(ERR_INTERN, "Line: %lu - Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
						return ERR_INTERN;
					}
					operand2->symbol = symbolTmp;

					tmpVariable(currentTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {

					tmpVariable(currentTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else {

					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
					return ERR_SEM_TYPE;
				}
			} else {

				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}

			generateInstruction: {

			eInstructionType instruction;
			switch(operator) {
				case TT_minus:
					instruction = iSUB;

					break;
				case TT_plus:
					instruction = iADD;

					break;
				case TT_multiply:
					instruction = iMUL;

					break;
				case TT_divide:
					instruction = iDIV;

					break;
				default:
					free(operand1);
					free(operand2);
					printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
					return ERR_SYNTAX;
			}

			createInstruction(instr, instruction, symbolExprTmp, operand1->symbol, operand2->symbol);
			insertInstruction(instr);
			if (insertErrCode == -1) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand1);
			precedenceSymbolFree(operand2);

			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			if (symbolStackPush(symbolStack, result) == ERR_INTERN) {
				precedenceSymbolFree(result);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;
			}
		}

		case TT_and:
		case TT_or: {

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			int64_t operator = precedenceStackPop(stack);
			if (operator != TT_and && operator != TT_or) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand2 = symbolStackPop(symbolStack);
			tPrecedenceSymbolPtr operand1 = symbolStackPop(symbolStack);

			if (operand1->symbol->Type != eBOOL || operand2->symbol->Type != eBOOL) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}

			tSymbolPtr symbolExprTmp;
			tmpVariable(currentTable, symbolExprTmp, eBOOL)

			tInstruction instr;

			if (operator == TT_and) {
				createInstruction(instr, iLAND, symbolExprTmp, operand1->symbol, operand2->symbol);
			}
			if (operator == TT_or) {
				createInstruction(instr, iLOR, symbolExprTmp, operand1->symbol, operand2->symbol);
			}

			insertInstruction(instr);
			if (insertErrCode == -1) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand1);
			precedenceSymbolFree(operand2);

			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			if (symbolStackPush(symbolStack, result) == ERR_INTERN) {
				precedenceSymbolFree(result);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;
		}

		// E -> ! E
		case TT_not:

			if (precedenceStackPop(stack) != TT_E) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_not) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_start) {
				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression\n", (unsigned long)LineCounter);
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand = symbolStackPop(symbolStack);
			if (operand->symbol->Type != eBOOL) {
				precedenceSymbolFree(operand);
				printError(ERR_SEM_TYPE, "Line: %lu - Incompatible types in expression\n", (unsigned long)LineCounter);
				return ERR_SEM_TYPE;
			}

			tSymbolPtr symbolExprTmp;
			tmpVariable(currentTable, symbolExprTmp, eBOOL);

			tInstruction instr = {iLNOT, symbolExprTmp, operand->symbol, NULL};

			insertInstruction(instr);
			if (insertErrCode == -1) {
				precedenceSymbolFree(operand);
				printError(ERR_INTERN, "Line: %lu - Insert of instruction wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand);

			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			if (symbolStackPush(symbolStack, result) == ERR_INTERN) {
				precedenceSymbolFree(result);
				printError(ERR_INTERN, "Line: %lu - Push on symbol stack wasn't successful\n", (unsigned long)LineCounter);
				return ERR_INTERN;
			}
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;

		default:
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression parsing\n", (unsigned long)LineCounter);
			return ERR_SYNTAX;
	}

	return ERR_OK;

}

//entry point of expressions parsing
eError precedenceParsing(Token* helpToken) {

	tmpString = strNew();
	if (tmpString == NULL) {
		return ERR_INTERN;
	}
	if (strAddChar(tmpString, '@') != ERR_OK) {
		strFree(tmpString);
		return ERR_INTERN;
	}
	tmpInt = strNew();
	if (tmpInt == NULL) {
		strFree(tmpString);
		return ERR_INTERN;
	}
	if (strAddChar(tmpInt, '#') != ERR_OK) {
		strFree(tmpString);
		strFree(tmpInt);
		return ERR_INTERN;
	}
	tmpDouble = strNew();
	if (tmpDouble == NULL) {
		strFree(tmpString);
		strFree(tmpInt);
		return ERR_INTERN;
	}
	if (strAddChar(tmpDouble, '.') != ERR_OK) {
		strFree(tmpString);
		strFree(tmpInt);
		strFree(tmpDouble);
		return ERR_INTERN;
	}
	tmpBool = strNew();
	if (tmpBool == NULL) {
		strFree(tmpString);
		strFree(tmpInt);
		strFree(tmpDouble);
		return ERR_INTERN;
	}
	if (strAddChar(tmpBool, '0') != ERR_OK) {
		strFree(tmpString);
		strFree(tmpInt);
		strFree(tmpDouble);
		strFree(tmpBool);
		return ERR_INTERN;
	}
	tmpNull = strNew();
	if (tmpNull == NULL) {
		strFree(tmpString);
		strFree(tmpInt);
		strFree(tmpDouble);
		strFree(tmpBool);
		return ERR_INTERN;
	}
	if (strAddChar(tmpNull, '*') != ERR_OK) {
		strFree(tmpString);
		strFree(tmpInt);
		strFree(tmpDouble);
		strFree(tmpBool);
		strFree(tmpNull);
		return ERR_INTERN;
	}

	eError errCode = parsing(helpToken);

	strFree(tmpString);
	strFree(tmpInt);
	strFree(tmpDouble);
	strFree(tmpBool);
	strFree(tmpNull);

	return errCode;

}

eError parsing(Token* helpToken) {


	tPrecedenceStackPtr stack;
	if ((stack = precedenceStackNew()) == NULL) {
		printError(ERR_INTERN, "Line: %lu - Cannot create precedence stack.\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}
	precedenceStackPush(stack, TT_dolar);

	tPrecedenceSymbolPtr symbol;
	if ((symbol = precedenceSymbolNew()) == NULL) {
		precedenceStackFree(stack);
		printError(ERR_INTERN, "Line: %lu - Cannot create precedence symbol.\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}

	tSymbolStackPtr symbolStack;
	if ((symbolStack = symbolStackNew()) == NULL) {
		precedenceSymbolFree(symbol);
		precedenceStackFree(stack);
		printError(ERR_INTERN, "Line: %lu - Cannot create precedence symbol stack.\n", (unsigned long)LineCounter);
		return ERR_INTERN;
	}

	int64_t stackTop;
	eError errCode;


	if(helpToken == NULL) {

		cleanToken(&token);
		errCode = getToken(token);
		if (errCode != ERR_OK) {
			goto freeAndExit;
		}

	} else {

		errCode = precedenceStackShift(stack);
		if (errCode != ERR_OK) {
			goto freeAndExit;
		}
		precedenceStackPush(stack, helpToken->type);
		symbol->type = helpToken->type;

		if ((symbol->type == TT_string)
	 	 || (symbol->type == TT_identifier
	 	 || (symbol->type == TT_fullIdentifier))) {

	 	 	symbol->stringOrId = strNew();
			strCopyStr(symbol->stringOrId, helpToken->str);

		} else if (symbol->type == TT_double) {

			symbol->dNum = helpToken->dNum;

		} else {

			symbol->iNum = helpToken->iNum;

		}
		if (symbolStackPush(symbolStack, symbol) == ERR_INTERN) {
			errCode = ERR_INTERN;
			goto freeAndExit;
		}
	}

	//main loop for expressions parsing
	do {

		//converting bools to make use of token->type consistent
		if(token->type == TT_keyword) {
			if (token->keywordType == KTT_true) {
				token->type = TT_boolean;
				token->iNum = 1;
			} else if (token->keywordType == KTT_false) {
				token->type = TT_boolean;
				token->iNum = 0;
			}
		}

		if (token->type > TT_semicolon) {
			printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression parsing\n", (unsigned long)LineCounter);
			errCode = ERR_SYNTAX;
			goto freeAndExit;
		}

		stackTop = precedenceStackTopTerminal(stack);
		switch (precedenceTable[stackTop][token->type]) {

			case '=':
			case '<':

				if (precedenceTable[stackTop][token->type] == '<') {
					errCode = precedenceStackShift(stack);
					if (errCode != ERR_OK) {
						goto freeAndExit;
					}
				}

				precedenceStackPush(stack, token->type);
				if (token->type < TT_not && token->type > TT_notEqual) {

					precedenceSymbolClean(symbol);
					symbol->type = token->type;

					if ((symbol->type == TT_string)
	 				 || (symbol->type == TT_identifier
	 				 || (symbol->type == TT_fullIdentifier))) {

	 				 	symbol->stringOrId = strNewFromStr(token->str);

					} else if (symbol->type == TT_double) {

						symbol->dNum = token->dNum;

					} else {

						symbol->iNum = token->iNum;

					}

					if (symbolStackPush(symbolStack, symbol) == ERR_INTERN) {
						errCode = ERR_INTERN;
						goto freeAndExit;
					}

				}

				//read next token
				cleanToken(&token);
				errCode = getToken(token);
				if (errCode != ERR_OK) {
					goto freeAndExit;
				}

				break;

			case '>':

				errCode = reduce(stack, symbolStack);
				if (errCode != ERR_OK) {
					result = NULL;
					goto freeAndExit;
				}

				continue;

			case 'f':

				errCode = functionParse(stack, symbolStack);
				if (errCode != ERR_OK) {
					result = NULL;
					goto freeAndExit;
				}
				break;

			case 'e':

				if (symbolStack->top > -1) {
					tPrecedenceSymbolPtr stackPop = symbolStackPop(symbolStack);
					result = stackPop->symbol;
					free(stackPop);
				} else {
					result = NULL;
				}
				errCode = ERR_OK;
				goto freeAndExit;

			default:

				printError(ERR_SYNTAX, "Line: %lu - Unexpected symbol in expression parsing\n", (unsigned long)LineCounter);
				errCode = ERR_SYNTAX;
				result = NULL;
				goto freeAndExit;

		}

	stackTop = precedenceStackTopTerminal(stack);
	} while(stackTop != TT_dolar
		|| (token->type != TT_comma
		&&	token->type != TT_semicolon
		&&  token->type != TT_rightRoundBracket));


	if (symbolStack->top > -1) {
		tPrecedenceSymbolPtr stackPop = symbolStackPop(symbolStack);
		result = stackPop->symbol;
		free(stackPop);
	} else {
		result = NULL;
	}

	freeAndExit:
	precedenceStackFree(stack);
	precedenceSymbolFree(symbol);
	symbolStackFree(symbolStack);
	return errCode;

}
