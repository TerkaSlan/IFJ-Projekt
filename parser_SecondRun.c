//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser_SecondRun.h"
#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "interpret.h"
#include "builtin.h"
// Borrowed from interpret.c
#define EXIT(err, ...) do{errCode = err; printError(err, __VA_ARGS__);}while(0)

eError classList_2();
eError classBody_2();
eError funcBody_2();
eError stmt_2();
eError var_2(bool defined);
eInstructionType getFunctionCallForBuiltin(dtStrPtr *string);
tSymbolPtr findSymbol(dtStrPtr symbolName);


extern tInstructionListPtr instructionList;
extern tConstContainerPtr constTable;
extern tHashTablePtr globalScopeTable;
Token *token;
extern tSymbolPtr currentFunction;
extern tSymbolPtr currentClass;
tSymbolPtr result;
tInstruction instr;
dtStrPtr symbolName;

bool updateInstructionIndex = true;

/*
 * Checks if given adept for identifier collides with any builtin
 */
eInstructionType getFunctionCallForBuiltin(dtStrPtr *string){
  if (strCmpCStr(*string, "ifj16.substr") == 0)
    return iSUBSTR;
  if (strCmpCStr(*string, "ifj16.readDouble") == 0)
    return iREAD;
  if (strCmpCStr(*string, "ifj16.readInt") == 0)
    return iREAD;
  if (strCmpCStr(*string, "ifj16.readString") == 0)
    return iREAD;
  if (strCmpCStr(*string, "ifj16.print") == 0)
    return iPRINT;
  if (strCmpCStr(*string, "ifj16.length") == 0)
    return iLEN;
  if (strCmpCStr(*string, "ifj16.compare") == 0)
    return iCOMPARE;
  if (strCmpCStr(*string, "ifj16.find") == 0)
    return iFIND;
  if (strCmpCStr(*string, "ifj16.sort") == 0)
    return iSORT;
	return iSTOP;
}

#define getNewToken(token, errCode)\
do{\
	cleanToken(&token);							\
	errCode = getToken(token);					\
	if (errCode != ERR_OK)					\
		return errCode;							\
} while (0)

eError initializeHelperVariables_2(){
  if ((token = newToken()) == NULL){
    return ERR_INTERN;
  }
	if ((symbolName = strNew()) == NULL){
		freeToken(&token);
		return ERR_INTERN;
	}
  return ERR_OK;
}

void freeHelperVariables_2(){
  freeToken(&token);
  strFree(symbolName);
	//symbolFree(currentFunction);
	//symbolFree(currentClass);
}

tSymbolPtr findSymbol(dtStrPtr symbolName){
  eError errCode;
  tSymbolPtr symbolAdept;
  if (strCharPos(symbolName, '.') != (-1)){ // AKA if fullIdentifier
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
  }
  return NULL;
}

/*
tSymbolPtr findUndefinedSymbol(tSymbolPtr symbol, void *param){
	if(!symbol)
		return NULL;

	if(!symbol->Defined){
		printf("Found Undefined: %s", symbol->Name->str);
		return symbol;
	}
	switch(symbol->Type){
		case eCLASS:
			htabForEach(symbol->Data.ClassData.LocalSymbolTable, findUndefinedSymbol, NULL);
			break;
    case eFUNCTION:
      htabForEach(symbol->Data.FunctionData.LocalSymbolTable, findUndefinedSymbol, NULL);
      break;
		default:
			break;
	}
  if (symbol->Defined)
	 return NULL;
  else{
    return symbol;
  }
}*/

/*
 * Prepares source file for reading, initiates the second run of parser
 */
eError finishParsing() {

	eError errCode = ERR_OK;
  if ((errCode = initializeHelperVariables_2()) != ERR_OK)
    return errCode;
  rewind(fSourceFile);

  // Ignoring syntactic checks, already done by 1. run
	errCode = getToken(token);
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
  // got our  [<KTT_CLASS>][<TT_identifier>][{], jumping into classBody
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		if (errCode != ERR_OK)
			return errCode;
	}
	if (token->type == TT_rightCurlyBracket){
		getNewToken(token, errCode);
    /*
      instr.type = iRET;
      instr.dst  = NULL;
      instr.arg1 = NULL; // [TODO] map to a class ?
      instr.arg2 = NULL;
      instrListInsertInstruction(instructionList, instr);
      */
    //
		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			errCode = classList_2();
			if (errCode != ERR_OK)
				return errCode;
		}
		return errCode;
	}
  return errCode;
}

eError classBody_2() {
	eError errCode = ERR_OK;

	getNewToken(token, errCode);
	//[<TT_identifier>]
	getNewToken(token, errCode);
  if(strCopyStr(symbolName, token->str) == STR_ERROR)
    return ERR_INTERN;

	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// variable without initialization
			getNewToken(token, errCode);
			break;

		case TT_assignment:
      // variable with initialization, generating iMOV
			errCode = precedenceParsing(NULL);
			if (errCode != ERR_OK)
				return errCode;
			// TODO: iMOV
			getNewToken(token, errCode);
			break;

		case TT_leftRoundBracket:
			//we have function
			getNewToken(token, errCode);
			//stops, when right round bracket is read - end of parameters
      currentFunction = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, symbolName);
      strClear(symbolName);
      // telling me next time i'll be generating an intruction within this function, i have to set instruction index
      updateInstructionIndex = true;
			while (token->type != TT_rightRoundBracket) {
        //[type]
        getNewToken(token, errCode);
				//[id]
				getNewToken(token, errCode);

				if (token->type == TT_comma)
					getNewToken(token, errCode);

			}
      //[rightCurlyBracket]
 			getNewToken(token, errCode);

			getNewToken(token, errCode);
			if (token->type != TT_rightCurlyBracket){
        // not an empty function, going to funcBody
				errCode = funcBody_2();
				if (errCode != ERR_OK)
					return errCode;
        // TODO: Good here?
        instr.type = iRET;
        instr.dst  = NULL;
        instr.arg1 = NULL;
        instr.arg2 = NULL;
        instrListInsertInstruction(instructionList, instr);
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
		if(errCode != ERR_OK)
			return errCode;
	}
	return ERR_OK;
}

eError funcBody_2() {
	eError errCode = ERR_OK;

	if (token->type == TT_keyword) {
			//current token is keyword type - we call var_2
		 	errCode = var_2(true);
		 	if (errCode != ERR_OK)
		 		return errCode;
		 	getNewToken(token, errCode);
	}
  else if (token->type == TT_identifier || token->type == TT_fullIdentifier){
    // save token->str maybe?
    errCode = var_2(false);
    if (errCode != ERR_OK)
      return errCode;

    getNewToken(token, errCode);
  }

  else {
		//current token is anything but keyword
		errCode = stmt_2();
		if (errCode != ERR_OK)
			return errCode;
	}

	if (token->type != TT_rightCurlyBracket) {
		errCode = funcBody_2();
		if (errCode != ERR_OK)
			return errCode;
	}
	return ERR_OK;
}

// TODO: Did not pay attention to stmt_2 just yet
eError stmt_2() {

	eError errCode = ERR_OK;

  //RULE: stmt_2 -> { stmt_2_LIST }
	switch(token->type) {

		case TT_leftCurlyBracket:

			getNewToken(token, errCode);
			//If there is something between CurlyBrackets, we need to process it recursively
			if (token->type != TT_rightCurlyBracket) {
				errCode = stmt_2();
				if (errCode != ERR_OK) {
					return errCode;
				}
			}
			else {
				getNewToken(token, errCode);
				break;
			}

		case TT_keyword:
			switch(token->keywordType){

				//RULE: stmt_2 -> return RETURN_VAL ;
				case KTT_return:

					//we call expressions parsing to parse RETURN_VAL
					//if there wasn't RETURN_var_2, result from expressions parsing will be NULL
					errCode = precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if (errCode != ERR_OK) {
						return errCode;
					}

					//expressions parsing could stop on semicolon or right round bracket before semicolon
					//otherwise, there is syntax error
					if (token->type != TT_semicolon) {
						getNewToken(token, errCode);
						if (token->type != TT_semicolon) {
							return ERR_SYNTAX;
						}
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//stmt_2 -> break ;
				case KTT_break:

					getNewToken(token, errCode);
					if (token->type != TT_semicolon) {
						return errCode;
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//stmt_2 -> continue ;
				case KTT_continue:

					getNewToken(token, errCode);
					if (token->type != TT_semicolon) {
						return errCode;
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//stmt_2 -> if ( EXPR ) stmt_2 ELSE
				case KTT_if:

					//next token have to be left rounf bracket
					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						return ERR_SYNTAX;
					}

					//call expressions parsing - parse condition
					errCode = precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if(errCode != ERR_OK) {
						return errCode;
					}

					//there have to be right round bracket after condition
					if(token->type != TT_rightRoundBracket) {
						return ERR_SYNTAX;
					}

					//read next token and call stmt_2 processing
					getNewToken(token, errCode);
					errCode = stmt_2();
					if (errCode != ERR_OK) {
						return errCode;
					}

					//after SMTM - there can be else
					if (token->type == TT_keyword && token->keywordType == KTT_else) {
						getNewToken(token, errCode);
						errCode = stmt_2();
						if (errCode != ERR_OK) {
							return errCode;
						}

					}

					break;

				//stmt_2 -> while ( EXPR ) stmt_2
				case KTT_while:

					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						return ERR_SYNTAX;
					}

					precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if(token->type != TT_rightRoundBracket) {
						return ERR_SYNTAX;
					}
					getNewToken(token, errCode);

					errCode = stmt_2();
					if (errCode != ERR_OK) {
						return errCode;
					}

					break;

				case KTT_for:

					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						return ERR_SYNTAX;
					}

					getNewToken(token, errCode);
					errCode = var_2(true);			//vo var_2 spracujem uz aj semicolon, ale ostane ako posledny token
					if(errCode != ERR_OK) {
						return errCode;
					}

					errCode = precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if (errCode != ERR_OK) {
						return errCode;
					}

					if (token->type != TT_semicolon) {
						return ERR_SYNTAX;
					}

					getNewToken(token, errCode);
					if (token->type != TT_identifier && token->type != TT_fullIdentifier) {
						return ERR_SYNTAX;
					}
					//zapamatat si id

					getNewToken(token, errCode);
					if (token->type != TT_assignment) {
						return ERR_SYNTAX;
					}
					errCode = precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if (errCode != ERR_OK) {
						return errCode;
					}

					if (token->type != TT_rightRoundBracket) {
						return ERR_SYNTAX;
					}
					getNewToken(token, errCode);

					errCode = stmt_2();
					if (errCode != ERR_OK) {
						return errCode;
					}

					break;

				case KTT_do:
					getNewToken(token, errCode);

					errCode = stmt_2();
					if (errCode != ERR_OK) {
						return errCode;
					}

					if (token->type != TT_keyword && token->keywordType != KTT_while) {
						return ERR_SYNTAX;
					}

					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						return ERR_SYNTAX;
					}

					errCode = precedenceParsing(NULL);
          printf("PrecedenceParsing returned: %d\n", errCode);
					if (errCode != ERR_OK) {
						return errCode;
					}

					if(token->type != TT_rightRoundBracket) {
						return ERR_SYNTAX;
					}
					getNewToken(token, errCode);

					if (token->type != TT_semicolon) {
						return ERR_SYNTAX;
					}
					getNewToken(token, errCode);

					break;

				default:
					return ERR_SYNTAX;
			}
			break;

		case TT_increment:
		case TT_decrement:
			errCode = precedenceParsing(NULL);
      printf("PrecedenceParsing returned: %d\n", errCode);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if(token->type != TT_semicolon) {
				return ERR_SYNTAX;
			}
			getNewToken(token, errCode);
			break;

		case TT_identifier:
		case TT_fullIdentifier: {
			//not LL case
			//might be assignment or function call or id++/--
			Token* helperToken;
			helperToken = newToken();
			helperToken->type = token->type;
			helperToken->str = strNew();
			if (strCopyStr(helperToken->str, token->str) != ERR_OK) {
				freeToken(&helperToken);
				return ERR_INTERN;
			}

			getNewToken(token, errCode);

			if (token->type == TT_assignment) {
				errCode = precedenceParsing(NULL);
        printf("PrecedenceParsing returned: %d\n", errCode);
				if(errCode != ERR_OK) {
					freeToken(&helperToken);
					return errCode;
				}


			} else if (token->type == TT_leftRoundBracket || token->type == TT_increment || token->type == TT_decrement) {
				errCode = precedenceParsing(helperToken);
        printf("PrecedenceParsing returned: %d\n", errCode);
				if (errCode != ERR_OK) {
					freeToken(&helperToken);
					return errCode;
				}
			}

			freeToken(&helperToken);

			if (token->type != TT_semicolon) {
				return ERR_SYNTAX;
			}

			getNewToken(token, errCode);
			break;
		}

		default:
			return ERR_SYNTAX;

	}

	if (token->type == TT_rightCurlyBracket) {
		return ERR_OK;
	}

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
	if (errCode != ERR_OK) {
		return errCode;
	}
	return ERR_OK;

}

eError var_2(bool defined) {

	eError errCode = ERR_OK;
  tSymbolPtr foundSymbol;
  if (!defined)
    strCopyStr(symbolName, token->str);
  else{
    getNewToken(token, errCode);
    strCopyStr(symbolName, token->str);
  }

	getNewToken(token, errCode);
	if (token->type == TT_assignment) {
		//call expressions parsing to parse intialize value
		errCode = precedenceParsing(NULL);
		instr.type = iMOV;
    // DO I HAVE SUCH SYMBOL? Checking for both defined and !define cause you never know, but might refactor later
    if((foundSymbol = findSymbol(symbolName)) == NULL)
      return ERR_SEM;
    else{
      instr.dst = foundSymbol;
    }
		// check na kompatibilitu typov -> tento provizorny zatial cajk, uvidime dokedy
		if ((*(tSymbolPtr)instr.dst).Type != (*result).Type){
			printf("Types aren't equal\n");
		}
		instr.arg1 = result; // p *(tSymbolPtr)instr.dst
    instr.arg2 = NULL;
		instrListInsertInstruction(instructionList, instr);
    if (updateInstructionIndex){
		  currentFunction->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1;
      updateInstructionIndex = false;
    }
    printf("PrecedenceParsing returned: %d\n", errCode);
		if (errCode != ERR_OK)
			return errCode;
	}
	if (token->type == TT_leftRoundBracket){
		eInstructionType functionCall = getFunctionCallForBuiltin(&symbolName);
		if (functionCall != iSTOP){  // isBuiltin = true
			//TODO: generate functionCall according to what individual FC need
      if (functionCall == iPRINT){
        errCode = precedenceParsing(NULL);
        symbolToString(result, &(result->Data));
        result->Type = eSTRING;
        instr.type = iPRINT;
        instr.arg1 = result;
        instr.arg2 = NULL;
        instr.dst = NULL;
        instrListInsertInstruction(instructionList, instr);
        if (updateInstructionIndex){
          currentFunction->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1;
          updateInstructionIndex = false;
        }
      }
      if (functionCall == iREAD){
        // TODO
        ;
      }
      if (functionCall == iLEN){
        //ASSERT(((tSymbolPtr) i->dst)->Type == eINT);
				//ASSERT(((tSymbolPtr) i->arg1)->Type == eSTRING);
        ;
      }
    }
    else{
      if((foundSymbol = findSymbol(symbolName)) == NULL)
        return ERR_SEM;
      else{
        instr.dst = foundSymbol;
        instr.type = iCALL;
        instr.arg1 = NULL;
        instr.arg2 = NULL;
        instr.dst = NULL;
        instrListInsertInstruction(instructionList, instr);
        if (updateInstructionIndex){
          currentFunction->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1;
          updateInstructionIndex = false;
        }
      }

    }
    skipFunctionCall(errCode);
	}
	else{
		// Defined variable without initialization, nothing to do here
		while (token->type != TT_semicolon){
      getNewToken(token, errCode);
      if (token->type == TT_EOF)
        return ERR_SYNTAX;
    }
	}

  strClear(symbolName);
	return ERR_OK;
}
