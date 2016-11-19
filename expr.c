//TODO
//
//	unary ++,--
//
//	unary -

#include "expr.h"
#include "scanner.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

#define PRECEDENCE_STACK_DEFAULT_SIZE 32
#define SYMBOL_STACK_DEFAULT_SIZE 32
#define NONTERMINALBORDER 25

// global variable from parser.c
extern Token* token;
extern tSymbolPtr result;

const uint32_t precedenceTable[27][27] =
{

//    +   -   *   /   <   >  <=  >=  ==  !=  id f.id str  d   i   b  ++  --  not and or   (   )   ,   $   ;   {
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','<','<','<','<','>','>','<','>','>','x','>','x'}, //TT_plus,
	{'>','>','<','<','>','>','>','>','>','>','<','<','<','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_minus,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_multiply,
	{'>','>','>','>','>','>','>','>','>','>','<','<','<','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_divide,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_less,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_greater,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_lessEqual,
	{'<','<','<','<','x','x','x','x','>','>','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_greaterEqual,
	{'<','<','<','<','<','<','<','<','x','x','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_equal,
	{'<','<','<','<','<','<','<','<','x','x','<','<','x','<','<','x','<','<','<','>','>','<','>','>','x','>','x'}, //TT_notEqual,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','>','>','x','>','>','f','>','>','x','>','x'}, //TT_identifier,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','>','>','x','>','>','f','>','>','x','>','x'}, //TT_fullIdentifier,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','>','>','x','>','>','x','>','x'}, //TT_string,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','>','>','x','>','>','x','>','x'}, //TT_double,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','>','x','>','>','x','>','>','x','>','x'}, //TT_number,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','x','x','x','>','>','x','>','>','x','>','x'}, //TT_boolean,
	{'>','>','>','>','>','>','>','>','>','>','u','u','x','x','x','x','>','>','<','>','>','<','>','>','x','>','x'}, //TT_increment,
	{'>','>','>','>','>','>','>','>','>','>','u','u','x','x','x','x','>','>','<','>','>','<','>','>','x','>','x'}, //TT_decrement,
	{'>','>','>','>','>','>','>','>','>','>','<','<','x','x','x','<','<','<','<','>','>','<','>','>','x','>','x'}, //TT_not,
	{'<','<','<','<','<','<','<','<','<','<','<','<','x','x','x','<','<','<','<','>','>','<','>','>','x','>','x'}, //TT_and,
	{'<','<','<','<','<','<','<','<','<','<','<','<','x','x','x','<','<','<','<','>','>','<','>','>','x','>','x'}, //TT_or,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','=','x','x','>','x'}, //TT_leftRoundBracket,
	{'>','>','>','>','>','>','>','>','>','>','x','x','x','x','x','x','>','>','>','>','>','x','>','x','x','>','x'}, //TT_rightRoundBracket,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','x','>','x','x','>','x'}, //TT_comma,
	{'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','e','e','x','e','x'},  //TT_dolar
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','>','x'}, //TT_semicolon
	{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'} //TT_leftCurlyBracket

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

	if (stack == NULL || stack->data == NULL) {
		return ERR_INTERN;
	}

	for (int32_t i = stack->top; i > -1; i--) {
		if (stack->data[i] <= NONTERMINALBORDER) {
			return stack->data[i];
		}
	}

	return ERR_INTERN; // does this have to be here? there should always be at least $ sign

}

int64_t precedenceStackPush(tPrecedenceStackPtr stack, uint32_t item) {

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

	return stack->top;

}

uint32_t precedenceStackPop(tPrecedenceStackPtr stack) {

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


tPrecedenceSymbolPtr newPrecedenceSymbol() {

	tPrecedenceSymbolPtr symbol = calloc(1, sizeof(tPrecedenceSymbol));
	return symbol;

}

void freePrecedenceSymbol(tPrecedenceSymbolPtr symbol) {
	if (symbol != NULL) {
		cleanPrecedenceSymbol(symbol);
		free(symbol);
	}
}


void cleanPrecedenceSymbol(tPrecedenceSymbolPtr symbol) {

	if ((symbol->type == TT_string)
	 || (symbol->type == TT_identifier
	 || (symbol->type == TT_fullIdentifier))) {
		strFree(symbol->stringOrId);
	}

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
		if ((stack->data[i].type == TT_string)
	 	 || (stack->data[i].type == TT_identifier)
	 	 || (stack->data[i].type == TT_fullIdentifier)) {

	 		strFree(stack->data[i].stringOrId);

		}
	}
	//TODO treba uvolnovat aj symbol?

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
	//TODO pridat pracu so symbol
	if ((item->type == TT_string)
	 || (item->type == TT_identifier
	 || (item->type == TT_fullIdentifier))) {

		stack->data[stack->top].stringOrId = strNew();
		strCopyStr(stack->data[stack->top].stringOrId, item->stringOrId);

	} else {

		stack->data[stack->top].stringOrId = item->stringOrId;

	}

	return stack->top;

}

tPrecedenceSymbolPtr symbolStackPop(tSymbolStackPtr stack) {

	if (stack->top > -1) {

		tPrecedenceSymbolPtr symbol = newPrecedenceSymbol();
		if (symbol == NULL) {
			return NULL;
		}

		symbol->type = stack->data[stack->top].type;
		//TODO: spracovat symbol
		if ((symbol->type == TT_string)
		 || (symbol->type == TT_identifier
		 || (symbol->type == TT_fullIdentifier))) {

			symbol->stringOrId = strNew();
			strCopyStr(symbol->stringOrId, stack->data[stack->top].stringOrId);

		} else {

			symbol->stringOrId = stack->data[stack->top].stringOrId;

		}

		return symbol;

	}

	return NULL;

}

eError functionParse(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	eError errCode;

	precedenceStackPop(stack);

	//vybrat nazov funkcie zo zasobnika symbolov

	printStack(stack);
	while (token->type != TT_rightRoundBracket) {

		errCode = precedenceParsing(NULL);
		if (errCode != ERR_OK) {
			return errCode;
		}
		//mam parameter

		//expressions parsing stoped on something else then right round bracket or comma - syntax error
		if (token->type != TT_rightRoundBracket && token->type != TT_comma) {
			return ERR_SYNTAX;
		}
		printStack(stack);

	}

	printStack(stack);
	cleanToken(&token);
	errCode = getToken(token);
	if (errCode != ERR_OK) {
		return errCode;
	}
	return ERR_OK;

}

eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack) {

	switch(precedenceStackTopTerminal(stack)) {

		//E -> ID
		case TT_identifier:
		case TT_fullIdentifier: {


			int64_t id = precedenceStackPop(stack);
			if (id != TT_identifier && id != TT_fullIdentifier){
				return ERR_INTERN;
			}

			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			// vybrat hornu hodnotu zo zasobnika premennych
			// spravit semanticke kontroly
			// do zasobnika premennych dat odkaz na premennu

			precedenceStackPush(stack, TT_E);
			break;
		}

		// E -> const
		case TT_string:
		case TT_double:
		case TT_number:
		case TT_boolean:


			precedenceStackPop(stack);
			// + otestovat spravnost typu
			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			// vybrat hornu hodnotu zo zasobnika premennych
			// ulozit ju do tabulky konstatnt
			// do zasobnika premennych dat odkaz na konstantu

			precedenceStackPush(stack, TT_E);
			break;

		// E -> ( E )
		// E -> function call - zatial nevyriesene, mozno sa to presunie do extra funkcie
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
		// tu musia byt case: vsetky operatory co su pred E
		case TT_plus:
		case TT_minus:
		case TT_multiply:
		case TT_divide:
		case TT_less:
		case TT_lessEqual:
		case TT_greater:
		case TT_greaterEqual:
		case TT_equal:
		case TT_notEqual:
		case TT_and:
		case TT_or: {

			int64_t operand2 = precedenceStackPop(stack);
			if (operand2 != TT_E) {
				return ERR_SYNTAX;
			}
			//zikat ukazatel na hodnotu operandu2 - pop symbolStack

			int64_t operator = precedenceStackPop(stack);
			if (operator) {
				//TODO overit ze je to operator a ze je spravny
			}

			int64_t operand1 = precedenceStackPop(stack);
			if (operand1 != TT_E) {
				return ERR_SYNTAX;
			}
			//ziskat ukazatel na hodnotu operandu1 - pop symbolStack


			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			//semanticke kontroly na kompatibilitu typov operandov + generovanie instrukcii na pretypovanie ?
			//generovat instrukciu

			//na symbolStack ulozit ukazatel na vyslednu hodnotu

			precedenceStackPush(stack, TT_E);
			break;
		}

		// E -> ! E
		case TT_not:

			//ziskat hornu hodnotu zo zasobnika premennych
			//spravit semanticke kontroly
			//vytvorit instrukciu
			//do zasobnika premennych vlozit odkaz na vysledok instrukcie

			precedenceStackPop(stack);
			precedenceStackPop(stack);
			if (precedenceStackPop(stack) != TT_start) {
				return ERR_SYNTAX;
			}

			precedenceStackPush(stack, TT_E);
			break;

		// E -> ++ E
		// E -> -- E
		// E -> E ++
		// E -> E --
		case TT_increment: //pozor, poze byt aj prefixovy aj postfixovy
		case TT_decrement: //pozor

			printf("unary\n");
			break;

		default:
			return ERR_SYNTAX;
	}

	return ERR_OK;

}


//helper function - just for testing
void printStack(tPrecedenceStackPtr stack) {

	for (uint32_t i = 0; i <= stack->top; i++) {

		switch(stack->data[i]) {
			case TT_plus:
				printf("+|");
				break;
			case TT_minus:
				printf("-|");
				break;
			case TT_multiply:
				printf("*|");
				break;
			case TT_divide:
				printf("/|");
				break;
			case TT_less:
				printf("<|");
				break;
			case TT_greater:
				printf(">|");
				break;
			case TT_lessEqual:
				printf("<=|");
				break;
			case TT_greaterEqual:
				printf(">=|");
				break;
			case TT_equal:
				printf("==|");
				break;
			case TT_notEqual:
				printf("!=|");
				break;
			case TT_identifier:
				printf("id|");
				break;
			case TT_fullIdentifier:
				printf("full.id|");
				break;
			case TT_string:
				printf("str|");
				break;
			case TT_double:
				printf("double|");
				break;
			case TT_number:
				printf("int|");
				break;
			case TT_boolean:
				printf("bool|");
				break;
			case TT_increment:
				printf("++|");
				break;
			case TT_decrement:
				printf("--|");
				break;
			case TT_not:
				printf("!|");
				break;
			case TT_and:
				printf("&&|");
				break;
			case TT_or:
				printf(" || |");
				break;
			case TT_leftRoundBracket:
				printf("(|");
				break;
			case TT_rightRoundBracket:
				printf(")|");
				break;
			case TT_comma:
				printf(",|");
				break;
			case TT_semicolon:
				printf(";|");
				break;
			case TT_dolar: // 25
				printf("$|");
				break;
			case TT_keyword:
				printf("K|");
				break;
			case TT_assignment:
				printf("=|");
				break;
			case TT_rightCurlyBracket:
				printf("}|");
				break;
			case TT_leftCurlyBracket:
				printf("{|");
				break;
			case TT_empty: // 30
				printf("nic|");
				break;
			case TT_EOF:
				printf("eof|");
				break;
			case TT_E:
				printf("E|");
				break;
			case TT_start:
				printf("start|");
				break;
		}

	}
	printf("\n");

}

eError precedenceParsing(Token* helpToken) {

	tPrecedenceSymbolPtr symbol;
	if ((symbol = newPrecedenceSymbol()) == NULL) {
		return ERR_INTERN;
	}

	tPrecedenceStackPtr stack;
	if ((stack = precedenceStackNew()) == NULL) {
		freePrecedenceSymbol(symbol);
		return ERR_INTERN;
	}
	precedenceStackPush(stack, TT_dolar);

	tSymbolStackPtr symbolStack;
	if ((symbolStack = symbolStackNew()) == NULL) {

		freePrecedenceSymbol(symbol);
		precedenceStackFree(stack);
		return ERR_INTERN;
	}

	int64_t stackTop;
	eError errCode;

	if(helpToken == NULL) {
		cleanToken(&token);
		errCode = getToken(token);
		if (errCode != ERR_OK) {
			precedenceStackFree(stack);
			freePrecedenceSymbol(symbol);
			symbolStackFree(symbolStack);
			return errCode;
		}
	} else {
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
		symbolStackPush(symbolStack, symbol);
	}

	do {
		stackTop = precedenceStackTopTerminal(stack);

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
			return ERR_SYNTAX;
		}

		printf("stackTop:%ld, token->type: %d\n", stackTop, token->type);
		switch (precedenceTable[stackTop][token->type]) {

			case '=':

				precedenceStackPush(stack, token->type);
				if (token->type < TT_keyword) {//TODO: bude sa mozno este menit

					cleanPrecedenceSymbol(symbol);
					symbol->type = token->type;

					if ((symbol->type == TT_string)
	 				 || (symbol->type == TT_identifier
	 				 || (symbol->type == TT_fullIdentifier))) {

	 				 	symbol->stringOrId = strNew();
						strCopyStr(symbol->stringOrId, token->str);

					} else if (symbol->type == TT_double) {

						symbol->dNum = token->dNum;

					} else {

						symbol->iNum = token->iNum;

					}

					symbolStackPush(symbolStack, symbol);


				}

				cleanToken(&token);
				errCode = getToken(token);
				if (errCode != ERR_OK) {
					precedenceStackFree(stack);
					freePrecedenceSymbol(symbol);
					symbolStackFree(symbolStack);
					return errCode;
				}

				printStack(stack);
				break;

			case '<':

				precedenceStackShift(stack);
				precedenceStackPush(stack, token->type);
				if (token->type < TT_keyword) {//TODO: bude sa mozno este menit

					cleanPrecedenceSymbol(symbol);
					symbol->type = token->type;

					if ((symbol->type == TT_string)
	 				 || (symbol->type == TT_identifier
	 				 || (symbol->type == TT_fullIdentifier))) {

	 				 	symbol->stringOrId = strNew();
						strCopyStr(symbol->stringOrId, token->str);

					} else if (symbol->type == TT_double) {

						symbol->dNum = token->dNum;

					} else {

						symbol->iNum = token->iNum;

					}

					symbolStackPush(symbolStack, symbol);


				}
				cleanToken(&token);
				errCode = getToken(token);
				if (errCode != ERR_OK) {
					precedenceStackFree(stack);
					freePrecedenceSymbol(symbol);
					symbolStackFree(symbolStack);
					return errCode;
				}

				printStack(stack);
				break;

			case '>':


				errCode = reduce(stack, symbolStack);
				if (errCode != ERR_OK) {
					return errCode;
				}
				printStack(stack);
				continue;

			case 'f':
				errCode = functionParse(stack, symbolStack);
				if (errCode != ERR_OK) {
					return errCode;
				}
				printf("break\n");
				break;

			case 'u':
				printf("unary\n");
				//spracovat unary ++,--
				//lebo moze byt aj prefixove aj postfixove a ja neviem ktore je toto
				break;

			case 'e':

				if (symbolStack->top > -1) {
					result = symbolStackPop(symbolStack)->symbol;
				} else {
					result = NULL;
				}
				precedenceStackFree(stack);
				freePrecedenceSymbol(symbol);
				symbolStackFree(symbolStack);
				return ERR_OK;

			case 'x':

				precedenceStackFree(stack);
				freePrecedenceSymbol(symbol);
				symbolStackFree(symbolStack);
				return ERR_SYNTAX;

			default:

				precedenceStackFree(stack);
				freePrecedenceSymbol(symbol);
				symbolStackFree(symbolStack);
				return ERR_SYNTAX;

		}

	} while (token->type != TT_comma
		  || token->type != TT_rightRoundBracket
		  || token->type != TT_semicolon
		  || token->type != TT_rightCurlyBracket
		  || stackTop != TT_dolar); //TODO: determine all conditions for stoping

	if (symbolStack->top > -1) {
		result = symbolStackPop(symbolStack)->symbol;
	} else {
		result = NULL;
	}
	precedenceStackFree(stack);
	freePrecedenceSymbol(symbol);
	symbolStackFree(symbolStack);
	return ERR_OK;

}
