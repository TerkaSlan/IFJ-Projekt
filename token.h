#ifndef TOKEN_H
#define TOKEN_H

#include "str.h"

typedef enum {

	TT_empty,
	TT_variable,
	TT_identifier,
	TT_string,
	TT_double,
	TT_number,
	TT_keyword
	//TODO: add all possible token types

} TokenType;

typedef enum {

	KTT_boolean,
	KTT_break,
	KTT_class,
	KTT_continue,
	KTT_do,
	KTT_double,
	KTT_else,
	KTT_false,
	KTT_for,
	KTT_if,
	KTT_int,
	KTT_return,
	KTT_String,
	KTT_static,
	KTT_true,
	KTT_void,
	KTT_while

} KeywordTokenType;

typedef struct {

    TokenType type;
    union {
        double dNum;
        int32_t iNum;
        KeywordTokenType keywordType;
        dtStr str;
    };

} Token;

/**
 * Create new token and initializes it to empty state
 * 
 */
Token* newToken();

/**
 * Deallocates memory of token's content if needed, deallocates token and sets pointer to NULL.
 *  
 * @param pToken token to free
 */
void freeToken(Token **pToken);

#endif