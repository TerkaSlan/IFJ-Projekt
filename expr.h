//EXPRESIONS.H

#ifndef EXPR_H
#define EXPR_H

#include "token.h"
#include "ial.h"
#include "error.h"
#include <stdint.h>

typedef struct { 

	int64_t top;
	uint32_t capacity;
	uint32_t *data;

} tPrecedenceStack, *tPrecedenceStackPtr;

typedef struct {

	TokenType type;
	union {
			
		dtStrPtr stringOrId;
		double dNum;
		int32_t iNum;
		int32_t boolean;
		tSymbolPtr symbol;
		
	};

} tPrecedenceSymbol, *tPrecedenceSymbolPtr;


typedef struct {

	int64_t top;
	uint32_t capacity;
	tPrecedenceSymbolPtr data;

} tSymbolStack, *tSymbolStackPtr;


/**
 * Allocates and initializes new precedence stack
 * 
 * @return 
 */
tPrecedenceStackPtr precedenceStackNew();

/**
 * [precedenceStackFree description]
 * @param stack [description]
 */
void precedenceStackFree(tPrecedenceStackPtr stack);

/**
 * [precedenceStackTop description]
 * @param  stack [description]
 * @return       terminal closest to top
 */
uint32_t precedenceStackTopTerminal(tPrecedenceStackPtr stack);



/**
 * [precedenceStackPush description]
 * @param  stack [description]
 * @param  item  [description]
 * @return       [description]
 */
int64_t precedenceStackPush(tPrecedenceStackPtr stack, uint32_t item); 


uint32_t precedenceStackPop(tPrecedenceStackPtr stack);

eError precedenceStackShift(tPrecedenceStackPtr stack);

tPrecedenceSymbolPtr newPrecedenceSymbol();

void freePrecedenceSymbol(tPrecedenceSymbolPtr symbol);

void cleanPrecedenceSymbol(tPrecedenceSymbolPtr symbol);

tSymbolStackPtr SymbolStackNew();

void symbolStackFree(tSymbolStackPtr stack);

int64_t symbolStackPush(tSymbolStackPtr stack, tPrecedenceSymbolPtr item);

tPrecedenceSymbolPtr symbolStackPop(tSymbolStackPtr stack);

eError functionParse(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

void printStack(tPrecedenceStackPtr stack);

eError precedenceParsing(Token* helpToken);

#endif