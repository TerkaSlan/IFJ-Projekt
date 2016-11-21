//TODO
//	unary ++,--, -

#include "expr.h"
#include "scanner.h"
#include "error.h"
#include "builtin.h"
#include "constants.h"
#include <stdlib.h>
#include <string.h>

#define PRECEDENCE_STACK_DEFAULT_SIZE 32
#define SYMBOL_STACK_DEFAULT_SIZE 32
#define NONTERMINALBORDER 25

// global variables from parser.c
extern Token* token;
extern tSymbolPtr result;
extern tHashTablePtr globalScopeTable;
extern tInstructionListPtr instructionList;
extern tConstContainerPtr constTable;
extern tSymbolPtr currentClass;
extern tSymbolPtr currentFunction;

// global variables for unique temporary variable names
dtStrPtr tmpString;
dtStrPtr tmpInt;
dtStrPtr tmpDouble;
dtStrPtr tmpBool;

#define convert(table, operand, instr, symbolTmp, dType)		\
do{																\
	dtStrPtr name;												\
	if (dType == eDOUBLE) {										\
		name = strNewFromCStr("@double");						\
		instr.type = iCONV2DOUBLE;								\
	} else { 													\
		name = strNewFromCStr("@string");						\
		instr.type = iCONV2STR;									\
	}															\
	symbolTmp = htabGetSymbol(table, name);						\
	if (symbolTmp == NULL) {									\
		tSymbolPtr tmp = symbolNew();							\
		tmp->Defined = false;									\
		tmp->Const = false;										\
		tmp->Type = dType;										\
		tmp->Name = strNewFromStr(name);						\
		symbolTmp = htabAddSymbol(table, tmp, false);			\
		symbolFree(tmp);										\
	}															\
	strFree(name);												\
	instr.dst = symbolTmp;										\
	instr.arg1 = operand;										\
	instr.arg2 = NULL;											\
} while (0);

/*
 * Creates new variable with given type, stores it into given table and returns pointer to variable in symbolExprTmp
 */
#define tmpVariable(table, symbolExprTmp, type)					\
do {															\
	tSymbolPtr exprTmp = symbolNew();							\
	exprTmp->Defined = true;									\
	exprTmp->Const = false;										\
	exprTmp->Type = type;										\
	switch(type) {												\
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

const uint32_t precedenceTable[26][26] =
{
// columns represent token->type
//    +   -   *   /   <   >  <=  >=  ==  !=  id f.id str  d   i   b  ++  --  not and or   (   )   ,   ;   $
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','<','u','u','<','>','>','<','>','>','>','x'}, //TT_plus,
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_minus,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_multiply,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_divide,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_less,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_greater,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_lessEqual,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_greaterEqual,
	{'<','<','<','<','<','<','<','<','x','x','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_equal,
	{'<','<','<','<','<','<','<','<','x','x','<','<','x','<','<','x','u','u','<','>','>','<','>','>','>','x'}, //TT_notEqual,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','u','u','x','>','>','f','>','>','>','x'}, //TT_identifier,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','u','u','x','>','>','f','>','>','>','x'}, //TT_fullIdentifier,
	{'>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_string,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_double,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','>','>','>','x'}, //TT_number,
	{'>','>','>','>','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','>','x','>','>','>','x'}, //TT_boolean,
	{'>','>','>','>','>','>','>','>','>','>','u','u','x','x','x','x','x','x','<','>','>','<','>','>','>','x'}, //TT_increment,
	{'>','>','>','>','>','>','>','>','>','>','u','u','x','x','x','x','x','x','<','>','>','<','>','>','>','x'}, //TT_decrement,
	{'>','>','>','>','>','>','>','>','>','>','<','<','x','x','x','<','u','u','<','>','>','<','>','>','>','x'}, //TT_not,
	{'<','<','<','<','<','<','<','<','<','<','<','<','x','x','x','<','u','u','<','>','>','<','>','>','>','x'}, //TT_and,
	{'<','<','<','<','<','<','<','<','<','<','<','<','x','x','x','<','u','u','<','>','>','<','>','>','>','x'}, //TT_or,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','=','x','>','x'}, //TT_leftRoundBracket,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','u','u','>','>','>','x','>','x','>','x'}, //TT_rightRoundBracket,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','x','>','x','>','x'}, //TT_comma,
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','x'}, //TT_semicolon
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','e','e','e','x'}  //TT_dolar
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
		if ((symbol->type == TT_string)
		 || (symbol->type == TT_identifier
		 || (symbol->type == TT_fullIdentifier))) {
			strFree(symbol->stringOrId);
		}
		free(symbol);
	}
}


void precedenceSymbolClean(tPrecedenceSymbolPtr symbol) {

	if ((symbol->type == TT_string)
	 || (symbol->type == TT_identifier
	 || (symbol->type == TT_fullIdentifier))) {
		strFree(symbol->stringOrId);
	}
//	if (symbol->type == TT_E) {
//		symbolFree(symbol->symbol);
//	}
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

int64_t symbolStackPush(tSymbolStackPtr stack, tPrecedenceSymbolPtr item) {

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

	return stack->top;

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

	eError errCode;

	int64_t funcId = precedenceStackPop(stack);
	if (funcId != TT_identifier && funcId != TT_fullIdentifier) {
		return ERR_SYNTAX;
	}
	if (precedenceStackPop(stack) != TT_start) {
		return ERR_SYNTAX;
	}

	tPrecedenceSymbolPtr funcName = symbolStackPop(symbolStack);
	if (funcName == NULL) {
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

		//look up class table in global scope table
		tSymbolPtr classSymbol = htabGetSymbol(globalScopeTable, className);
		strFree(className);
		if (classSymbol == NULL) {
			precedenceSymbolFree(funcName);
			return ERR_SEM;
		}

		//get pointer to classTable from classSymbol
		tHashTablePtr classTable = classSymbol->Data.ClassData.LocalSymbolTable;
		if (classTable == NULL) {
			precedenceSymbolFree(funcName);
			return ERR_INTERN;
		}

		//get function name from fullIdentifier
		dtStrPtr func;
		errCode = substr(funcName->stringOrId, strCharPos(funcName->stringOrId, '.') + 1, strGetLength(funcName->stringOrId), &func);
		precedenceSymbolFree(funcName);
		if (errCode != ERR_OK) {
			return errCode;
		}

		//look up function in current class table
		funcSymbol = htabGetSymbol(classTable, func);
		strFree(func);
		if (funcSymbol == NULL) {
			return ERR_SEM;
		}

	} else {

		//function have to be in current class
		funcSymbol = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, funcName->stringOrId);
		if (funcSymbol == NULL) {
			return ERR_SEM;
		}
		precedenceSymbolFree(funcName);

	}

	tInstruction instr = {iFRAME, NULL, funcSymbol, NULL};
	errCode =  instrListInsertInstruction(instructionList, instr);
	if (errCode == -1) {
		return ERR_INTERN;
	}

	int32_t paramCount = funcSymbol->Data.FunctionData.NumberOfArguments;
	tArgumentListItem* argument = funcSymbol->Data.FunctionData.ArgumentList;

	while (token->type != TT_rightRoundBracket) {

		errCode = parsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}

		if (paramCount > 0 ) {
			if (result->Type == eINT && argument->Symbol->Type == eDOUBLE) {
				//convert result from expressions parsing to double
				tSymbolPtr symbolTmp;
				convert(funcSymbol->Data.FunctionData.LocalSymbolTable, result, instr, symbolTmp, eDOUBLE);

				//add instr to instructionList
				errCode =  instrListInsertInstruction(instructionList, instr);
				if (errCode == -1) {
					return ERR_SEM_TYPE;
				}
				result = symbolTmp;
			}
			if (result->Type == argument->Symbol->Type) {

				instr.type = iPUSH;
				instr.dst = NULL;
				instr.arg1 = result;
				instr.arg2 = NULL;
				errCode =  instrListInsertInstruction(instructionList, instr);
				if (errCode == -1) {
					symbolFree(funcSymbol);
					return ERR_INTERN;
				}
				paramCount--;
				argument = argument->Next;
			} else {

				return ERR_SEM_TYPE;

			}

		} else {
			return ERR_SEM_TYPE;
		}

		//expressions parsing stoped on something else then right round bracket or comma - syntax error
		if (token->type != TT_rightRoundBracket && token->type != TT_comma) {
			return ERR_SYNTAX;
		}

	}

	instr.type = iCALL;
	instr.dst = NULL;
	instr.arg1 = NULL;
	instr.arg2 = NULL;

	errCode =  instrListInsertInstruction(instructionList, instr);
	if (errCode == -1) {
		return ERR_INTERN;
	}

	precedenceStackPush(stack, TT_E);

	tPrecedenceSymbolPtr funcPrecedenceSymbol = precedenceSymbolNew();
	funcPrecedenceSymbol->type = TT_E;

	tSymbolPtr symbolExprTmp;
	tmpVariable(funcSymbol->Data.FunctionData.LocalSymbolTable, symbolExprTmp, funcSymbol->Data.FunctionData.ReturnType)

	instr.type = iGETRETVAL;
	instr.dst = symbolExprTmp;
	instr.arg1 = NULL;
	instr.arg2 = NULL;

	errCode =  instrListInsertInstruction(instructionList, instr);
	if (errCode == -1) {
		precedenceSymbolFree(funcPrecedenceSymbol);
		return ERR_INTERN;
	}

	funcPrecedenceSymbol->symbol = symbolExprTmp;

	symbolStackPush(symbolStack, funcPrecedenceSymbol);
	precedenceSymbolFree(funcPrecedenceSymbol);

	cleanToken(&token);
	errCode = getToken(token);
	if (errCode != ERR_OK) {
		return errCode;
	}
	return ERR_OK;

}

eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	eError errCode;
	tHashTablePtr currentFuncTable = currentFunction->Data.FunctionData.LocalSymbolTable;
	if (currentFuncTable == NULL) {
	}

	switch(precedenceStackTopTerminal(stack)) {

		//E -> ID
		case TT_identifier: {

			if (precedenceStackPop(stack) != TT_identifier) {
				return ERR_INTERN;
			}

			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			//get precedence symbol representing current id from symbol stack
			tPrecedenceSymbolPtr id = symbolStackPop(symbolStack);

			//look up id in symbol table of current function scope
			tSymbolPtr symbolId;
			symbolId = htabGetSymbol(currentFuncTable, id->stringOrId);
			if (symbolId == NULL) {

				//id isn't in current function symbol table - but it still could be static id in current class symbol table
				symbolId = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, id->stringOrId);
				if (symbolId == NULL) {
					precedenceSymbolFree(id);
					return ERR_SEM;
				}
			}

			//precedence symbol containing pointer to current id in symbol table - push on symbol stack
			precedenceSymbolClean(id);
			id->type = TT_E;
			id->symbol = symbolNewCopy(symbolId);
			symbolStackPush(symbolStack, id);
			precedenceSymbolFree(id);

			//push type representing operand on precedence stack
			precedenceStackPush(stack, TT_E);
			break;
		}
		case TT_fullIdentifier: {

			if (precedenceStackPop(stack) != TT_fullIdentifier) {
				return ERR_INTERN;
			}

			if (precedenceStackPop(stack) != TT_start) {
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
				return ERR_SEM;
			}

			//get pointer to classTable from classSymbol
			tHashTablePtr classTable = classSymbol->Data.ClassData.LocalSymbolTable;
			if (classTable == NULL) {
				precedenceSymbolFree(fullId);
				return ERR_INTERN;
			}

			dtStrPtr id;
			errCode = substr(fullId->stringOrId, strCharPos(fullId->stringOrId, '.') + 1, strGetLength(fullId->stringOrId), &id);
			if (errCode != ERR_OK) {
				precedenceSymbolFree(fullId);
				strFree(className);
				return errCode;
			}

			tSymbolPtr symbolId;
			symbolId = htabGetSymbol(classTable, id);
			strFree(id);
			if (symbolId == NULL) {
				precedenceSymbolFree(fullId);
				return ERR_SEM;
			}

			precedenceSymbolClean(fullId);
			fullId->type = TT_E;
			fullId->symbol = symbolNewCopy(symbolId);
			symbolStackPush(symbolStack, fullId);

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
					return ERR_SYNTAX;
			}

			precedenceSymbolClean(constant);

			//store created constant to constTable
			constant->symbol = constInsertSymbol(constTable, *symbolConst);
			constant->type = TT_E;

			symbolStackPush(symbolStack, constant);
			precedenceSymbolFree(constant);
			free(symbolConst); //nemozem pouzit symbolFree, lebo ten zrusi aj nieco, co je v kontajneri konstant

			precedenceStackPush(stack, TT_E);
			break;

		// E -> ( E )
		case TT_rightRoundBracket:

			if (precedenceStackPop(stack) != TT_rightRoundBracket) {
				return ERR_INTERN;
			}
			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_leftRoundBracket) {
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			precedenceStackPush(stack, TT_E);
			break;

		// all E -> E op E
		case TT_minus:
		case TT_plus:
		case TT_multiply:
		case TT_divide:
		case TT_less:
		case TT_lessEqual:
		case TT_greater:
		case TT_greaterEqual:
		case TT_equal:
		case TT_notEqual: {

			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}

			int64_t operator = precedenceStackPop(stack);
			if (operator == -1 ) {
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand2 = symbolStackPop(symbolStack);
			tPrecedenceSymbolPtr operand1 = symbolStackPop(symbolStack);
			tSymbolPtr symbolExprTmp;
			tInstruction instr;

			if (operand1->symbol->Type == eSTRING) {
				if (operator == TT_plus) {
				if (operand2->symbol->Type != eSTRING) {
					//pretypovat 2. operand na string
					tSymbolPtr symbolTmp = symbolNew();
					convert(currentFuncTable, operand2->symbol, instr, symbolTmp, eSTRING);

					errCode =  instrListInsertInstruction(instructionList, instr);
						if (errCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						return ERR_SEM_TYPE;
					}
					operand2->symbol = symbolTmp;
				}
				//pomocna premenna typy string symbolTmp
				tmpVariable(currentFuncTable, symbolExprTmp, eSTRING)
				goto generateInstruction;

				} else {
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					return ERR_SEM_TYPE;
				}
			}
			if (operand2->symbol->Type == eSTRING) {
				if (operator == TT_plus) {
				if (operand1->symbol->Type != eSTRING) {
					//pretypovat 1. operand na string
					tSymbolPtr symbolTmp = symbolNew();
					convert(currentFuncTable, operand2->symbol, instr, symbolTmp, eSTRING);

					//pridat instrukciu na instrukcnu pasku
					errCode =  instrListInsertInstruction(instructionList, instr);
					if (errCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						return ERR_SEM_TYPE;
					}
					operand1->symbol = symbolTmp;
				}
				//pomocna premenna typu string
				tmpVariable(currentFuncTable, symbolExprTmp, eSTRING)

				goto generateInstruction;
			} else {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				return ERR_SEM_TYPE;
			}
			}

			if (operand1->symbol->Type == eINT) {
				if (operand2->symbol->Type == eINT) {
					//pomocna premenna typu int
					tmpVariable(currentFuncTable, symbolExprTmp, eINT)

					goto generateInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {

					//generovat instrukciu na prevod prveho operandu na double
					tSymbolPtr symbolTmp;
					convert(currentFuncTable, operand1->symbol, instr, symbolTmp, eDOUBLE);

					//pridat instrukciu na instrukcnu pasku
					errCode =  instrListInsertInstruction(instructionList, instr);
					if (errCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						return ERR_SEM_TYPE;
					}
					operand1->symbol = symbolTmp;

					//pomocna premenna typu double
					tmpVariable(currentFuncTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else {
					//uvolnit vsetko
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					return ERR_SEM_TYPE;
				}
			} else if (operand1->symbol->Type == eDOUBLE) {
				if (operand2->symbol->Type == eINT) {

					//generovat instrukciu na prevod druheho operandu na double
					tSymbolPtr symbolTmp;
					convert(currentFuncTable, operand2->symbol, instr, symbolTmp, eDOUBLE);

					//pridat instrukciu na instrukcnu pasku
					errCode =  instrListInsertInstruction(instructionList, instr);
					if (errCode == -1) {
						precedenceSymbolFree(operand1);
						precedenceSymbolFree(operand2);
						return ERR_SEM_TYPE;
					}
					operand2->symbol = symbolTmp;

					//pomocna premenna typu double
					tmpVariable(currentFuncTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else if (operand2->symbol->Type == eDOUBLE) {
					//pomocna premenna typu double
					tmpVariable(currentFuncTable, symbolExprTmp, eDOUBLE)

					goto generateInstruction;
				} else {
					//uvolnit vsetko
					precedenceSymbolFree(operand1);
					precedenceSymbolFree(operand2);
					return ERR_SEM_TYPE;
				}
			} else {
				//uvolnit vsetko
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
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
					return ERR_SYNTAX;
			}

			instr.type = instruction;
			instr.dst = symbolExprTmp;
			instr.arg1 = operand1->symbol;
			instr.arg2 = operand2->symbol;

			//pridat na instrukcnu pasku
			errCode =  instrListInsertInstruction(instructionList, instr);
			if (errCode == -1) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand1);
			precedenceSymbolFree(operand2);

			//na symbolStack ulozit ukazatel na vyslednu hodnotu
			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			symbolStackPush(symbolStack, result);
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;
			}
		}

		case TT_and:
		case TT_or: {

			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}

			int64_t operator = precedenceStackPop(stack);
			if (operator != TT_and && operator != TT_or) {
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}

			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand2 = symbolStackPop(symbolStack);
			tPrecedenceSymbolPtr operand1 = symbolStackPop(symbolStack);

			if (operand1->symbol->Type != eBOOL || operand2->symbol->Type != eBOOL) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				return ERR_SEM_TYPE;
			}

			//pomocna premenna typu bool
			tSymbolPtr symbolExprTmp;
			tmpVariable(currentFuncTable, symbolExprTmp, eBOOL)

			tInstruction instr;

			if (operator == TT_and) {
				instr.type = iLAND;
				instr.dst = symbolExprTmp;
				instr.arg1 = operand1->symbol;
				instr.arg2 = operand2->symbol;
			}
			if (operator == TT_or) {
				instr.type = iLOR;
				instr.dst = symbolExprTmp;
				instr.arg1 = operand1->symbol;
				instr.arg2 = operand2->symbol;
			}

			errCode =  instrListInsertInstruction(instructionList, instr);
			if (errCode == -1) {
				precedenceSymbolFree(operand1);
				precedenceSymbolFree(operand2);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand1);
			precedenceSymbolFree(operand2);

			//na symbolStack ulozit ukazatel na vyslednu hodnotu
			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			symbolStackPush(symbolStack, result);
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;
		}

		// E -> ! E
		case TT_not:

			if (precedenceStackPop(stack) != TT_E) {
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_not) {
				return ERR_SYNTAX;
			}
			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			tPrecedenceSymbolPtr operand = symbolStackPop(symbolStack);
			if (operand->symbol->Type != eBOOL) {
				precedenceSymbolFree(operand);
				return ERR_SEM_TYPE;
			}

			tSymbolPtr symbolExprTmp;
			tmpVariable(currentFuncTable, symbolExprTmp, eBOOL);

			tInstruction instr = {iLNOT, symbolExprTmp, operand->symbol, NULL};
			//vlozenie instrukcie na zasobnik instrukcii
			errCode =  instrListInsertInstruction(instructionList, instr);
			if (errCode == -1) {
				precedenceSymbolFree(operand);
				return ERR_INTERN;
			}

			precedenceSymbolFree(operand);

			//na symbolStack ulozit ukazatel na vyslednu hodnotu
			tPrecedenceSymbolPtr result = precedenceSymbolNew();
			result->type = TT_E;
			result->symbol = symbolExprTmp;
			symbolStackPush(symbolStack, result);
			precedenceSymbolFree(result);

			precedenceStackPush(stack, TT_E);
			break;

		// E -> ++ E
		// E -> -- E
		// E -> E ++
		// E -> E --
		// E -> - E
		case TT_increment:
		case TT_decrement:

			printf("unary not supported yet\n");
			return ERR_SYNTAX;

		default:
			return ERR_SYNTAX;
	}

	return ERR_OK;

}

//entry point of expressions parsing
eError precedenceParsing(Token* helpToken) {

	tmpString = strNew();
	strAddChar(tmpString, '@');
	tmpInt = strNew();
	strAddChar(tmpInt, '#');
	tmpDouble = strNew();
	strAddChar(tmpDouble, '.');
	tmpBool = strNew();
	strAddChar(tmpBool, '0');

	eError errCode = parsing(helpToken);

	strFree(tmpString);
	strFree(tmpInt);
	strFree(tmpDouble);
	strFree(tmpBool);

	return errCode;

}

eError parsing(Token* helpToken) {


	tPrecedenceStackPtr stack;
	if ((stack = precedenceStackNew()) == NULL) {
		printError(ERR_INTERN, "Cannot create precedence stack.\n");
		return ERR_INTERN;
	}
	precedenceStackPush(stack, TT_dolar);

	tPrecedenceSymbolPtr symbol;
	if ((symbol = precedenceSymbolNew()) == NULL) {
		precedenceStackFree(stack);
		printError(ERR_INTERN, "Cannot create precedence symbol.\n");
		return ERR_INTERN;
	}

	tSymbolStackPtr symbolStack;
	if ((symbolStack = symbolStackNew()) == NULL) {
		precedenceSymbolFree(symbol);
		precedenceStackFree(stack);
		printError(ERR_INTERN, "Cannot create precedence symbol stack.\n");
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

		precedenceStackPush(stack, helpToken->type);
		precedenceStackShift(stack);
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
		symbolStackPush(symbolStack, symbol);
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

		if (token->type > NONTERMINALBORDER) {
			errCode = ERR_SYNTAX;
			goto freeAndExit;
		}

		stackTop = precedenceStackTopTerminal(stack);
		printf("stackTop:%ld, token->type: %d\n", stackTop, token->type);
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
				if (token->type < TT_increment && token->type > TT_notEqual) {//TODO: bude sa mozno este menit

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

					symbolStackPush(symbolStack, symbol);

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
					goto freeAndExit;
				}
				continue;

			case 'f':

				errCode = functionParse(stack, symbolStack);
				if (errCode != ERR_OK) {
					goto freeAndExit;
				}
				break;

			case 'u':
				printf("unary not supported yet\n");
				errCode = ERR_SYNTAX;
				goto freeAndExit;

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

				errCode = ERR_SYNTAX;
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
