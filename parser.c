//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser.h"
#include "expr.h"
#include "scanner.h"
#include "ial.h"

/*
*  Converts between token type and symbol type
*/
#define TTtoeSymbolType(ktt, eSymbol)\
do{                                  \
  switch (ktt){                      \
    case KTT_int:                    \
    eSymbol = eINT; break;         \
    case KTT_double:                 \
    eSymbol = eDOUBLE; break;      \
    case KTT_boolean:                \
    eSymbol = eBOOL; break;        \
    case KTT_String:                 \
    eSymbol = eSTRING; break;      \
    default:                         \
    eSymbol = eNULL;               \
  }                                  \
}while(0)

#define printSymbol(what_symbol, symbol)do{\
  printf("%s symbol living at: %p\nSymbol->Const: %d\nSymbol->Defined: %d\nSymbol->Name: %s\nGST: %p \nCurrentClass: %p\nCurrent function: %p\n\n", what_symbol, symbol, symbol->Const, symbol->Defined, symbol->Name->str, globalScopeTable, currentClass, currentFunction);\
}while (0);

// Borrowed from interpret.c
#define EXIT(err, ...) do{errCode = err; printError(err, __VA_ARGS__);}while(0)
/*
 * Creates a new class symbol and stores it in globalScopeTable
 * Changes currentScopeTable
 */
 #define createClass(name)                                                    \
 do {		                                                                     \
  if (strCmpCStr(name, "ifj16") == 0 ) {                                     \
    errCode = ERR_SEM;                                                        \
    break;                                                                    \
  } 																						                             \
 	tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE);                      \
  tSymbolPtr addedClass;\
 	if (newTable == NULL) {errCode = ERR_INTERN; break;}	                     \
 	tSymbolPtr classSymbol = symbolNew();											                 \
 	if (classSymbol == NULL) {errCode = ERR_INTERN; htabFree(newTable); break;}\
   classSymbol->Type = eCLASS;																                 \
   classSymbol->Const = true;																                 \
   classSymbol->Defined = true;															                 \
   classSymbol->Name = strNewFromStr(name);						                       \
   classSymbol->Next = NULL;						                                       \
   classSymbol->Data.ClassData.LocalSymbolTable = newTable;                   \
 	newTable->Parent = classSymbol;						                                 \
 	if ((addedClass = htabAddSymbol(globalScopeTable, classSymbol, false)) == NULL)            \
 		{errCode = ERR_SEM; htabFree(newTable); symbolFree(classSymbol);}  else {   \
 	currentClass = addedClass;                                                \
   printSymbol("Class", currentClass); }                                       \
 } while (0)
 // TODO: ifj16 trieda nemoze byt

 /*
  * Creates a new function symbol and stores it in currentScopeTable
  * Changes currentScopeTable
  */


 #define createFunction(type, defined, name)		                           \
 do{																										\
  tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE);\
 	if (newTable == NULL) {errCode = ERR_INTERN;}	\
 	tSymbolPtr functionSymbol = symbolNew();											\
  tSymbolPtr addedFunc;\
  if (functionSymbol == NULL) {errCode = ERR_INTERN; htabFree(newTable); }\
 	functionSymbol->Type = eFUNCTION;																\
 	functionSymbol->Name = strNewFromStr(name);						\
   functionSymbol->Defined = defined;\
 	functionSymbol->Data.FunctionData.ArgumentList = NULL;\
 	functionSymbol->Data.FunctionData.NumberOfArguments = 0;\
 	functionSymbol->Data.FunctionData.ReturnType = type;\
 	functionSymbol->Data.FunctionData.InstructionIndex = 0;\
 	functionSymbol->Data.FunctionData.LocalSymbolTable = newTable;\
 	newTable->Parent = functionSymbol;\
  if ((addedFunc =htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, functionSymbol, false)) == NULL) \
    {errCode = ERR_SEM; htabFree(newTable); symbolFree(functionSymbol);} \
  if (!htabForEach(currentClass->Data.ClassData.LocalSymbolTable, checkForIdConflictWithinClass, name)) \
     {errCode = ERR_SEM; if (addedFunc != NULL){htabFree(newTable); symbolFree(functionSymbol);}} else{\
  currentFunction = addedFunc;\
  printSymbol("Function", currentFunction);}\
 } while (0)

 /*
  * Creates a new static variable symbol and stores it table of static variables and functions
  */
 #define createStaticVariable(type, defined, name)		\
 do{																										\
 	tSymbolPtr currentVariable = symbolNew();						\
 	if (currentVariable == NULL) {errCode = ERR_INTERN;}	\
 	currentVariable->Type = type;																\
 	currentVariable->Const = true;																\
 	currentVariable->Defined = defined;															\
 	currentVariable->Name = name;\
 	if (htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, currentVariable, false) == NULL)\
     {errCode = ERR_SEM; /*symbolFree(currentVariable)*/;} else {\
   printSymbol("Static variable", currentVariable);}\
 } while (0)


 /*
  * Creates a new local variable or parameter symbol and stores it table of function variables
  */

 #define createFunctionVariable(type, defined, name, isArgument)                                        \
 do{                                                                                                     \
 	tSymbolPtr currentVariable = symbolNew();                                                             \
  tSymbolPtr addedVar;\
  tSymbolPtr helperVar;\
  if (currentVariable == NULL) {errCode = ERR_INTERN;}                                                  \
 	currentVariable->Type = type;																                                          \
 	currentVariable->Const = false;																                                        \
 	currentVariable->Defined = defined;															                                      \
 	currentVariable->Name = name;                                                                         \
 	if ((addedVar = htabAddSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, currentVariable, false)) == NULL) {\
    errCode = ERR_SEM; symbolFree(currentVariable);\
  } else {    \
    helperVar = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, currentVariable->Name);\
    if (helperVar != NULL && helperVar->Type == eFUNCTION) {\
      errCode = ERR_SEM; symbolFree(currentVariable);\
    } else {\
      printSymbol("Function variable", currentVariable);                                                    \
      if (isArgument)                                                                                      \
        symbolFuncAddArgument(currentFunction, addedVar);\
      errCode = ERR_OK;\
    }     \
  }                                       \
 } while (0)

#define getNewToken(token, errCode)   \
do{                                   \
	cleanToken(&token);							    \
	errCode = getToken(token);					\
	if (errCode != ERR_OK)					    \
		return errCode;                   \
} while (0)

/*
 */
eError skipPrecedenceParsing(eError errCode);
eError skipFunctionCall(eError errCode);
eError classList();
eError classBody();
eError funcBody();
eError stmt();
eError var();

// global variables - used in multiple functions in parser.c and expr.c
Token *token;
// TODO: I cannot free these 2 until after Interpret is called since it results in SIGSEGV while accessing table -> Figure out what to do with it
tSymbolPtr currentFunction;
tSymbolPtr currentClass;
tSymbolPtr result;
dtStrPtr symbolName;
eSymbolType symbolTokenType;

extern tHashTablePtr globalScopeTable;
extern tConstContainerPtr constTable;

// urobim main, run do GST, nastrkam tam staticke inicializovane, nastrkam instrukcie do inej instr. pasky
/*
 * A mock of expression evaluation, which I don't really need in 1. run,
 * but need to check for correct syntax and skip tokens.
 */
eError skipPrecedenceParsing(eError errCode){
  while (token->type != TT_semicolon) {
    getNewToken(token, errCode);
    if (errCode != ERR_OK)
      return errCode;
    if (token->type == TT_EOF)
      return ERR_SYNTAX;
  }
  return ERR_OK;
}

/*
 * A mock of function call, which I don't generate in 1. run,
 * but need to check for correct syntax and skip tokens.
 */
eError skipFunctionCall(eError errCode){
  while (token->type != TT_rightRoundBracket) {
    getNewToken(token, errCode);
    if (token->type == TT_EOF)
      return ERR_SYNTAX;
  }
  return ERR_OK;
}

tSymbolPtr checkForIdConflictWithinClass(tSymbolPtr symbol, void* name){
  if (symbol->Type == eFUNCTION){
    if (htabGetSymbol(symbol->Data.FunctionData.LocalSymbolTable, name) == NULL) {
      return symbol;
    } else {
      return NULL;
    }
  }
  else {
    if ((strCmpStr(symbol->Name, (dtStrPtr)name)) == 0)
      return NULL;
  return symbol;
  }
}

eError initializeHelperVariables(){
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
  if ((symbolName = strNew()) == NULL){
    symbolFree(currentFunction);
    symbolFree(currentClass);
    freeToken(&token);
    return ERR_INTERN;
  }
  return ERR_OK;
}

void freeHelperVariables(){
  freeToken(&token);
  strFree(symbolName);
}

eError fillSymbolTable() {

	eError errCode = ERR_OK;
  if((errCode = initializeHelperVariables()) != ERR_OK)
    return errCode;
	//1. Token -> [<KTT_CLASS>]
  errCode = getToken(token);
  if (errCode != ERR_OK)
    goto freeResourcesAndFinish;
  // looking for EOF before going into class
  if (token->type == TT_EOF){
    EXIT(ERR_SEM, "Input file is empty\n");
    errCode = ERR_SEM;
    goto freeResourcesAndFinish;
  }

	if (token->type == TT_keyword && token->keywordType == KTT_class) {
		if ((errCode = classList()) != ERR_OK)
      goto freeResourcesAndFinish;
	}
	//if 1. Token != [<KTT_CLASS>] I'm expecting EOF, otherwise syntactic error
	if (token->type == TT_EOF)
    goto freeResourcesAndFinish;

  else{
    EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {PROG -> CLASS_LIST eof}\n",  __FILE__, __LINE__);
    errCode = ERR_SYNTAX;
  }

  freeResourcesAndFinish:
  freeHelperVariables();
  // I won't go into interpretation, therefore currentClass and currentFunction need to be freed
  // I cannot currupt contents of globalScopeTable if there are some though
  if (errCode != ERR_OK && globalScopeTable->NumberOfItems == 0){
    symbolFree(currentClass);
    symbolFree(currentFunction);
  }
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
	// backup of what i want to call the symbol later
	if (strCopyStr(symbolName, token->str) != STR_SUCCESS)
    return ERR_INTERN;
	//[<KTT_CLASS>][<TT_identifier>]3. Token -> [{]
	getNewToken(token, errCode);
	if (token->type == TT_leftCurlyBracket){
      // Creating our first class symbol! This one will go to globalScopeTable and currentScopeTable will be changed
			createClass(symbolName);
      htabGetSymbol(globalScopeTable, symbolName);
      strClear(symbolName);
      if (errCode != ERR_OK)
        return errCode;
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
		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			// I'm over with first class, let's go to another one !
			// Recursive call of classList()
			errCode = classList();
			if (errCode != ERR_OK)
				return errCode;
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
  return errCode;
}

eError classBody() {
	eError errCode = ERR_OK;
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

	TTtoeSymbolType(token->keywordType, symbolTokenType);

	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>]6.->[<TT_identifier>]
	getNewToken(token, errCode);
  if (token->type != TT_identifier)
		return ERR_SYNTAX;
	if (strCopyStr(symbolName, token->str) != STR_SUCCESS)
    return ERR_INTERN;
	//read next token - now we will find out, if it is function or identifier
	//VAR_OR_FUNC -> ( PARAM ) { FUNC_BODY }
	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// Variable without initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][;]
      if (symbolTokenType == eNULL){
        EXIT(ERR_SYNTAX, "Unexpected token type in %s at %d \n",  __FILE__, __LINE__);
        return ERR_SYNTAX;
      }
      // treba overit co sa vyhodnocuje (iba konstanty + uz zname staticke) - asi check vo vyrazoch - over
      createStaticVariable(symbolTokenType, true, symbolName);
      if (errCode != ERR_OK) {
        return errCode;
      }
			getNewToken(token, errCode);
			break;

		case TT_assignment:
			//we have variable with initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][=]
			//after assignment have an expression - we call expressions parsing
      // if variable and i didn't find symbolTokenType in TTtoeSymbolType() -> uknown type -> error
      if (symbolTokenType == eNULL){
        EXIT(ERR_SYNTAX, "Unexpected token type in %s at %d \n",  __FILE__, __LINE__);
        return ERR_SYNTAX;
      }
			errCode = skipPrecedenceParsing(errCode);
			if (errCode != ERR_OK)
				return errCode;

			createStaticVariable(symbolTokenType, true, symbolName);
      if (errCode != ERR_OK) {
        return errCode;
      }

			//expressions parsing read one token outside of expression - has to be semicolon
			if (token->type != TT_semicolon){
        EXIT(ERR_SYNTAX, "Unexpected token in %s at %d violating {INITIALIZE -> = EXPR ;}\n",  __FILE__, __LINE__);
        return ERR_SYNTAX;
      }

			getNewToken(token, errCode);
			break;

		case TT_leftRoundBracket:
      createFunction(symbolTokenType, true, symbolName);
      if (errCode != ERR_OK){
        return errCode;
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

				//we have a type
				// adds argument as a local variable to function scope
				TTtoeSymbolType(token->type, symbolTokenType);
				getNewToken(token, errCode);

				if (token->type != TT_identifier)
					return ERR_SYNTAX;

				//we have an id
				if (strCopyStr(symbolName,token->str) != STR_SUCCESS)
					return ERR_INTERN;
				//read next token
				getNewToken(token, errCode);
				//if it is comma - we read another token and next cycle will check if it is token we want (type)
				if (token->type == TT_comma) {
					getNewToken(token, errCode);
					//but we can't get bracket right after comma
					if (token->type == TT_rightRoundBracket)
						return ERR_SYNTAX;
				}
        // last true for isArgument, updating argument list of currentFunction
				createFunctionVariable(symbolTokenType, true, symbolName, true);
        if (errCode != ERR_OK) {
          return errCode;
        }
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
          errCode = ERR_SYNTAX;
          return ERR_SYNTAX;
				}
			}
      strClear(symbolName);
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
	if (token->type == TT_keyword && token->keywordType == KTT_static)
		errCode = classBody();

  return errCode;
}

eError funcBody() {

	eError errCode = ERR_OK;
	//RULE: VAR -> type ID INITIALIZE
	if (token->type == TT_keyword) {
    if (token->keywordType == KTT_boolean
     || token->keywordType == KTT_double
     || token->keywordType == KTT_int
     || token->keywordType == KTT_String) {

		  //current token is keyword type - we call var
	 	  errCode = var();
	 	  if (errCode != ERR_OK)
	 		  return errCode;
	 	  getNewToken(token, errCode);

    } else {
      //current token is keyword, but it isn't type - this could be STMT - we call stmt()
      errCode = stmt();
      if (errCode != ERR_OK) {
        return errCode;
      }

    }

	} else {
		//current token is anything but keyword - this could be STMT - we call stmt()
		errCode = stmt();
		if (errCode != ERR_OK)
			return errCode;
	}

	//after funcBody parsing we got one more token - to determine what to do next
	if (token->type != TT_rightCurlyBracket) {
		errCode = funcBody();
		if (errCode != ERR_OK)
			return errCode;
	}

  return errCode;
}

eError stmt() {

	eError errCode = ERR_OK;

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
			if (token->type == TT_rightCurlyBracket) {
        //read next token - at the end of the function we will determine what to do next
        getNewToken(token, errCode);
        break;

      }

      return ERR_SYNTAX;

		case TT_keyword:
			switch(token->keywordType){

				//RULE: STMT -> return RETURN_VAL ;
				case KTT_return:

					//we call expressions parsing to parse RETURN_VAL
					//if there wasn't RETURN_VAR, result from expressions parsing will be NULL
					errCode = skipPrecedenceParsing(errCode);
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

				//STMT -> if ( EXPR ) STMT ELSE
				case KTT_if:

					//next token have to be left rounf bracket
					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						return ERR_SYNTAX;
					}
					//call expressions parsing - parse condition
					errCode = skipFunctionCall(errCode); //lebo je to ukoncene zatvorkou, nie bodkociarkou
					if(errCode != ERR_OK)
						return errCode;

					//there have to be right round bracket after condition
					if(token->type != TT_rightRoundBracket)
						return ERR_SYNTAX;

					//read next token and call STMT processing
					getNewToken(token, errCode);
					errCode = stmt();
					if (errCode != ERR_OK)
						return errCode;

					//after SMTM - there can be else
					if (token->type == TT_keyword && token->keywordType == KTT_else) {
						getNewToken(token, errCode);
						errCode = stmt();
						if (errCode != ERR_OK)
							return errCode;
					}
					break;

				//STMT -> while ( EXPR ) STMT
				case KTT_while:

					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket)
						return ERR_SYNTAX;

					errCode = skipFunctionCall(errCode); //lebo je to ukoncene zatvorkou, nie bodkociarkou
          if(errCode != ERR_OK)
            return errCode;

          getNewToken(token, errCode);

          errCode = stmt();
					if (errCode != ERR_OK)
						return errCode;

					break;

				default:
					return ERR_SYNTAX;
			}
			break;

		case TT_increment:
		case TT_decrement:
			errCode = skipPrecedenceParsing(errCode);
      if (errCode != ERR_OK)
        return errCode;
			if(token->type != TT_semicolon)
				return ERR_SYNTAX;

			getNewToken(token, errCode);
			break;

		case TT_identifier:
		case TT_fullIdentifier: {
			//not LL case
			//might be assignment or function call or id++/--

			getNewToken(token, errCode);

			if (token->type == TT_assignment || token->type == TT_increment || token->type == TT_decrement) {
				errCode = skipPrecedenceParsing(errCode);
				if(errCode != ERR_OK){
					return errCode;
        }
      } else if (token->type == TT_leftRoundBracket){
        errCode = skipFunctionCall(errCode);
        if(errCode != ERR_OK){
          return errCode;
        }
        getNewToken(token, errCode);

      } else {
        return ERR_SYNTAX;
      }

			if (token->type != TT_semicolon)
				return ERR_SYNTAX;

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
	errCode = stmt();
	if (errCode != ERR_OK)
		return errCode;

	return ERR_OK;

}

eError var() {

	eError errCode = ERR_OK;

  TTtoeSymbolType(token->keywordType, symbolTokenType);
  getNewToken(token, errCode);

  if (token->type != TT_identifier)
    return ERR_SYNTAX;

  if (strCopyStr(symbolName, token->str) != STR_SUCCESS)
    return ERR_INTERN;
  if (symbolName == NULL)
    return ERR_SEM;

  createFunctionVariable(symbolTokenType, true, symbolName, false);
  if (errCode != ERR_OK) {
    return errCode;
  }

  getNewToken(token, errCode);
  // INITIALIZE ->     = EXPR ;
  if (token->type == TT_assignment) {
    // [reserved_name] [=]  -> Nope
    errCode = skipPrecedenceParsing(errCode);
    if (errCode != ERR_OK)
      return errCode;
	}

  if (token->type != TT_semicolon) {
    return ERR_SYNTAX;
  }
  strClear(symbolName);
	return ERR_OK;
}
/*
  [1] htabForEach
  [2] 2. instr. paska v 2. behu
*/
