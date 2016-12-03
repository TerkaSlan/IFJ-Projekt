//
// Basic structure created by Katika
// Refined into more elaborate structure by Terka
//
#include "parser.h"
#include "scanner.h"
#include "ial.h"
#include "token.h"

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


#define printSymbol(what_symbol, symbol) ; /*do{\
  printf("%s symbol living at: %p\nSymbol->Const: %d\nSymbol->Defined: %d\nSymbol->Name: %s\nGST: %p \nCurrentClass: %p\nCurrent function: %p\n\n", what_symbol, symbol, symbol->Const, symbol->Defined, symbol->Name->str, globalScopeTable, currentClass, currentFunction);\
}while (0);*/

// Borrowed from interpret.c
#define EXIT(err, ...) do{errCode = err; printError(err, "Line: %lu - %s", (unsigned long)LineCounter, __VA_ARGS__);}while(0)
#define INTERN() EXIT(ERR_INTERN, "Error allocating new space. Out of memory.\n")
#define CHECK_ERRCODE() if (errCode != ERR_OK) return errCode

/*
 * Creates a new class symbol and stores it in globalScopeTable
 * Changes currentScopeTable
 */
 #define createClass(name)                                                    \
 do {		                                                                     \
	if (strCmpCStr(name, "ifj16") == 0 ) {                                     \
		EXIT(ERR_SEM, "Class name ifj16 is reserved.\n");                                                        \
		break;                                                                    \
	} 																						                             \
 	tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE);                      \
	tSymbolPtr addedClass;\
	if (newTable == NULL) {INTERN(); break;}	                     \
																		\
	tSymbolPtr classSymbol = symbolNew();											                 \
	if (classSymbol == NULL) {INTERN(); htabFree(newTable); break;}          \
																					\
	classSymbol->Type = eCLASS;																                 \
	classSymbol->Const = true;																                 \
	classSymbol->Defined = true;															                 \
	classSymbol->Name = name;						                       \
	classSymbol->Next = NULL;						                                       \
	classSymbol->Data.ClassData.LocalSymbolTable = newTable;                   \
	if(htabGetSymbol(globalScopeTable, name))\
		{EXIT(ERR_SEM, "Redefining symbol.\n"); htabFree(newTable); symbolFree(classSymbol); name = NULL; break;  }			\
																				\
	if ((addedClass = htabAddSymbol(globalScopeTable, classSymbol, true)) == NULL)            \
	    {INTERN(); htabFree(newTable); symbolFree(classSymbol); name = NULL; break;  }\
	newTable->Parent = addedClass;						                                 \
	currentClass = addedClass;                                                \
	printSymbol("Class", currentClass);                                       \
	symbolFree(classSymbol);\
	name = NULL;\
 } while (0)


 /*
  * Creates a new function symbol and stores it in currentScopeTable
  * Changes currentScopeTable
  */
 #define createFunction(type, defined, name)		                           \
do{																										\
	tHashTablePtr newTable = htabInit(HTAB_DEFAULT_SIZE);\
	if (newTable == NULL) {INTERN();}	\
	tSymbolPtr functionSymbol = symbolNew();											\
	tSymbolPtr addedFunc;\
	if (functionSymbol == NULL) {INTERN(); htabFree(newTable); break; }\
	functionSymbol->Type = eFUNCTION;																\
	functionSymbol->Name = name;						\
	functionSymbol->Defined = defined;\
	functionSymbol->Data.FunctionData.ArgumentList = NULL;\
	functionSymbol->Data.FunctionData.NumberOfArguments = 0;\
	functionSymbol->Data.FunctionData.ReturnType = type;\
	functionSymbol->Data.FunctionData.InstructionIndex = 0;\
	functionSymbol->Data.FunctionData.LocalSymbolTable = newTable;\
  if (currentClass != NULL && !htabForEach(currentClass->Data.ClassData.LocalSymbolTable, checkForIdConflictWithinClass, name)) \
		{EXIT(ERR_SEM, "Redefining symbol.\n"); htabFree(newTable); symbolFree(functionSymbol);name = NULL; break;  }			\
																				\
	if ((addedFunc = htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, functionSymbol, false)) == NULL) \
		{INTERN(); htabFree(newTable); symbolFree(functionSymbol); name = NULL; break;} \
	\
	newTable->Parent = addedFunc;\
	currentFunction = addedFunc;\
	printSymbol("Function", currentFunction);\
	symbolFree(functionSymbol);\
	name = NULL;\
} while (0)

 /*
  * Creates a new static variable symbol and stores it table of static variables and functions
  */
 #define createStaticVariable(type, defined, name)		\
 do{																										\
 	tSymbolPtr currentVariable = symbolNew();						\
 	if (currentVariable == NULL) {INTERN(); break;}	\
 	currentVariable->Type = type;																\
 	currentVariable->Const = true;																\
 	currentVariable->Defined = defined;														\
 	currentVariable->Name = name;\
	if(htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, name))\
		{EXIT(ERR_SEM, "Redefining symbol.\n"); symbolFree(currentVariable); name = NULL; break;  }			\
																				\
	if (htabAddSymbol(currentClass->Data.ClassData.LocalSymbolTable, currentVariable, false) == NULL)\
		{INTERN(); symbolFree(currentVariable); name = NULL; break;}\
	printSymbol("Static variable", currentVariable);\
	symbolFree(currentVariable);\
	name = NULL;\
 } while (0)


 /*
  * Creates a new local variable or parameter symbol and stores it table of function variables
  */

 #define createFunctionVariable(type, defined, name, isArgument)                                        \
 do{                                                                                                     \
 	tSymbolPtr currentVariable = symbolNew();                                                             \
	tSymbolPtr addedVar;\
	if (currentVariable == NULL) {INTERN(); break;}                                                  \
	currentVariable->Type = type;																                                          \
	currentVariable->Const = false;																                                        \
	currentVariable->Defined = defined;															                                      \
	currentVariable->Name = name;                                                                        \
	if((currentFunction != NULL && htabGetSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, name)) || (currentClass != NULL && (addedVar = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, name)) != NULL && addedVar->Type == eFUNCTION))\
		{EXIT(ERR_SEM, "Redefining symbol.\n"); symbolFree(currentVariable); name = NULL; break;  }			\
																				\
	if ((addedVar = htabAddSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, currentVariable, false)) == NULL) \
	    {INTERN(); symbolFree(currentVariable); name = NULL; break;}\
\
	printSymbol("Function variable", currentVariable);                                                    \
	if (isArgument){                                                                                      \
        if(!symbolFuncAddArgument(currentFunction, addedVar))\
            {INTERN(); symbolFree(currentVariable); name = NULL; break;}\
	}\
\
    symbolFree(currentVariable);\
	name = NULL;\
 } while (0)

#define getNewToken(token, errCode)   \
do{                                   \
	cleanToken(&token);							    \
	errCode = getToken(token);					\
	CHECK_ERRCODE();                  \
} while (0)

extern tHashTablePtr globalScopeTable;
extern tConstContainerPtr constTable;


/*
 */
static eError skipPrecedenceParsing(eError errCode);
eError skipFunctionCall(eError errCode); //TODO:: is it really used? in header also...
static eError classList();
static eError classBody();
static eError funcBody();
static eError stmt();
static eError var();

// global variables - used in multiple functions in parser.c and expr.c //nope they are not...
static Token *token;
static tSymbolPtr currentFunction;
static tSymbolPtr currentClass;
static dtStrPtr symbolName;
static eSymbolType symbolTokenType;

tSymbolPtr checkForIdConflictWithinClass(tSymbolPtr symbol, void* name){
	if ((strCmpStr(symbol->Name, (dtStrPtr)name)) == 0)
		return NULL;

	if (symbol->Type == eFUNCTION && htabGetSymbol(symbol->Data.FunctionData.LocalSymbolTable, name))
		return NULL;

  return symbol;
}

/*
 * A mock of expression evaluation, which I don't really need in 1. run,
 * but need to check for correct syntax and skip tokens.
 */
eError skipPrecedenceParsing(eError errCode){
  while (token->type != TT_semicolon) {
    getNewToken(token, errCode);
    if (token->type == TT_EOF)
      return ERR_SYNTAX;
  }
  return ERR_OK;
}

/*
 * A mock of function call, which I don't generate in 1. run,
 * but need to check for correct syntax and skip tokens.
 */
eError skipFunctionCall(eError errCode) {
  uint32_t counter = 1;
  while (token->type < TT_assignment && counter != 0) {
    getNewToken(token, errCode);
    if (token->type == TT_leftRoundBracket) {
    	counter++;
    }
    if (token->type == TT_rightRoundBracket) {
    	counter--;
    }
    if (token->type == TT_EOF)
      return ERR_SYNTAX;
  }

  if (counter == 0) {
  	return ERR_OK;
  } else {
  	return ERR_SYNTAX;
  }
}


eError initializeHelperVariables(){
  if ((token = newToken()) == NULL){
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
        goto freeResourcesAndFinish;
    }

	//recursive parsing
	if (token->type == TT_keyword && token->keywordType == KTT_class) {
		if ((errCode = classList()) != ERR_OK)
            goto freeResourcesAndFinish;
	}
	//if 1. Token != [<KTT_CLASS>] I'm expecting EOF, otherwise syntactic error
	if (token->type != TT_EOF)
        EXIT(ERR_SYNTAX, "Unexpected token violating {PROG -> CLASS_LIST eof}\n");

freeResourcesAndFinish:
	freeHelperVariables();
	return errCode;
}

eError classList() {

	eError errCode = ERR_OK;
	//[<KTT_CLASS>] 2. Token -> [<TT_identifier>]
	//RULE: CLASS_LIST -> class simple_id { CLASS_BODY } CLASS_LIST
	getNewToken(token, errCode);
	if (token->type != TT_identifier){
	    EXIT(ERR_SYNTAX, "Identifier expected.\n");
		return ERR_SYNTAX;
    }

	// backup of what i want to call the symbol later
	if ((symbolName = strNewFromStr(token->str)) == NULL){
		INTERN();
		return ERR_INTERN;
	}


	//[<KTT_CLASS>][<TT_identifier>]3. Token -> [{]
	getNewToken(token, errCode);
	if (token->type != TT_leftCurlyBracket) {
		EXIT(ERR_SYNTAX, "{ expected.\n");
		CHECK_ERRCODE();
	}

	// Creating our first class symbol! This one will go to globalScopeTable and currentScopeTable will be changed
	createClass(symbolName);
	CHECK_ERRCODE();


	//[<KTT_CLASS>][<TT_identifier>][{]4. Token -> [<KTT_STATIC>]
	getNewToken(token, errCode);
	if (token->type == TT_keyword && token->keywordType == KTT_static) {
		//recursive call
		errCode = classBody();
		CHECK_ERRCODE();
		currentClass = NULL;
	}

	//[<KTT_CLASS>][<TT_identifier>][{]4. Token -> [}]
	if (token->type == TT_rightCurlyBracket){
		//[<KTT_CLASS>][<TT_identifier>][{][}]5. Token -> [fist token of CLASS_LIST]
		getNewToken(token, errCode);

		if (token->type == TT_keyword && token->keywordType == KTT_class) {
			// I'm over with first class, let's go to another one !
			// Recursive call of classList()
			errCode = classList();
			CHECK_ERRCODE();
		}
		//[<KTT_CLASS>][<TT_identifier>][{][}]5. Token -> [not fist token of CLASS_LIST], returning to prog()
		// Current token be other than EOF, the check is in prog()
		return errCode;
	}
	else{
		// not [static], nor [}]
	    EXIT(ERR_SYNTAX, "Declaration expected.\n");
		return ERR_SYNTAX;
    }

  return errCode;
}

eError classBody() {
	eError errCode = ERR_OK;

	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>]5.Token -> [<KTT_*>]
	//RULE: CLASS_BODY -> static type simple_id VAR_OR_FUNC CLASS_BODY
	getNewToken(token, errCode);
	if (token->type != TT_keyword
	    && token->keywordType != KTT_boolean
	    && token->keywordType != KTT_String
	    && token->keywordType != KTT_double
	    && token->keywordType != KTT_int
	    && token->keywordType != KTT_void) {

        EXIT(ERR_SYNTAX, "Type declaration expected.\n");
		return ERR_SYNTAX;
	}

	TTtoeSymbolType(token->keywordType, symbolTokenType);
    getNewToken(token, errCode);
	if (token->type != TT_identifier){
		EXIT(ERR_SYNTAX, "Identifier expected.\n");
		return ERR_SYNTAX;
	}

	//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>]6.->[<TT_identifier>]
	if ((symbolName = strNewFromStr(token->str)) == NULL){
		INTERN();
		return ERR_INTERN;
	}


	//read next token - now we will find out, if it is function or identifier
	//VAR_OR_FUNC -> ( PARAM ) { FUNC_BODY }
	getNewToken(token, errCode);
	switch(token->type) {
		case TT_semicolon:
			// Variable without initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][;]
			if (symbolTokenType == eNULL){
				EXIT(ERR_SYNTAX, "Type declaration expected\n");
				return ERR_SYNTAX;
			}

			// treba overit co sa vyhodnocuje (iba konstanty + uz zname staticke) - asi check vo vyrazoch - over
      			createStaticVariable(symbolTokenType, false, symbolName);
			CHECK_ERRCODE();

			getNewToken(token, errCode);
			break;

		case TT_assignment:
			//we have variable with initialization
			//[<KTT_CLASS>][<TT_identifier>][{][<KTT_STATIC>][<KTT_*>][<TT_identifier>][=]
			//after assignment have an expression - we call expressions parsing

			// if variable and i didn't find symbolTokenType in TTtoeSymbolType() -> uknown type -> error
			if (symbolTokenType == eNULL){
				EXIT(ERR_SYNTAX, "Type declaration expected\n");
				return ERR_SYNTAX;
			}
			errCode = skipPrecedenceParsing(errCode);
			CHECK_ERRCODE();

			createStaticVariable(symbolTokenType, false, symbolName);
			CHECK_ERRCODE();

			//expressions parsing read one token outside of expression - has to be semicolon
			if (token->type != TT_semicolon){
		        EXIT(ERR_SYNTAX, "; expected\n");
				return ERR_SYNTAX;
			}

			getNewToken(token, errCode);
			break;

		case TT_leftRoundBracket:
			createFunction(symbolTokenType, true, symbolName);
			CHECK_ERRCODE();

			//reading all parameters
			getNewToken(token, errCode);
			//stops, when right round bracket is read - end of parameters
			while (token->type != TT_rightRoundBracket) {

				if (token->type != TT_keyword
				 && token->keywordType != KTT_boolean
				 && token->keywordType != KTT_String
				 && token->keywordType != KTT_double
				 && token->keywordType != KTT_int) {
					EXIT(ERR_SYNTAX, "Type declaration expected.\n");
					return ERR_SYNTAX;
				}

				//we have a type
				// adds argument as a local variable to function scope
				TTtoeSymbolType(token->keywordType, symbolTokenType);
				getNewToken(token, errCode);

				if (token->type != TT_identifier){
					EXIT(ERR_SYNTAX, "Identifier expected.\n");
					return ERR_SYNTAX;
				}

				//we have an id
				if ((symbolName = strNewFromStr(token->str)) == NULL){
					INTERN();
					return ERR_INTERN;
				}

				//read next token
				getNewToken(token, errCode);
				//if it is comma - we read another token and next cycle will check if it is token we want (type)
				if (token->type == TT_comma) {
					getNewToken(token, errCode);
					//but we can't get bracket right after comma
					if (token->type == TT_rightRoundBracket){
						EXIT(ERR_SYNTAX, "Type declaration expected.\n");
						return ERR_SYNTAX;
					}
				}


                // last true for isArgument, updating argument list of currentFunction
				createFunctionVariable(symbolTokenType, true, symbolName, true);
		        CHECK_ERRCODE();
            }

			//read next token - it should be left curly bracket
			//RULE: VAR_OR_FUNC -> ( PARAM ) { FUNC_BODY }
 			getNewToken(token, errCode);
			if (token->type != TT_leftCurlyBracket) {
				EXIT(ERR_SYNTAX, "{ expected.\n");
				return ERR_SYNTAX;
			}

			getNewToken(token, errCode);
			if (token->type != TT_rightCurlyBracket) {
				//recursive call
				errCode = funcBody();
				CHECK_ERRCODE();
				currentFunction = NULL;

				//checks if there really is right curly bracket after funcBody
				if (token->type != TT_rightCurlyBracket) {
					EXIT(ERR_SYNTAX, "} expected.\n");
					return ERR_SYNTAX;
				}
			}

			getNewToken(token, errCode);
			break;

		default:
			EXIT(ERR_SYNTAX, "Declaration/definition expected. Missing ';' ?\n");
			return ERR_SYNTAX;
	}

	//token to determine what to do next
	//if it is right curly bracket - this is end of classBody
	if (token->type == TT_rightCurlyBracket) {
		return ERR_OK;
	}

	//if it is keyword static - it is first token of next classBody
	if (token->type == TT_keyword && token->keywordType == KTT_static)
		//recursive call
		errCode = classBody();

	return errCode;
}

eError funcBody() {

	eError errCode = ERR_OK;
  do {
  	//RULE: VAR -> type ID INITIALIZE
  	if (token->type == TT_keyword
       && (token->keywordType == KTT_boolean
       || token->keywordType == KTT_double
       || token->keywordType == KTT_int
       || token->keywordType == KTT_String)) {
  		  //current token is keyword type - we call var
  	 	  errCode = var();
  	 	  CHECK_ERRCODE();
  	 	  getNewToken(token, errCode);
  	} else {
  		//current token isnt a keyword or isnt a type - this could be STMT - we call stmt()
  		errCode = stmt();
  		CHECK_ERRCODE();
  	}
  }while (token->type != TT_rightCurlyBracket);

  return errCode;
}

eError stmtBody(){
	eError errCode = ERR_OK;
	getNewToken(token, errCode);
	while (token->type != TT_rightCurlyBracket) {
		if (token->type == TT_keyword
		  && (token->keywordType == KTT_boolean
		      || token->keywordType == KTT_double
		      || token->keywordType == KTT_int
		      || token->keywordType == KTT_String)) {

		  //trying to declare a new variable inside a scope
		  EXIT(ERR_SYNTAX, "Declarations in scope are not permitted.\n");
		  return ERR_SYNTAX;
		} else {
		  //current token isnt a keyword or isnt a type - this could be STMT - we call stmt()
		  errCode = stmt();
		  CHECK_ERRCODE();
		}
	}
  return errCode;
}

eError stmt() {

	eError errCode = ERR_OK;

	//RULE: STMT -> { STMT_LIST }
	switch(token->type) {

		case TT_leftCurlyBracket:
			errCode = stmtBody();
			CHECK_ERRCODE();
			getNewToken(token, errCode);
			break;

		case TT_keyword:
			switch(token->keywordType){

				//RULE: STMT -> return RETURN_VAL ;
				case KTT_return:

					//we call expressions parsing to parse RETURN_VAL
					//if there wasn't RETURN_VAR, result from expressions parsing will be NULL
					errCode = skipPrecedenceParsing(errCode);
					CHECK_ERRCODE();

					//expressions parsing could stop on semicolon or right round bracket before semicolon
					//otherwise, there is syntax error
					if (token->type != TT_semicolon) {
						getNewToken(token, errCode);
						if (token->type != TT_semicolon) {
							EXIT(ERR_SYNTAX, "; expected.\n");
							return ERR_SYNTAX;
						}
					}
					getNewToken(token, errCode);
					break;

				//STMT -> if ( EXPR ) STMT ELSE
				case KTT_if:

					//next token have to be left round bracket
					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						EXIT(ERR_SYNTAX, "( expected.\n");
						return ERR_SYNTAX;
					}

					//call expressions parsing - parse condition
					errCode = skipFunctionCall(errCode);
					CHECK_ERRCODE();

					//there have to be right round bracket after condition
					if(token->type != TT_rightRoundBracket) {
						EXIT(ERR_SYNTAX, ") expected.\n");
						return ERR_SYNTAX;
					}

					getNewToken(token, errCode);
					errCode = stmt();

					CHECK_ERRCODE();
					break;

		        case KTT_else:
					getNewToken(token, errCode);
					errCode = stmt();

					CHECK_ERRCODE();
					break;


				//STMT -> while ( EXPR ) STMT
				case KTT_while:

					getNewToken(token, errCode);
					if (token->type != TT_leftRoundBracket) {
						EXIT(ERR_SYNTAX, "( expected.\n");
						return ERR_SYNTAX;
					}

					errCode = skipFunctionCall(errCode);
                    CHECK_ERRCODE();

					//there have to be right round bracket after condition
					if(token->type != TT_rightRoundBracket) {
						EXIT(ERR_SYNTAX, ") expected.\n");
						return ERR_SYNTAX;
					}

                    getNewToken(token, errCode);
					errCode = stmt();

					CHECK_ERRCODE();
					break;

				default:
					EXIT(ERR_SYNTAX, "Statment expected.\n");
					return ERR_SYNTAX;
			}
			break;

		case TT_increment:
		case TT_decrement:
			errCode = skipPrecedenceParsing(errCode);
			CHECK_ERRCODE();
			if(token->type != TT_semicolon) {
				EXIT(ERR_SYNTAX, "; expected.\n");
				return ERR_SYNTAX;
			}
			getNewToken(token, errCode);
			break;

		case TT_identifier:
		case TT_fullIdentifier: {
			//not LL case
			//might be assignment or function call or id++/--

			getNewToken(token, errCode);

			if (token->type == TT_assignment || token->type == TT_increment || token->type == TT_decrement) {
				errCode = skipPrecedenceParsing(errCode);
				CHECK_ERRCODE();

            } else if (token->type == TT_leftRoundBracket){
				errCode = skipFunctionCall(errCode);
				CHECK_ERRCODE();
				getNewToken(token, errCode);

			} else {
				EXIT(ERR_SYNTAX, "Assignment, unary operator or function call expected.\n");
				return ERR_SYNTAX;
			}

			if (token->type != TT_semicolon){
				EXIT(ERR_SYNTAX, "; expected.\n");
				return ERR_SYNTAX;
			}
			getNewToken(token, errCode);
			break;
		}

		case TT_EOF:
			EXIT(ERR_SYNTAX, "Unexpected end of file.\n");
			return ERR_SYNTAX;
		default:
			EXIT(ERR_SYNTAX, "Unexpected token.\n");
			return ERR_SYNTAX;
	}


	return ERR_OK;

}

eError var() {

	eError errCode = ERR_OK;

	TTtoeSymbolType(token->keywordType, symbolTokenType);
	getNewToken(token, errCode);

	if (token->type != TT_identifier){
		EXIT(ERR_SYNTAX, "Identifier expected.\n");
		return ERR_SYNTAX;
	}

	if ((symbolName = strNewFromStr(token->str)) == NULL) {
		INTERN();
		return ERR_INTERN;
	}

	//TODO::->deleted part - Can be identifier empty? ""

	createFunctionVariable(symbolTokenType, true, symbolName, false);
	CHECK_ERRCODE();

	getNewToken(token, errCode);
	// INITIALIZE ->     = EXPR ;
	if (token->type == TT_assignment) {
		// [reserved_name] [=]  -> Nope
		errCode = skipPrecedenceParsing(errCode);
		CHECK_ERRCODE();
	}

	if (token->type != TT_semicolon) {
		EXIT(ERR_SYNTAX, "Assignment expected. Missing ';'?\n");
		return ERR_SYNTAX;
	}

	return ERR_OK;
}
