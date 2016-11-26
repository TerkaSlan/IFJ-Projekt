//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser_second.h"
#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "interpret.h"
#include "builtin.h"
#include "instruction.h"

// Sets error, prints message with line number
#define EXIT(err, ...) do{errCode = err; printError(err, "Line: %lu - %s\n", (unsigned long)LineCounter, __VA_ARGS__);}while(0)
#define CHECK_ERRCODE() if (errCode != ERR_OK) return errCode

//Add Instruction
#define AI(type, dst, arg1, arg2) do{ tInstruction instr = {type, dst, arg1, arg2}; int64_t index = instrListInsertInstruction(instructionList, instr); if(index < 0) {printError(ERR_INTERN, "Could not insert next instruction. Out of memory.\n"); return ERR_INTERN;} }while(0)
//Add Instruction with output - sets index of inserted instruction to output
#define AIwO(type, dst, arg1, arg2, output) do{ tInstruction instr = {type, dst, arg1, arg2}; int64_t index = instrListInsertInstruction(instructionList, instr);if(index < 0) {printError(ERR_INTERN, "Could not insert next instruction. Out of memory.\n"); return ERR_INTERN;} else output = (uint32_t)index; }while(0)

eError classList_2();
eError classBody_2();
eError funcBody_2();
eError stmt_2();
eError var_2();
eInstructionType getFunctionCallForBuiltin(dtStrPtr *string);
tSymbolPtr findSymbol(dtStrPtr symbolName);

extern uint32_t LineCounter;
extern tInstructionListPtr preInstructionList;
extern tInstructionListPtr instructionList;
extern tConstContainerPtr constTable;
extern tHashTablePtr globalScopeTable;
Token *token;
extern tSymbolPtr currentFunction;
extern tSymbolPtr currentClass;
tSymbolPtr result;
tInstruction instr;
dtStrPtr symbolName;
tSymbolPtr foundSymbol;

#define checkTypesAndGenerateiMOV(errCode)\
do{\
	foundSymbol = findSymbol(symbolName);\
	if (result->Type == foundSymbol->Type){\
		AI(iMOV, foundSymbol, result, NULL);\
		errCode = ERR_OK;\
	}\
	else{\
		errCode = ERR_SEM_TYPE;\
	}\
}while(0)

#define getNewToken(token, errCode)\
do{\
	cleanToken(&token);							\
	errCode = getToken(token);					\
	CHECK_ERRCODE();							\
} while (0)

eError initializeHelperVariables_2(){
	if ((token = newToken()) == NULL){
    return ERR_INTERN;
    }
	if ((symbolName = strNew()) == NULL){
		freeToken(&token);
		return ERR_INTERN;
	}
	if ((preInstructionList = instrListNew()) == NULL){
		freeToken(&token);
		strFree(symbolName);
		return ERR_INTERN;
	}
	return ERR_OK;
}

void freeHelperVariables_2(){
  freeToken(&token);
  strFree(symbolName);
}

tSymbolPtr findSymbol(dtStrPtr symbolName){
  eError errCode;
  tSymbolPtr symbolAdept;
  if (strCharPos(symbolName, '.') != (-1)){
    // find out what is the class's name
    dtStrPtr functionName, className;
    tSymbolPtr classSymbol, functionSymbol;

    if ((errCode = substr(symbolName, strCharPos(symbolName, '.') + 1, strGetLength(symbolName), &functionName)) != ERR_OK)
      return NULL;
    if ((errCode = substr(symbolName, 0, strCharPos(symbolName, '.'), &className)) != ERR_OK)
      return NULL;

    if((classSymbol = htabGetSymbol(globalScopeTable, className)) == NULL)
      return NULL;
    else{
      if((functionSymbol = htabGetSymbol(classSymbol->Data.ClassData.LocalSymbolTable, functionName)) == NULL)
        return NULL;
      else{
        return functionSymbol;
      }
    }

  }
  else{
    if ((symbolAdept = htabGetSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, symbolName)) == NULL){
      if ((symbolAdept = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, symbolName)) == NULL){
        return NULL;
      }
      else{
        // I'm in currentClass, not in currentFunction
        return symbolAdept;
      }
    }
    else{
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
  if ((errCode = initializeHelperVariables_2()) != ERR_OK)
    return errCode;
	rewind(fSourceFile);
	LineCounter = 0;

  // Ignoring syntactic checks, already done by 1. run
	errCode = (eError)getToken(token);
  if (errCode != ERR_OK)
    goto freeResourcesAndFinish_2;

	errCode = classList_2();

	if (token->type == TT_EOF)
    goto freeResourcesAndFinish_2;

	freeResourcesAndFinish_2:
  freeHelperVariables_2();

  return errCode;
}

eError classList_2() {

	eError errCode = ERR_OK;
	getNewToken(token, errCode);

  // currentClass and currentFunction variables are not changed, but have to be updated as i go along with 2nd run
  currentClass = htabGetSymbol(globalScopeTable, token->str);
	getNewToken(token, errCode);
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		CHECK_ERRCODE();
	}
	if (token->type == TT_rightCurlyBracket){
		getNewToken(token, errCode);
		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			errCode = classList_2();
			CHECK_ERRCODE();
		}
		return errCode;
	}
  return errCode;
}

eError classBody_2() {
	eError errCode = ERR_OK;
  //[<TT_keyword>]
	getNewToken(token, errCode);
  //[<TT_identifier>]
	getNewToken(token, errCode);

  if(strCopyStr(symbolName, token->str) == STR_ERROR)
    return ERR_INTERN;

	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// STATIC! variable without initialization

			getNewToken(token, errCode);
			break;

		case TT_assignment:
      // STATIC! variable with initialization, calling expressions and generating iMOV
			errCode = precedenceParsing(NULL);
			CHECK_ERRCODE();
      if((foundSymbol = findSymbol(symbolName)) == NULL)
        return ERR_SEM;
      {tInstruction instr = {iMOV, foundSymbol, result, NULL}; instrListInsertInstruction(preInstructionList, instr);}

			getNewToken(token, errCode);
			break;

		case TT_leftRoundBracket:
      currentFunction = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, symbolName);
      //trying out instructionList update without a flag
      getNewToken(token, errCode);
      // no iFRAME
      strClear(symbolName);

			while (token->type != TT_rightRoundBracket) {
        getNewToken(token, errCode);
        if (token->type == TT_EOF)
          return ERR_SYNTAX;

        // Incoming parameters in function definition, don't care in 2.run
        getNewToken(token, errCode);
        getNewToken(token, errCode);
        if (token->type == TT_comma)
          getNewToken(token, errCode);
			}
      //[leftCurlyBracket]
 			getNewToken(token, errCode);
			getNewToken(token, errCode);
			if (token->type != TT_rightCurlyBracket){
        // not an empty function, going to funcBody
				errCode = funcBody_2();
				CHECK_ERRCODE();
			}
      else{
        // Empty function, the only instruction is iRET
        AI(iRET, NULL, NULL, NULL);
        //currentFunction->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1; ??
      }
			getNewToken(token, errCode);
			break;

		default:
			return ERR_SYNTAX;
	}
	if (token->type == TT_rightCurlyBracket)
		return ERR_OK;
	// next token has to be KTT_static, recursively calling classBody_2 again
	if (token->type == TT_keyword && token->keywordType == KTT_static){
		errCode = classBody_2();
		CHECK_ERRCODE();
	}
	return ERR_OK;
}

eError funcBody_2() {
	eError errCode = ERR_OK;
	currentFunction->Data.FunctionData.InstructionIndex =  1;
	if (token->type == TT_keyword) {
    if (token->keywordType == KTT_boolean
     || token->keywordType == KTT_double
     || token->keywordType == KTT_int
     || token->keywordType == KTT_String) {
			//Keyword, calling var() with parameter true as for 'defined'
		 	errCode = var_2();
		 	CHECK_ERRCODE();
		 	getNewToken(token, errCode);
    }
    else{
      //current token is a keyword, but it isn't type - this could be STMT - we call stmt()
      errCode = stmt_2();
      CHECK_ERRCODE();
    }
	}
  else{
    //current token is anything but keyword - this could be STMT - we call stmt()
    errCode = stmt_2();
    CHECK_ERRCODE();
  }

	if (token->type != TT_rightCurlyBracket) {
		errCode = funcBody_2();
		CHECK_ERRCODE();
	}
	return errCode;
}

eError stmt_2() {

	eError errCode = ERR_OK;
  //RULE: stmt_2 -> { stmt_2_LIST }
	switch(token->type) {

		case TT_leftCurlyBracket:

			getNewToken(token, errCode);
			//If there is something between CurlyBrackets, we need to process it recursively
			if (token->type != TT_rightCurlyBracket) {
				errCode = stmt_2();
				CHECK_ERRCODE();
			}
			if (token->type == TT_rightCurlyBracket) {
				//read next token - at the end of the function we will determine what to do next
				getNewToken(token, errCode);
				break;
			}

			return ERR_SYNTAX;

		case TT_keyword:
			switch(token->keywordType){

				//RULE: stmt_2 -> return RETURN_VAL ;
				case KTT_return:
					//we call expressions parsing to parse RETURN_VAL
					errCode = precedenceParsing(NULL);
                    printf("PrecedenceParsing returned: %d\n", errCode);
					CHECK_ERRCODE();

                    AI(iRET, NULL , result, NULL);

					//expressions parsing could stop on semicolon or right round bracket before semicolon
					//otherwise, there is syntax error
					if (token->type != TT_semicolon) {
						getNewToken(token, errCode);
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//stmt_2 -> if ( EXPR ) stmt_2 ELSE
				case KTT_if: {
					//next token have to be left rounf bracket
					getNewToken(token, errCode);

					//call expressions parsing - parse condition
					errCode = precedenceParsing(NULL);
					printf("PrecedenceParsing returned: %d\n", errCode);
					CHECK_ERRCODE();

					if(!result) {
						EXIT(ERR_SYNTAX, "Expression expected.");
						CHECK_ERRCODE();
					}

					if(result->Type != eBOOL){
						EXIT(ERR_SEM_TYPE, "Expression cannot be converted to boolean.");
						CHECK_ERRCODE();
					}


					uint32_t indexIF;
					AIwO(iIFNGOTO, NULL, result, NULL, indexIF);


					//read next token and call stmt_2 processing
					getNewToken(token, errCode);
					errCode = stmt_2();
					CHECK_ERRCODE();

					//after SMTM - there can be else

					uint32_t indexFalse = instrListGetNextInsertedIndex(instructionList);

					if(token->type == TT_keyword && token->keywordType == KTT_else) {

						//insert jump over false block, dst is to be refined later
						uint32_t indexGOTO;
						AIwO(iGOTO, NULL, NULL, NULL, indexGOTO);
						indexFalse++;

						getNewToken(token, errCode);
						errCode = stmt_2();
						CHECK_ERRCODE();

						//Refine where is it that we should jump on finished true block (over false block)
						tInstructionPtr igoto = instrListGetInstruction(instructionList, indexGOTO);
						igoto->dst = (void *) instrListGetNextInsertedIndex(instructionList);
					}

					//Refine where is it that we should jump on false condition
					tInstructionPtr iIFN = instrListGetInstruction(instructionList, indexIF);
					iIFN->dst = (void *) indexFalse;

					break;
				}

				//stmt_2 -> while ( EXPR ) stmt_2
				case KTT_while: {
					getNewToken(token, errCode);

					//get condition
					errCode = precedenceParsing(NULL);
					printf("PrecedenceParsing returned: %d\n", errCode);
					CHECK_ERRCODE();
					//{?
					getNewToken(token, errCode);

					if(!result) {
						EXIT(ERR_SYNTAX, "Expression expected.");
						CHECK_ERRCODE();
					}

					if(result->Type != eBOOL){
						EXIT(ERR_SEM_TYPE, "Expression cannot be converted to boolean.");
						CHECK_ERRCODE();
					}

					//on false condition jump over while body - to be refined later
					uint32_t indexIF;
					AIwO(iIFNGOTO, NULL, result, NULL, indexIF);

					//start while body
					errCode = stmt_2();
					CHECK_ERRCODE();

					//add jump back to the condition
					uint32_t whileEndBodyIndex;
					AIwO(iGOTO, (void*)indexIF, NULL, NULL, whileEndBodyIndex);

					////Refine where is it that we should jump on false condition - after while body
					tInstructionPtr condition = instrListGetInstruction(instructionList, indexIF);
					condition->dst = (void*)(whileEndBodyIndex + 1);

					break;
				}

				default:
					return ERR_SYNTAX;
			}
			break;

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
			helperToken = newToken();
			helperToken->type = token->type;
			helperToken->str = strNew();

			if (strCopyStr(symbolName, token->str) != ERR_OK) {
				freeToken(&helperToken);
				return ERR_INTERN;
			}

			getNewToken(token, errCode);

			if (token->type == TT_assignment) {
				errCode = precedenceParsing(NULL);
        printf("PrecedenceParsing returned: %d\n", errCode);
				CHECK_ERRCODE();
				checkTypesAndGenerateiMOV(errCode);
				CHECK_ERRCODE();
			} else if (token->type == TT_leftRoundBracket || token->type == TT_increment || token->type == TT_decrement) {
				//not LL case, must create helperToken for precedenceParsing
				helperToken = newToken();
				helperToken->type = token->type;
				helperToken->str = strNew();

				errCode = precedenceParsing(helperToken);
        printf("PrecedenceParsing returned: %d\n", errCode);
				if (errCode != ERR_OK) {
					freeToken(&helperToken);
					return errCode;
				}
			}

			freeToken(&helperToken);

			getNewToken(token, errCode);
			break;
		}

		default:
			return ERR_SYNTAX;

	}

	if (token->type == TT_rightCurlyBracket)
		return ERR_OK;

	if (token->type == TT_keyword) {
		if (token->keywordType == KTT_boolean
		 || token->keywordType == KTT_double
		 || token->keywordType == KTT_int
		 || token->keywordType == KTT_String
		 || token->keywordType == KTT_else) {
		 	return ERR_OK;
		}
	}
	errCode = stmt_2();
	CHECK_ERRCODE();
	return ERR_OK;

}

eError var_2() {

	eError errCode = ERR_OK;
  getNewToken(token, errCode);
  strCopyStr(symbolName, token->str);
	getNewToken(token, errCode);

	if (token->type == TT_assignment) {
		errCode = precedenceParsing(NULL);
    CHECK_ERRCODE();
    checkTypesAndGenerateiMOV(errCode);
	CHECK_ERRCODE();
	}
	if (token->type != TT_semicolon)
    return ERR_SYNTAX;

  strClear(symbolName);
	return ERR_OK;
}
