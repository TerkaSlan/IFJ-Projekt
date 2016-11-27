//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser_second.h"
#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "builtin.h"
#include "ial.h"
#include "instruction.h"
#include "token.h"

// Sets error, prints message with line number
#define EXIT(err, ...) do{errCode = err; printError(err, "Line: %lu - %s\n", (unsigned long)LineCounter, __VA_ARGS__);}while(0)
#define CHECK_ERRCODE() if (errCode != ERR_OK) return errCode

//Add Instruction
#define AI(type, dst, arg1, arg2) do{ tInstruction instr = {type, dst, arg1, arg2}; int64_t index = instrListInsertInstruction(instructionList, instr); if(index < 0) {printError(ERR_INTERN, "Could not insert next instruction. Out of memory.\n"); return ERR_INTERN;} }while(0)
//Add Instruction with output - sets index of inserted instruction to output
#define AIwO(type, dst, arg1, arg2, output) do{ tInstruction instr = {type, dst, arg1, arg2}; int64_t index = instrListInsertInstruction(instructionList, instr);if(index < 0) {printError(ERR_INTERN, "Could not insert next instruction. Out of memory.\n"); return ERR_INTERN;} else output = (uint32_t)index; }while(0)

/*
	Functions are similar to parser's counterparts, but instead of inserting
	symbols into symbol table and checking for syntactic errors they're adjusted
	to generate instructions.
*/
static eError classList_2();

static eError classBody_2();

static eError funcBody_2();

static eError stmt_2();

static eError stmtBody_2();

static eError var_2();

//static eInstructionType getFunctionCallForBuiltin(dtStrPtr *string); //TODO::where is it used??
static tSymbolPtr findSymbol(dtStrPtr symbolName);


/* Instruction tape for preinterpretation needed in specific case when there is
   static variable with initialization which needs to be done before interpretation */
extern tInstructionListPtr preInstructionList;
// Main instruction tape for every instruction used in interpretation
extern tInstructionListPtr instructionList;
//extern tConstContainerPtr constTable; TODO:: wat???
// Main table holding all other class tables
extern tHashTablePtr       globalScopeTable;

Token             *token;
// Pointer to current function I'm in
tSymbolPtr        currentFunction;
// Pointer to current class I'm in
tSymbolPtr        currentClass;
// Symbol returned from expr.c as a result of expression evaluation
tSymbolPtr        result;
// String holding identifier when needed throghout the module
static dtStrPtr   symbolName;
// result of findSymbol()
static tSymbolPtr foundSymbol;
/* Flag available to expr module so that it knows when to generate instructions
   on preInstructionList as opposed to instructionList where it generates
	 instructions most of the time.*/
bool preinterpretation = false;

#define checkTypesAndGenerateiMOV(errCode)\
do{\
    if (!result)\
        {EXIT(ERR_SYNTAX, "Expression expected."); return ERR_SYNTAX;}\
    foundSymbol = findSymbol(symbolName);\
    /*The only time different types are allowed is implicit conversion when dst is double and src is int.
	  Interpret deals with the conversion*/        \
    if (result->Type == foundSymbol->Type || (result->Type == eINT && foundSymbol->Type  == eDOUBLE)){\
        AI(iMOV, foundSymbol, result, NULL);\
    }\
    else{\
        EXIT(ERR_SEM_TYPE, "Assigning of unconvertible types.");\
        return ERR_SEM_TYPE;\
    }\
}while(0)

#define getNewToken(token, errCode)\
do{\
    cleanToken(&token);                            \
    errCode = getToken(token);                    \
    CHECK_ERRCODE();                            \
} while (0)

eError initializeHelperVariables_2() {
	eError errCode = ERR_OK; //so I can use macro exit
	if((token = newToken()) == NULL) {
		EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
		return ERR_INTERN;
	}
	if((symbolName = strNew()) == NULL) {
		freeToken(&token);
		EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
	}

	return errCode;
}

void freeHelperVariables_2() {
	freeToken(&token);
	strFree(symbolName);
}

/**
 *  \brief Checks if symbol with given name exists in symbol table
 *	\param symbolName Name to be checked for
 *  \return Pointer to the symbol found in symbol table on success, NULL on failure
 */
tSymbolPtr findSymbol(dtStrPtr symbolName) {
	tSymbolPtr symbolAdept;
	int32_t    dotPosition = strCharPos(symbolName, '.');

	// Full Identifier -> needs to be split into class and simple id and checked for existence of both
	if(dotPosition != (-1)) {
		dtStrPtr   functionName, className;
		tSymbolPtr classSymbol, functionSymbol;

		if(substr(symbolName, dotPosition + 1, strGetLength(symbolName), &functionName) != ERR_OK)
			return NULL;
		if(substr(symbolName, 0, dotPosition, &className) != ERR_OK)
			return NULL;

		if((classSymbol = htabGetSymbol(globalScopeTable, className)) == NULL)
			return NULL;
		else {
			if((functionSymbol = htabGetSymbol(classSymbol->Data.ClassData.LocalSymbolTable, functionName)) == NULL)
				return NULL;
			else {
				return functionSymbol;
			}
		}

	}
		// Simple Identifier
	else {
		if((symbolAdept = htabGetSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, symbolName)) == NULL) {
			if((symbolAdept = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, symbolName)) == NULL) {
				return NULL;
			} else {
				// I'm in currentClass, not in currentFunction
				return symbolAdept;
			}
		} else {
			return symbolAdept;
		}
	}
	return NULL;
}

/*
 * Prepares source file for reading, initiates the second run of parser
 */
eError generateInstructions() {

	eError errCode = ERR_OK;
	if((errCode = initializeHelperVariables_2()) != ERR_OK)
		return errCode;
	rewind(fSourceFile);
	LineCounter = 1;

	// Ignoring syntactic checks, already done by 1. run
	errCode = getToken(token);
	if(errCode != ERR_OK)
		goto freeResourcesAndFinish_2;

	errCode = classList_2();
	if(errCode != ERR_OK)
		goto freeResourcesAndFinish_2;

	//at the end add iRET instruction to the preInstructionList
	{
		tInstruction instr = {iRET, NULL, NULL, NULL};
		if(instrListInsertInstruction(preInstructionList, instr) < 0)
		{
			EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
			goto freeResourcesAndFinish_2;
		}
	}

	//if(token->type == TT_EOF)
		//then wat?

	freeResourcesAndFinish_2:
	freeHelperVariables_2();

	return errCode;
}

/**
 *  \brief Handles instruction generation for symbols present on global level
 *  \return ERR_OK on success, any other eError on failure
 */
eError classList_2() {

	eError errCode = ERR_OK;
	getNewToken(token, errCode);

	// currentClass and currentFunction variables are not changed, but have to be updated as i go along with 2nd run
	currentClass = htabGetSymbol(globalScopeTable, token->str);
	getNewToken(token, errCode);
	getNewToken(token, errCode);
	if(token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		CHECK_ERRCODE();
	}
	if(token->type == TT_rightCurlyBracket) {
		getNewToken(token, errCode);
		if(token->type == TT_keyword && token->keywordType == KTT_class) {
			errCode = classList_2();
			CHECK_ERRCODE();
		}
		return errCode;
	}
	return errCode;
}

/**
 *  \brief Handles instruction generation for symbols present on class level
 *  \return ERR_OK on success, any other eError on failure
 */
eError classBody_2() {
	eError errCode = ERR_OK;
	//[<TT_keyword>]
	getNewToken(token, errCode);
	//[<TT_identifier>]
	getNewToken(token, errCode);

	if(strCopyStr(symbolName, token->str) == STR_ERROR) {
		EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
		return ERR_INTERN;
	}


	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// Static variable without initialization
			getNewToken(token, errCode);
			break;

		case TT_assignment:
			// Static variable with initialization, calling expressions and generating iMOV on preInstructionList
			preinterpretation = true;
			errCode           = precedenceParsing(NULL);
			preinterpretation = false;
			CHECK_ERRCODE();
			if(!result) {
				EXIT(ERR_SYNTAX, "Expression expected.");
				return ERR_SYNTAX;
			}
			if((foundSymbol = findSymbol(symbolName)) == NULL) {
				EXIT(ERR_SEM, "Unresolved symbol.");
				return ERR_SEM;
			}

			if(foundSymbol->Type != result->Type && !(foundSymbol->Type == eDOUBLE && result->Type == eINT))
			{
				EXIT(ERR_SEM_TYPE, "Assigning of unconvertible types.");
				return ERR_SEM_TYPE;
			}

			{
				tInstruction instr = {iMOV, foundSymbol, result, NULL};
				if(instrListInsertInstruction(preInstructionList, instr) < 0)
				{
					EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
					return ERR_INTERN;
				}
			}

			getNewToken(token, errCode);
			break;

		case TT_leftRoundBracket:
			// Function definition
			currentFunction = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, symbolName);
			getNewToken(token, errCode);
			strClear(symbolName);

			while(token->type != TT_rightRoundBracket) {
				getNewToken(token, errCode);
				if(token->type == TT_EOF)
					return ERR_SYNTAX;

				// Incoming parameters in function definition, don't care in 2.run
				getNewToken(token, errCode);
				getNewToken(token, errCode);
				if(token->type == TT_comma)
					getNewToken(token, errCode);
			}
			//[leftCurlyBracket]
			getNewToken(token, errCode);
			getNewToken(token, errCode);

			//set first instruction of current function
			currentFunction->Data.FunctionData.InstructionIndex = instrListGetNextInsertedIndex(instructionList);

			if(token->type != TT_rightCurlyBracket) {
				// not an empty function, going to funcBody

				errCode = funcBody_2();
				CHECK_ERRCODE();

			}

			//checking for correct return value
			{
				uint32_t lastInstructionInsertedIndex = instrListGetNextInsertedIndex(instructionList) - 1; //returns only > 0 - no underflow
				tInstructionPtr instr = instrListGetInstruction(instructionList, lastInstructionInsertedIndex);

				if(instr->type != iRET) {
					if(currentFunction->Data.FunctionData.ReturnType == eNULL) {
						//OK - void type
						AI(iRET, NULL, NULL, NULL);
					}
					else{
						EXIT(ERR_SEM, "Non void function must return a value.");
						return ERR_SEM;
					}
				}


				//Insert padding after each function for the return type detection to be even possible - like INT3
				AI(iINT, NULL, NULL, NULL);

			}


			getNewToken(token, errCode);
			break;

		default:
			return ERR_SYNTAX;
	}
	if(token->type == TT_rightCurlyBracket)
		return ERR_OK;

	// next token is KTT_static, recursively calling classBody_2 again
	if(token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		CHECK_ERRCODE();
	}
	return errCode;
}

/**
 *  \brief Handles instruction generation for symbols present on function level
 *  \return ERR_OK on success, any other eError on failure
 */
eError funcBody_2() {
	eError errCode = ERR_OK;

	do {
		if(token->type == TT_keyword
		   && (token->keywordType == KTT_boolean
		       || token->keywordType == KTT_double
		       || token->keywordType == KTT_int
		       || token->keywordType == KTT_String)) {
			//Keyword, calling var() with parameter true as for 'defined'
			errCode = var_2();
			CHECK_ERRCODE();
			getNewToken(token, errCode);
		} else {
			//current token is a keyword, but it isn't type - this could be STMT - we call stmt()
			errCode = stmt_2();
			CHECK_ERRCODE();
		}


	}while(token->type != TT_rightCurlyBracket);

	return errCode;
}

eError stmtBody_2(){
	eError errCode = ERR_OK;
	getNewToken(token, errCode);
	while (token->type != TT_rightCurlyBracket) {
		if (token->type == TT_keyword
		    && (token->keywordType == KTT_boolean
		        || token->keywordType == KTT_double
		        || token->keywordType == KTT_int
		        || token->keywordType == KTT_String)) {

			//trying to declare a new variable inside a scope
			EXIT(ERR_SEM_OTHER, "Declarations in scope are not permitted.\n");
			return ERR_SEM_OTHER;
		} else {
			//current token isnt a keyword or isnt a type - this could be STMT - we call stmt()
			errCode = stmt_2();
			CHECK_ERRCODE();
		}
	}
	return errCode;
}




/**
 *  \brief Handles instruction generation for statements
 *  \return ERR_OK on success, any other eError on failure
 */
eError stmt_2() {

	eError errCode = ERR_OK;
	//RULE: stmt_2 -> { stmt_2_LIST }
	switch(token->type) {

		case TT_leftCurlyBracket:
			errCode = stmtBody_2();
			CHECK_ERRCODE();
			getNewToken(token, errCode);
			break;

		case TT_keyword:
			switch(token->keywordType) {

				//RULE: stmt_2 -> return RETURN_VAL ;
				case KTT_return:
					//we call expressions parsing to parse RETURN_VAL
					errCode = precedenceParsing(NULL);
					CHECK_ERRCODE();


					//type checking
					if(result == NULL)
					{
						//return void
						if(currentFunction->Data.FunctionData.ReturnType != eNULL)
						{
							EXIT(ERR_SEM, "Function must return a value.");
							return ERR_SEM;
						}
					}
					else
					{
						//return value
						if(currentFunction->Data.FunctionData.ReturnType != result->Type && !(currentFunction->Data.FunctionData.ReturnType == eDOUBLE && result->Type == eINT))
						{
							EXIT(ERR_SEM_TYPE, "Returning unconvertible type.");
							return ERR_SEM_TYPE;
						}
					}

					AI(iRET, NULL, result, NULL);
					//expressions parsing could stop on semicolon or right round bracket before semicolon
					//otherwise, there is syntax error
					if(token->type != TT_semicolon) {
						getNewToken(token, errCode);
					}

					getNewToken(token, errCode);
					break;

					//stmt_2 -> if ( EXPR ) stmt_2 ELSE
				case KTT_if: {
					//next token have to be left round bracket
					getNewToken(token, errCode);

					//call expressions parsing - parse condition
					errCode = precedenceParsing(NULL);
					CHECK_ERRCODE();

					if(!result) {
						EXIT(ERR_SYNTAX, "Expression expected.");
						CHECK_ERRCODE();
					}

					if(result->Type != eBOOL) {
						EXIT(ERR_SEM_TYPE, "Expression cannot be converted to boolean.");
						CHECK_ERRCODE();
					}


					uint32_t indexIF;
					AIwO(iIFNGOTO, NULL, result, NULL, indexIF);


					//read next token and call stmt_2 processing
					getNewToken(token, errCode);
					errCode = stmtBody_2();
					CHECK_ERRCODE();

					//after SMTM - there can be else

					uint32_t indexFalse = instrListGetNextInsertedIndex(instructionList);

					getNewToken(token, errCode); //else check
					if(token->type == TT_keyword && token->keywordType == KTT_else) {

						//insert jump over false block, dst is to be refined later
						uint32_t indexGOTO;
						AIwO(iGOTO, NULL, NULL, NULL, indexGOTO);
						indexFalse++;

						getNewToken(token, errCode); // {
						errCode = stmt_2();
						CHECK_ERRCODE();

						//Refine where is it that we should jump on finished true block (over false block)
						tInstructionPtr igoto = instrListGetInstruction(instructionList, indexGOTO);
						igoto->dst = (void *) instrListGetNextInsertedIndex(instructionList);

						getNewToken(token, errCode); // next after }
					}

					//Refine where is it that we should jump on false condition
					tInstructionPtr iIFN = instrListGetInstruction(instructionList, indexIF);
					iIFN->dst = (void *) indexFalse;

					break;
				}

					//stmt_2 -> while ( EXPR ) stmt_2
				case KTT_while: {
					getNewToken(token, errCode);


					//Get index of condition
					uint32_t indexCond = instrListGetNextInsertedIndex(instructionList);


					//get condition
					errCode = precedenceParsing(NULL);
					CHECK_ERRCODE();

					//{
					getNewToken(token, errCode);

					if(!result) {
						EXIT(ERR_SYNTAX, "Expression expected.");
						CHECK_ERRCODE();
					}

					if(result->Type != eBOOL) {
						EXIT(ERR_SEM_TYPE, "Expression cannot be converted to boolean.");
						CHECK_ERRCODE();
					}

					//on false condition jump over while body - to be refined later
					uint32_t indexIF;
					AIwO(iIFNGOTO, NULL, result, NULL, indexIF);

					//start while body
					errCode = stmtBody_2();
					CHECK_ERRCODE();

					getNewToken(token, errCode); // next after }

					//add jump back to the condition
					uint32_t whileEndBodyIndex;
					AIwO(iGOTO, (void *) indexCond, NULL, NULL, whileEndBodyIndex);

					////Refine where is it that we should jump on false condition - after while body
					tInstructionPtr condition = instrListGetInstruction(instructionList, indexIF);
					condition->dst = (void *) (whileEndBodyIndex + 1);

					break;
				}

				default:
					EXIT(ERR_SEM, "Statment expected.\n");
					return ERR_SEM;
			}
			break;

			// NOT YET IMPLEMENTED IN EXPR.C
/*		case TT_increment:
		case TT_decrement:
			errCode = precedenceParsing(NULL);
      printf("PrecedenceParsing returned: %d\n", errCode);
			CHECK_ERRCODE();

			getNewToken(token, errCode);
			break;
*/

			//not a keyword
		case TT_identifier:
		case TT_fullIdentifier: {
			Token *helperToken;
			if((helperToken = newToken()) == NULL)
			{
				EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
				return ERR_INTERN;
			}
			helperToken->type = token->type;
			if((helperToken->str  = strNewFromStr(token->str)) == NULL)
			{
				EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
				freeToken(&helperToken);
				return ERR_INTERN;
			}

			if(strCopyStr(symbolName, token->str) != ERR_OK) {
				freeToken(&helperToken);
				EXIT(ERR_INTERN, "Error allocating new space. Out of memory.");
				return ERR_INTERN;
			}

			getNewToken(token, errCode);

			if(token->type == TT_assignment) {
				freeToken(&helperToken);
				errCode = precedenceParsing(NULL);
				CHECK_ERRCODE();
				checkTypesAndGenerateiMOV(errCode);


			} else if(token->type == TT_leftRoundBracket || token->type == TT_increment ||
			          token->type == TT_decrement) {

				errCode = precedenceParsing(helperToken); //TODO::empty func call returns eNULL!
				freeToken(&helperToken);
				CHECK_ERRCODE();
			}

			getNewToken(token, errCode);
			break;
		}

		default:
			EXIT(ERR_SYNTAX, "Unexpected token.\n");
			return ERR_SYNTAX;
	}

	return ERR_OK;

}

eError var_2() {

	eError errCode = ERR_OK;
	getNewToken(token, errCode);
	strCopyStr(symbolName, token->str);
	getNewToken(token, errCode);

	if(token->type == TT_assignment) {
		errCode = precedenceParsing(NULL);
		CHECK_ERRCODE();
		checkTypesAndGenerateiMOV(errCode);
	}
	if(token->type != TT_semicolon) { //TODO::precedence parsing may stop on )??
		EXIT(ERR_SYNTAX, "Assignment expected. Missing ';'?");
		return ERR_SYNTAX;
	}

	strClear(symbolName);
	return ERR_OK;
}
