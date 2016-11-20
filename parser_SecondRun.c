//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser_SecondRun.h"
#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "interpret.h"
/*
#define printSymbol(what_symbol, symbol)do{\
  printf("%s symbol living at: %p\nSymbol->Const: %d\nSymbol->Defined: %d\nSymbol->Name: %s\nGST: %p \n Current symbol: %p\n\n", what_symbol, symbol, symbol->Const, symbol->Defined, symbol->Name->str, globalScopeTable, symbol);\
}while (0);*/

// Borrowed from interpret.c
#define EXIT(err, ...) do{errCode = err; printError(err, __VA_ARGS__);}while(0)

eError prog_2();
eError classList_2();
eError classBody_2();
eError funcBody_2();
eError stmt_2();
eError var_2();

extern tInstructionListPtr instructionList;
extern tConstContainerPtr constTable;
extern tHashTablePtr globalScopeTable;
Token *token;
extern tSymbolPtr currentFunction;
extern tSymbolPtr currentClass;
dtStrPtr className;
tSymbolPtr result;
tInstruction instr;


#define getNewToken(token, errCode)\
do{\
	cleanToken(&token);							\
	errCode = getToken(token);					\
	if (errCode != ERR_OK)					\
		return errCode;							\
} while (0)

eError initializeHelperVariables_2(){
  if ((currentFunction = symbolNew()) == NULL)
    return ERR_INTERN;
  if((currentClass = symbolNew()) == NULL){
    symbolFree(currentFunction);
    return ERR_INTERN;
  }
  if ((token = newToken()) == NULL){
    symbolFree(currentFunction);
    symbolFree(currentClass);
    return ERR_INTERN;
  }
  if ((className = strNew()) == NULL){
    symbolFree(currentFunction);
    symbolFree(currentClass);
    freeToken(&token);
    return ERR_INTERN;
  }
  return ERR_OK;
}

void freeHelperVariables_2(){
  freeToken(&token);
  symbolFree(currentFunction);
  symbolFree(currentClass);
  strFree(className);
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

		/*case eFUNCTION:
			htabForEach(symbol->Data.FunctionData.LocalSymbolTable, findUndefinedSymbol, NULL);
			break;

		default:
			break;
	}
	return symbol;
}*/

eError parse_2(){
  dtStrPtr string = strNewFromCStr("Main");
  htabGetSymbol(globalScopeTable, string);
	eError errCode;
  if ((errCode = initializeHelperVariables_2()) != ERR_OK)
    return errCode;
  rewind(fSourceFile);
	errCode = prog_2();

  freeHelperVariables_2();
  return errCode;
}

eError prog_2() {

	eError errCode;
  //htabForEach(globalScopeTable, findUndefinedSymbol, NULL);
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_class) {
		errCode = classList_2();
	}
	if (token->type == TT_EOF){
    return errCode;
  }
  else{
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {prog_2 -> CLASS_LIST eof}\n",  __FILE__, __LINE__);
    errCode = ERR_SYNTAX;
  }
	return errCode;
}

eError classList_2() {

	eError errCode = ERR_OK;
	//[<KTT_CLASS>] 2. Token -> [<TT_identifier>]
	//RULE: CLASS_LIST -> class simple_id { CLASS_BODY } CLASS_LIST
	getNewToken(token, errCode);
	if (token->type != TT_identifier){
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {CLASS_LIST -> class simple_id { CLASS_BODY } CLASS_LIST}\n",  __FILE__, __LINE__);
    return ERR_SYNTAX;
  }
  // updating the currentScope var_2iable in second run
  // htabGetSymbol(globalScopeTable, token->str) should NOT return nothing
  //currentClass = htabGetSymbol(globalScopeTable, token->str);
  className = strNewFromStr(token->str);
  //printSymbol("In 2. run class update", currentClass);
	getNewToken(token, errCode);
	if (token->type == TT_leftCurlyBracket){
    ;
	}
	else{
		return ERR_SYNTAX;
	}
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		if (errCode != ERR_OK)
			return errCode;
	}
	if (token->type == TT_rightCurlyBracket){
		getNewToken(token, errCode);
      instr.type = iRET;
      instr.dst  = NULL;
      instr.arg1 = NULL; // [TODO] map to a class ?
      instr.arg2 = NULL;
      instrListInsertInstruction(instructionList, instr);
		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			errCode = classList_2();
			if (errCode != ERR_OK)
				return errCode;
		}
		return errCode;
	}
	else{
    return ERR_SYNTAX;
  }
}

eError classBody_2() {
	eError errCode;
	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>]5.Token -> [<KTT_*>]
	//RULE: CLASS_BODY -> static type simple_id var_2_OR_FUNC CLASS_BODY
	getNewToken(token, errCode);
	if (token->type != TT_keyword){
  return ERR_SYNTAX;
  }
	if (token->keywordType != KTT_boolean
	 && token->keywordType != KTT_String
	 && token->keywordType != KTT_double
	 && token->keywordType != KTT_int
	 && token->keywordType != KTT_void) {
		return ERR_SYNTAX;
	}


	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>]6.->[<TT_identifier>]
	getNewToken(token, errCode);
  if (token->type != TT_identifier)
		return ERR_SYNTAX;
  //tSymbolPtr helperSymbol = (tSymbolPtr)htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, className);
  //currentFunction = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, token->str);
	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// var_2iable without initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][;]
			getNewToken(token, errCode);
			break;

		case TT_assignment:
			errCode = precedenceParsing(NULL);
			if (errCode != ERR_OK) {
				return errCode;
			}
			if (token->type != TT_semicolon){
        EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {INITIALIZE -> = EXPR ;}\n",  __FILE__, __LINE__);
        return ERR_SYNTAX;
      }

			getNewToken(token, errCode);

			break;

		case TT_leftRoundBracket:
			//we have function
        instr.type = iFRAME;
        instr.dst  = NULL;
        instr.arg1 = currentFunction->Name;
        instr.arg2 = NULL;
        instrListInsertInstruction(instructionList, instr);
			//reading all parameters
			getNewToken(token, errCode);
			//stops, when right round bracket is read - end of parameters
			while (token->type != TT_rightRoundBracket) {

				if (token->type != TT_keyword) {
					return ERR_SYNTAX;
				}
				if (token->keywordType != KTT_boolean
				 && token->keywordType != KTT_String
				 && token->keywordType != KTT_double
				 && token->keywordType != KTT_int) {
					return ERR_SYNTAX;
				}

				//we have type
				// adds argument as a local var_2iable to function scope
				getNewToken(token, errCode);

				if (token->type != TT_identifier)
					return ERR_SYNTAX;

				//we have id
				//read next token
				getNewToken(token, errCode);
				//if it is comma - we read another token and next cycle will check if it is token we want (type)
				if (token->type == TT_comma) {
					getNewToken(token, errCode);
					//but we can't get bracket right after comma
					if (token->type == TT_rightRoundBracket)
						return ERR_SYNTAX;
				}
			}
      // Completing the fuction by adding iCALL
      instr.type = iCALL;
      instr.dst  = NULL;
      instr.arg1 = NULL;
      instr.arg2 = NULL;
      instrListInsertInstruction(instructionList, instr);
      currentFunction->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1;

 			getNewToken(token, errCode);
			if (token->type != TT_leftCurlyBracket) {
				return ERR_SYNTAX;
			}

			getNewToken(token, errCode);
			if (token->type != TT_rightCurlyBracket) {
				errCode = funcBody_2();
				if (errCode != ERR_OK)
					return errCode;

				//checks if there really is right curly bracket after funcBody_2
				if (token->type != TT_rightCurlyBracket) {
					return ERR_SYNTAX;
				}
        else{
            instr.type = iRET;
            instr.dst  = NULL;
            instr.arg1 = NULL;
            instr.arg2 = NULL;
            instrListInsertInstruction(instructionList, instr);
        }
			}

			getNewToken(token, errCode);
			break;

		default:
			return ERR_SYNTAX;
	}

	//token to determine what to do next
	//if it is right curly bracket - this is end of classBody_2
	if (token->type == TT_rightCurlyBracket) {
		return ERR_OK;
	}
	//if it is keyword static - it is first token of next classBody_2
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody_2();
		if(errCode != ERR_OK) {
			return errCode;
		}
	}

	return ERR_OK;

}

eError funcBody_2() {

	eError errCode;

	//elements in function can be stmt_2 or var_2
	//var_2 has to start with keyword type
	//RULE: var_2 -> type ID INITIALIZE
	if (token->type == TT_keyword) {
			//current token is keyword type - we call var_2
		 	errCode = var_2();
		 	if (errCode != ERR_OK) {
		 		return errCode;
		 	}

		 	getNewToken(token, errCode);

	} else {
		//current token is anything but keyword - this could be stmt_2 - we call stmt_2()
		errCode = stmt_2();
		if (errCode != ERR_OK) {
			return errCode;
		}

	}

	//after funcBody_2 parsing we got one more token - to determine what to do next
	if (token->type != TT_rightCurlyBracket) {

		errCode = funcBody_2();
		if (errCode != ERR_OK) {
			return errCode;
		}
	}

	return ERR_OK;

}

eError stmt_2() {

	eError errCode;

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

					//zavolat vyrazy
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
					errCode = var_2();			//vo var_2 spracujem uz aj semicolon, ale ostane ako posledny token
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

					//zavolat vyrazy
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
					return errCode;
				}


			} else if (token->type == TT_leftRoundBracket || token->type == TT_increment || token->type == TT_decrement) {
				errCode = precedenceParsing(helperToken);
        printf("PrecedenceParsing returned: %d\n", errCode);
				if (errCode != ERR_OK) {
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

	//bude tam var_2
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

eError var_2() {

	eError errCode;

  //RULE: svar_2 -> type ID INITIALIZE
	if (token->keywordType != KTT_boolean
	 && token->keywordType != KTT_String
	 && token->keywordType != KTT_double
	 && token->keywordType != KTT_int) {
		return ERR_SYNTAX;
	}

	getNewToken(token, errCode);
	if (token->type != TT_identifier && token->type != TT_fullIdentifier) {
		return ERR_SYNTAX;
	}

	// INITIALIZE ->     = EXPR ;
	getNewToken(token, errCode);
	if (token->type == TT_assignment) {
		//call expressions parsing to parse intialize value
		errCode = precedenceParsing(NULL);
    printf("PrecedenceParsing returned: %d\n", errCode);
		if (errCode != ERR_OK) {
			return errCode;
		}
    //createvar_2iable(symbolName, ...);
	}

	//expressions parsing could stop on semicolon or right round bracket before semicolon
	//otherwise, there is syntax error
	if (token->type != TT_semicolon) {
		getNewToken(token, errCode);
		if (token->type != TT_semicolon) {
			return ERR_SYNTAX;
		}
    //createvar_2iable(symbolName, ...);
	}

	return ERR_OK;

}
