/**
  *  \file expr.h
  *  \brief Contains declarations structures and functions regarding expression handling
  *  \author Katka
  *
  * Project: IFJ
  * Implementace interpretu imperativního jazyka IFJ16.
  *
  * Description:
  * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2016.pdf
  *
  * Team:
  * Michal Charvát          (xcharv16)
  * Terézia Slanináková     (xslani06)
  * Katarína Grešová        (xgreso00)
  * Marek Schauer           (xschau00)
  * Jakub Handzuš           (xhandz00)
  */

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
 *
 * @param  stack pointer to precedence stack
 * @return       value of item on top of precedence stack
 */
int64_t precedenceStackPop(tPrecedenceStackPtr stack);

/**
 * Inserts value representing shift before top most terminal on stack
 *
 * @param  stack pointer to precedence stack
 * @return       ERR_OK if success, ERR_INTERN otherwise
 */
eError precedenceStackShift(tPrecedenceStackPtr stack);

/**
 * Allocates memory for new precedence symbol
 *
 * @return pointer to new precedence symbol
 */
tPrecedenceSymbolPtr precedenceSymbolNew();

/**
 * Deallocates precedence symbol and its data.
 *
 * @param symbol pointer to precedence symbol to be freed
 */
void precedenceSymbolFree(tPrecedenceSymbolPtr symbol);

/**
 * Deallocates data in symbol and sets its type to TT_empty
 *
 * @param symbol pointer to precedence symbol to be cleaned
 */
void precedenceSymbolClean(tPrecedenceSymbolPtr symbol);

/**
 * Allocates and initializes new precedence symbol stack.
 *
 * @return new precedence symbol stack, NULL if error
 */
tSymbolStackPtr symbolStackNew();

/**
 * Deallocates precedence symbol stack and its data.
 *
 * @param pointer to stack to be freed
 */
void symbolStackFree(tSymbolStackPtr stack);

/**
 * Push item to precedence symbol stack and increments top of precedence symbol stack
 *
 * @param  stack pointer to precedence symbol stack
 * @param  item  value of item to by pushed
 * @return       ERR_OK if success, ERR_INTERN otherwise
 */
eError symbolStackPush(tSymbolStackPtr stack, tPrecedenceSymbolPtr item);

/**
 * Returns value of item on top of precedence symbol stack and decrement top of precedence symbol stack
 *
 * @param  stack pointer to precedence symbol stack
 * @return       item on top of precedence symbol stack
 */
tPrecedenceSymbolPtr symbolStackPop(tSymbolStackPtr stack);

/**
 * Handle function call and all its parameters
 *
 * @param  stack       pointer to precedence stack
 * @param  symbolStack pointer to precedence symbol stack
 * @return             ERR_OK if success, correct error code otherwise
 */
eError functionParse(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

/**
 * Hande builtin call and all its parameters
 *
 * @param  builtin     pointer to dtStr containing name of called builtin
 * @param  stack       pointer to precedence stack
 * @param  symbolStack pointer to precedence symbol stack
 * @return             ERR_OK if success, correct error code otherwise
 */
eError builtinCall(dtStrPtr builtin, tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

/**
 * Reduces expression on top of precedence stack
 *
 * @param  stack       pointer to precedence stack
 * @param  symbolStack pointer to precedence symbol stack
 * @return             ERR_OK if success, correct error code otherwise
 */
eError reduce(tPrecedenceStackPtr stack, tSymbolStackPtr symbolStack);

/**
 * Entry function of expressions parsing. Allocates necessary varilble before calling precedenceParsing
 *
 * @param  helpToken pointer to Token, which will be pushed on precedence and precedence symbol stack before processing will start
 * @return           ERR_OK if success, correct error code otherwise
 */
eError precedenceParsing(Token* helpToken);

/**
 * Processes expression
 *
 * @param  helpToken pointer to Token, which will be pushed on precedence and precedence symbol stack before processing will start
 * @return           ERR_OK if success, correct error code otherwise
 */
eError parsing(Token* helpToken);

#endif
