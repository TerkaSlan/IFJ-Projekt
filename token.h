#ifndef TOKEN_H
#define TOKEN_H

#include "str.h"

typedef enum {

	TT_empty,
	TT_identifier,
	TT_string,
	TT_double,
	TT_number,
	TT_keyword,
	TT_notEqual,
	TT_not,
	TT_divide,
	TT_fullIdentifier,
	TT_assignment,
	TT_less,
	TT_lessEqual,
	TT_greater,
	TT_greaterEqual,
	TT_comma,
	TT_multiply,
	TT_semicolon,
	TT_minus,
	TT_plus,
	TT_rightCurlyBracket,
	TT_leftCurlyBracket,
	TT_rightRoundBracket,
	TT_leftRoundBracket,
	TT_EOF

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
        dtStrPtr str;
    } ;

} Token;

/**
 * Creates new token and initializes it to empty state
 *
 * @return new token or NULL when error
 */
Token* newToken();

/**
 * Deallocates memory of token's content if (*pToken)->type is one of the following: TT_fullIdentifier, TT_identifier, TT_string, TT_keyword. 
 * Deallocates token and sets pointer to NULL.
 *  
 * @param pToken token to free
 */
void freeToken(Token **pToken);

/**
 * Deallocates memory of token's content if (*pToken)->type is one of the following: TT_fullIdentifier, TT_identifier, TT_string, TT_keyword. 
 * Change (*pToken)->type to TT_empty
 * 
 * @param pToken token to clean
 */
void cleanToken(Token **pToken);

#endif