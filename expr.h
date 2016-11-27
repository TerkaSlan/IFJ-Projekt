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
 * Allocates and initializes new precedence stack.
 *
 * @return new precedence stack, NULL if error
 */
tPrecedenceStackPtr precedenceStackNew();

/**
 * Deallocates precedence stack and its data.
 *
 * @param pointer to stack to be freed
 */
void precedenceStackFree(tPrecedenceStackPtr stack);

/**
 * Finds terminal closest to top of given precedence stack.
 *
 * @param  stack pointer to precedence stack
 * @return       terminal closest to top, or ERR_INTERN if error (ERR_INTERN is equal to 99 and terminal value is less then 25)
 */
uint32_t precedenceStackTopTerminal(tPrecedenceStackPtr stack);

/**
 * Push item to precedence stack and increments top of precedence stack
 *
 * @param  stack pointer to precedence stack
 * @param  item  value of item to by pushed
 * @return       ERR_OK if success, ERR_INTERN otherwise
 */
eError precedenceStackPush(tPrecedenceStackPtr stack, uint32_t item);

/**
 * Returns value of item on top of precedence stack and decrement top of precedence stack
 * @param  stack pointer to precedence stack
 * @return       value of item on top of precedence stack
 */
int64_t precedenceStackPop(tPrecedenceStackPtr stack);

/**
 * Inserts value representing shift before top most terminal on stack
 * @param  stack pointer to precedence stack
 * @return       ERR_OK if success, ERR_INTERN otherwise
 */
eError precedenceStackShift(tPrecedenceStackPtr stack);

tPrecedenceSymbolPtr precedenceSymbolNew();

void precedenceSymbolFree(tPrecedenceSymbolPtr symbol);

void precedenceSymbolClean(tPrecedenceSymbolPtr symbol);

tSymbolStackPtr symbolStackNew();

void symbolStackFree(tSymbolStackPtr stack);

int64_t symbolStackPush(tSymbolStackPtr stack, tPrecedenceSymbolPtr item);

tPrecedenceSymbolPtr symbolStackPop(tSymbolStackPtr stack);

eError functionParse(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

eError builtinCall(dtStrPtr builtin, tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

void printStack(tPrecedenceStackPtr stack);

eError precedenceParsing(Token* helpToken);

eError parsing(Token* helpToken);

#endif
