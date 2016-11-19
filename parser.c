//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//


#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "interpret.h"

/*
 * Checks if given adept for identifier collides with any builtin
 */
#define isBuiltin(string)\
do{                                                                                     \
  if (   (strCmpCStr(string, "substr") == 0) || (strCmpCStr(string, "readDouble") == 0) \
      || (strCmpCStr(string, "readInt") == 0) || (strCmpCStr(string, "readString") == 0) \
      || (strCmpCStr(string, "print") == 0) || (strCmpCStr(string, "length") == 0)      \
      || (strCmpCStr(string, "compare") == 0) || (strCmpCStr(string, "find") == 0)      \
      || (strCmpCStr(string, "sort") == 0) )                                            \
      strFree(string);          \
      string = NULL;      \
}while(0)\
/*
 *  Converts between token type and symbol type
 */
#define TTtoeSymbolType(ktt, eSymbol)\
do{                                  \
  switch (ktt){                      \
    case KTT_int:                    \
      eSymbol = eINT;                \
    case KTT_double:                 \
      eSymbol = eDOUBLE;             \
    case KTT_boolean:                \
      eSymbol = eBOOL;               \
    case KTT_String:                \
      eSymbol = eSTRING;              \
    default:                          \
      eSymbol = eNULL;                \
  }                                    \
}while(0)

#define printSymbol(what_symbol, saved_into, symbol)do{\
  printf("%s symbol living at: %p\nSymbol->Const: %d\nSymbol->Defined: %d\nSymbol->Name: %s\nGST: %p \nCST: %p\n Current symbol: %p\nSaved Into table: %p\n\n", what_symbol, symbol, symbol->Const, symbol->Defined, symbol->Name->str, globalScopeTable, currentScopeTable, currentSymbol, saved_into);\
}while (0);

// Borrowed from interpret.c
#define EXIT(err, ...) do{errCode = err; printError(err, __VA_ARGS__);}while(0)
/*
 * Creates a new class symbol and stores it in globalScopeTable
 * Changes currentScopeTable
 */
#define createClass(name) \
do {																								\
	tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE); \
	if (newTable == NULL) {errCode = ERR_INTERN; break;}	\
	tSymbolPtr classSymbol = symbolNew();											\
	if (classSymbol == NULL) {errCode = ERR_INTERN; htabFree(newTable); break;}\
  classSymbol->Type = eCLASS;																\
  classSymbol->Const = true;																\
  classSymbol->Defined = true;															\
  classSymbol->Name = strNewFromStr(name);						\
  classSymbol->Data.ClassData.LocalSymbolTable = newTable;\
	newTable->Parent = classSymbol;						\
	if (htabAddSymbol(globalScopeTable, classSymbol, true) == NULL) \
		{errCode = ERR_INTERN; htabFree(newTable); symbolFree(classSymbol);} \
  printSymbol("Class", globalScopeTable, classSymbol);     \
	currentScopeTable = classSymbol->Data.ClassData.LocalSymbolTable;\
	currentSymbol = classSymbol;\
} while (0)

/*
 * Creates a new function symbol and stores it in currentScopeTable
 * Changes currentScopeTable
 */
#define createFunction(type, defined, name)		\
do{																										\
	tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE);\
	if (newTable == NULL) {errCode = ERR_INTERN; break;}	\
	tSymbolPtr functionSymbol = symbolNew();											\
	if (functionSymbol == NULL) {errCode = ERR_INTERN; htabFree(newTable); break;}\
	functionSymbol->Type = eFUNCTION;																\
	functionSymbol->Name = name;						\
  functionSymbol->Defined = defined;\
	functionSymbol->Data.FunctionData.ArgumentList = NULL;\
	functionSymbol->Data.FunctionData.NumberOfArguments = 0;\
	functionSymbol->Data.FunctionData.ReturnType = type;\
	functionSymbol->Data.FunctionData.InstructionIndex = 0;\
	functionSymbol->Data.FunctionData.LocalSymbolTable = newTable;\
	newTable->Parent = functionSymbol;\
	if (htabAddSymbol(currentScopeTable, functionSymbol, true) == NULL) \
		{errCode = ERR_INTERN; htabFree(newTable); symbolFree(functionSymbol);} \
  printSymbol("Function", currentScopeTable, functionSymbol);\
	currentScopeTable = functionSymbol->Data.FunctionData.LocalSymbolTable;\
	currentSymbol = functionSymbol;\
} while (0)

/*
 * Creates a new variable symbol and stores it in currentScopeTable
 */
#define createVariable(type, defined, name, data)		\
do{																										\
	tSymbolPtr currentVariable = symbolNew();						\
	if (currentVariable == NULL) {errCode = ERR_INTERN; break;}	\
	currentVariable->Type = type;																\
	currentVariable->Const = true;																\
	currentVariable->Defined = defined;															\
	currentVariable->Name = name;\
	if (htabAddSymbol(currentScopeTable, currentVariable, true) == NULL){errCode = ERR_INTERN; symbolFree(currentVariable);}\
		errCode = ERR_INTERN;														\
  printSymbol("Variable", currentScopeTable, currentVariable);						\
} while (0)

#define getNewToken(token, errCode)\
do{\
	cleanToken(&token);							\
	errCode = getToken(token);					\
	if (errCode != ERR_OK)					\
		return errCode;							\
} while (0)

/*
 */
eError parse();
eError classList();
eError classBody();
eError paramList();
eError funcBody();
eError stmt();
eError var();

tHashTablePtr currentScopeTable;
// global variables - used in multiple functions in parser.c and expr.c
Token *token;
tSymbolPtr currentSymbol;
tSymbolPtr testSymbol;
tHashTablePtr testHash;
bool firstRun = true;
tSymbolPtr result;
dtStrPtr symbolName;
tInstruction instr;

tHashTablePtr globalScopeTable;
tInstructionListPtr instructionList;
tConstContainerPtr constTable;

eError parse(){

	eError errCode;
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

	if (firstRun){
		if((errCode = prog()) != ERR_OK){
      constFree(constTable);
      htabFree(globalScopeTable);
      instrListFree(instructionList);
		}
    firstRun = false;
    //currentSymbol = NULL;
    //currentScopeTable = globalScopeTable;
	}
	if (! firstRun){
    rewind(fSourceFile);
    //tSymbolPtr testSymbol;
    //tHashTablePtr testHash;
		errCode = prog();
	}
  /*
  dtStrPtr   string = strNewFromCStr("Main");
  tSymbolPtr mainSymbol;
  mainSymbol = htabGetSymbol(globalScopeTable, string);
  //mainSymbol++;*/
  Interpret(globalScopeTable, instructionList);
  return errCode;
}

eError prog() {

	eError errCode;
	if ((token = newToken()) == NULL)
		return ERR_INTERN;
	if ((currentSymbol = symbolNew()) == NULL){
		freeToken(&token);
		return ERR_INTERN;
	}
	if (firstRun){
		if ((symbolName = strNew()) == NULL){
			freeToken(&token);
			symbolFree(currentSymbol);
			return ERR_INTERN;
		}
	}
	//1. Token -> [<KTT_CLASS>]
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_class) {
		errCode = classList();
	}
	//if 1. Token != [<KTT_CLASS>] I'm expecting EOF, otherwise syntactic error
	if (token->type == TT_EOF){
    freeToken(&token);
    symbolFree(currentSymbol);
    if (firstRun)
  		strFree(symbolName);
    return errCode;
  }
  else{
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {PROG -> CLASS_LIST eof}\n",  __FILE__, __LINE__);
    errCode = ERR_SYNTAX;
  }
  freeToken(&token);
  symbolFree(currentSymbol);
	return errCode;
}

eError classList() {

	eError errCode = ERR_OK;
	//[<KTT_CLASS>] 2. Token -> [<TT_identifier>]
	//RULE: CLASS_LIST -> class simple_id { CLASS_BODY } CLASS_LIST
	getNewToken(token, errCode);
	if (token->type != TT_identifier){
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {CLASS_LIST -> class simple_id { CLASS_BODY } CLASS_LIST}\n",  __FILE__, __LINE__);
    return ERR_SYNTAX;
  }
	if (firstRun){
		// backup of what i want to call the symbol later
		if (strCopyStr(symbolName, token->str) != STR_SUCCESS)
      return ERR_INTERN;
  }
  // updating the currentScope variable in second run
  if (!firstRun){
    //htabGetSymbol(globalScopeTable, token->str) should NOT return nothing
    testSymbol = htabGetSymbol(globalScopeTable, token->str);
    testHash = testSymbol->Data.ClassData.LocalSymbolTable;
    //printSymbol("In 2. run class update", NULL, currentSymbol);
    //printf("a");
  }
	//[<KTT_CLASS>][<TT_identifier>]3. Token -> [{]
	getNewToken(token, errCode);
	if (token->type == TT_leftCurlyBracket){
		if (firstRun){
      // Creating our first class symbol! This one will go to globalScopeTable and currentScopeTable will be changed
			createClass(symbolName);

      // TODO: If i encounter error after this, can I rely on htabFree(globalScopeTable) to free the symbol?
		}
	}
	else{
		return ERR_SYNTAX;
	}
	//[<KTT_CLASS>][<TT_identifier>][{]4. Token -> [<KTT_STATIC>]
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody();
		if (errCode != ERR_OK)
			return errCode;
	}
	//[<KTT_CLASS>][<TT_identifier>][{]4. Token -> [}]
	if (token->type == TT_rightCurlyBracket){
		//[<KTT_CLASS>][<TT_identifier>][{][}]5. Token -> [fist token of CLASS_LIST]
		getNewToken(token, errCode);
    if (!firstRun){
      instr.type = iRET;
      instr.dst  = NULL;
      instr.arg1 = NULL; // [TODO] map to a class ?
      instr.arg2 = NULL;
      instrListInsertInstruction(instructionList, instr);
    }
		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			// I'm over with first class, let's go to another one !
			// Recursive call of classList()
			errCode = classList();
			if (errCode != ERR_OK)
				return errCode;
				// TODO: Aren't i missing some kind of free?
		}
		//[<KTT_CLASS>][<TT_identifier>][{][}]5. Token -> [not fist token of CLASS_LIST], returning to prog()
		// Current token be other than EOF, the check is in prog()
		return errCode;
	}
	// not [static], nor [}]
	else{
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {CLASS_BODY -> static type simple_id VAR_OR_FUNC CLASS_BODY}\n",  __FILE__, __LINE__);
    return ERR_SYNTAX;
  }
}

eError classBody() {
	dtStrPtr helperString;
	eError errCode;
	eSymbolType helperTokenType;
	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>]5.Token -> [<KTT_*>]
	//RULE: CLASS_BODY -> static type simple_id VAR_OR_FUNC CLASS_BODY
	getNewToken(token, errCode);
	if (token->type != TT_keyword){
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {CLASS_BODY -> static type simple_id VAR_OR_FUNC CLASS_BODY}\n",  __FILE__, __LINE__);
  return ERR_SYNTAX;
  }
	if (token->keywordType != KTT_boolean
	 && token->keywordType != KTT_String
	 && token->keywordType != KTT_double
	 && token->keywordType != KTT_int
	 && token->keywordType != KTT_void) {
     EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {CLASS_BODY -> static type simple_id VAR_OR_FUNC CLASS_BODY}\n",  __FILE__, __LINE__);
		return ERR_SYNTAX;
	}

	if (firstRun)
		TTtoeSymbolType(token->keywordType, helperTokenType);

	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>]6.->[<TT_identifier>]
	getNewToken(token, errCode);
  if (token->type != TT_identifier)
		return ERR_SYNTAX;
  if (!firstRun){
    //htabGetSymbol(globalScopeTable, token->str) should NOT return nothing
    testSymbol = htabGetSymbol(testHash, token->str);
    testHash = testSymbol->Data.ClassData.LocalSymbolTable;
    //printSymbol("In 2. run function update", NULL, currentSymbol);
  }
	if (firstRun){
		if ((helperString = strNewFromStr(token->str)) == NULL){
      return ERR_INTERN;
      // Checking if token->str doesn't collide with builtins
      isBuiltin(helperString);
      if (helperString == NULL){
        EXIT(ERR_SYNTAX, "Identifier collides with a builtin in %s at %d \n",  __FILE__, __LINE__);
        return ERR_SEM;
      }
    }
	}
	//read next token - now we will find out, if it is function or identifier
	//if next token will be semicolon or assigment - it is variable
	//VAR_OR_FUNC -> INITIALIZE
	//	INITIALIZE -> ;
	//	INITIALIZE -> = EXPR ;
	//if next token will be left round bracket - it is function
	//VAR_OR_FUNC -> ( PARAM ) { FUNC_BODY }
	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// Variable without initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][;]

			if (firstRun){
        // if variable and i didn't find helperTokenType in TTtoeSymbolType() -> uknown type -> error
        if (helperTokenType == eNULL){
          EXIT(ERR_SYNTAX, "Unexpected token type in %s at %d \n",  __FILE__, __LINE__);
          strFree(helperString);
          return ERR_SYNTAX;
        }
				createVariable(helperTokenType, true, helperString, NULL);
			}

			getNewToken(token, errCode);
			break;

		case TT_assignment:
			//we have variable with initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][=]
			//after assignment have an expression - we call expressions parsing
      // if variable and i didn't find helperTokenType in TTtoeSymbolType() -> uknown type -> error
      if (firstRun){
        if (helperTokenType == eNULL){
          EXIT(ERR_SYNTAX, "Unexpected token type in %s at %d \n",  __FILE__, __LINE__);
          strFree(helperString);
          return ERR_SYNTAX;
        }
      }
			errCode = precedenceParsing(NULL); // currently broken for some reason
      printf("PrecedenceParsing returned: %d\n", errCode);
			if (errCode != ERR_OK) {
        if (firstRun) {strFree(helperString);}
				return errCode;
			}
			// check result here
			//we should have result of axpressions parsing in variable result
			//we can assign it to variable
			if (firstRun){
				createVariable(helperTokenType, true, helperString, result);
			}
			//expressions parsing read one token outside of expression - has to be semicolon
			if (token->type != TT_semicolon){
        EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {INITIALIZE -> = EXPR ;}\n",  __FILE__, __LINE__);
        return ERR_SYNTAX;
      }

			getNewToken(token, errCode);

			break;

		case TT_leftRoundBracket:
			//we have function
			if (firstRun){
				createFunction(eFUNCTION, true, helperString);
        /*
        dtStrPtr   string = strNewFromCStr("Main");
        tSymbolPtr mainSymbol;
        mainSymbol = htabGetSymbol(globalScopeTable, string);
        strClear(string);
        strAddCStr(string, "run");
        mainSymbol = htabGetSymbol(mainSymbol->Data.ClassData.LocalSymbolTable, string);
        mainSymbol++;*/
        if (errCode != ERR_OK){
          strFree(helperString);
          return errCode;
        }
			}
      if (!firstRun){
        instr.type = iFRAME;
        instr.dst  = NULL;
        instr.arg1 = currentSymbol->Name;
        instr.arg2 = NULL;
        instrListInsertInstruction(instructionList, instr);
      }
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
				// adds argument as a local variable to function scope
				helperTokenType = token->type;
				getNewToken(token, errCode);

				if (token->type != TT_identifier)
					return ERR_SYNTAX;

				//we have id
				if (firstRun){
					if ((helperString = strNewFromStr(token->str)) == NULL) // + freeShit
            strFree(helperString);
						return ERR_INTERN;
				}
				//read next token
				getNewToken(token, errCode);
				//if it is comma - we read another token and next cycle will check if it is token we want (type)
				if (token->type == TT_comma) {
					getNewToken(token, errCode);
					//but we can't get bracket right after comma
					if (token->type == TT_rightRoundBracket)
						return ERR_SYNTAX;
				}
				if(firstRun){
					createVariable(helperTokenType, true, helperString, data);
					symbolFuncAddArgument(currentSymbol, htabGetSymbol(currentScopeTable, helperString));
				}
			}
      // Completing the fuction by adding iCALL
      if (!firstRun){
        instr.type = iCALL;
        instr.dst  = NULL;
        instr.arg1 = NULL;
        instr.arg2 = NULL;
        instrListInsertInstruction(instructionList, instr);
        //currentSymbol->Data.FunctionData.InstructionIndex = instructionList->usedSize - 1;
      }
			//read next token - it should be left curly bracket
			//RULE: VAR_OR_FUNC -> ( PARAM ) { FUNC_BODY }
 			getNewToken(token, errCode);
			if (token->type != TT_leftCurlyBracket) {
				return ERR_SYNTAX;
			}

			getNewToken(token, errCode);
			if (token->type != TT_rightCurlyBracket) {
				errCode = funcBody();
				if (errCode != ERR_OK)
					return errCode;

				//checks if there really is right curly bracket after funcBody
				if (token->type != TT_rightCurlyBracket) {
					return ERR_SYNTAX;
				}
        else{
          if (!firstRun){
            instr.type = iRET;
            instr.dst  = NULL;
            instr.arg1 = NULL;
            instr.arg2 = NULL;
            instrListInsertInstruction(instructionList, instr);
          }
        }
			}

			getNewToken(token, errCode);
			break;

		default:
			return ERR_SYNTAX;
	}

	//token to determine what to do next
	//if it is right curly bracket - this is end of classBody
	if (token->type == TT_rightCurlyBracket) {
		return ERR_OK;
	}
	//if it is keyword static - it is first token of next classBody
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		errCode = classBody();
		if(errCode != ERR_OK) {
			return errCode;
		}
	}

	return ERR_OK;

}

eError funcBody() {

	eError errCode;

	//elements in function can be STMT or VAR
	//VAR has to start with keyword type
	//RULE: VAR -> type ID INITIALIZE
	if (token->type == TT_keyword) {
			//current token is keyword type - we call var
		 	errCode = var();
		 	if (errCode != ERR_OK) {
		 		return errCode;
		 	}

		 	getNewToken(token, errCode);

		 	//current token is keyword, but it isn't type - this could be STMT - we call stmt()
		 	errCode = stmt();
			if (errCode != ERR_OK) {
				return errCode;
			}

	} else {
		//current token is anything but keyword - this could be STMT - we call stmt()
		errCode = stmt();
		if (errCode != ERR_OK) {
			return errCode;
		}

	}

	//after funcBody parsing we got one more token - to determine what to do next
	if (token->type != TT_rightCurlyBracket) {

		errCode = funcBody();
		if (errCode != ERR_OK) {
			return errCode;
		}
	}

	return ERR_OK;

}

eError stmt() {

	eError errCode;

  //RULE: STMT -> { STMT_LIST }
	switch(token->type) {

		case TT_leftCurlyBracket:

			getNewToken(token, errCode);
			//If there is something between CurlyBrackets, we need to process it recursively
			if (token->type != TT_rightCurlyBracket) {
				errCode = stmt();
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

				//RULE: STMT -> return RETURN_VAL ;
				case KTT_return:

					//we call expressions parsing to parse RETURN_VAL
					//if there wasn't RETURN_VAR, result from expressions parsing will be NULL
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

				//STMT -> break ;
				case KTT_break:

					getNewToken(token, errCode);
					if (token->type != TT_semicolon) {
						return errCode;
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//STMT -> continue ;
				case KTT_continue:

					getNewToken(token, errCode);
					if (token->type != TT_semicolon) {
						return errCode;
					}

					//read next token - at the end of the function we will determine what to do next
					getNewToken(token, errCode);
					break;

				//STMT -> if ( EXPR ) STMT ELSE
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

					//read next token and call STMT processing
					getNewToken(token, errCode);
					errCode = stmt();
					if (errCode != ERR_OK) {
						return errCode;
					}

					//after SMTM - there can be else
					if (token->type == TT_keyword && token->keywordType == KTT_else) {
						getNewToken(token, errCode);
						errCode = stmt();
						if (errCode != ERR_OK) {
							return errCode;
						}

					}

					break;

				//STMT -> while ( EXPR ) STMT
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

					errCode = stmt();
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
					errCode = var();			//vo var spracujem uz aj semicolon, ale ostane ako posledny token
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

					errCode = stmt();
					if (errCode != ERR_OK) {
						return errCode;
					}

					break;

				case KTT_do:
					getNewToken(token, errCode);

					errCode = stmt();
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

	//bude tam var
	if (token->type == TT_keyword) {
		if (token->keywordType == KTT_boolean
		 || token->keywordType == KTT_double
		 || token->keywordType == KTT_int
		 || token->keywordType == KTT_String
		 || token->keywordType == KTT_else) {
		 	return ERR_OK;
		}
	}
	errCode = stmt();
	if (errCode != ERR_OK) {
		return errCode;
	}
	return ERR_OK;

}

eError var() {

	eError errCode;

  //RULE: sVAR -> type ID INITIALIZE
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
  if (firstRun){
    if (strCopyStr(symbolName, token->str) != STR_SUCCESS){
      return ERR_INTERN;
    }
  }

	// INITIALIZE ->     = EXPR ;
	getNewToken(token, errCode);
	if (token->type == TT_assignment) {
		//call expressions parsing to parse intialize value
		errCode = precedenceParsing(NULL);
    printf("PrecedenceParsing returned: %d\n", errCode);
		if (errCode != ERR_OK) {
      if (firstRun){strFree(symbolName);}
			return errCode;
		}
    //createVariable(symbolName, ...);
	}

	//expressions parsing could stop on semicolon or right round bracket before semicolon
	//otherwise, there is syntax error
	if (token->type != TT_semicolon) {
		getNewToken(token, errCode);
		if (token->type != TT_semicolon) {
			return ERR_SYNTAX;
		}
    //createVariable(symbolName, ...);
	}

	return ERR_OK;

}
