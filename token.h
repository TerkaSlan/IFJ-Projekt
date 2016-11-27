/*
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

#ifndef TOKEN_H
#define TOKEN_H

#include "str.h"

typedef enum {

	TT_plus,
	TT_minus,
	TT_multiply,
	TT_divide,
	TT_less,
	TT_greater,
	TT_lessEqual,
	TT_greaterEqual,
	TT_equal,
	TT_notEqual,
	TT_identifier,	//10
	TT_fullIdentifier,
	TT_string,
	TT_double,
	TT_number,
	TT_boolean,
	TT_increment,
	TT_decrement,
	TT_not,
	TT_and,
	TT_or,
	TT_leftRoundBracket,
	TT_rightRoundBracket,
	TT_comma,
	TT_semicolon,
	TT_dolar, // 25
	TT_keyword,
	TT_assignment,
	TT_rightCurlyBracket,
	TT_leftCurlyBracket,
	TT_empty, // 30
	TT_EOF,
	TT_E,
	TT_void,
	TT_start

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
	KTT_while,
	KTT_none

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
 * Changes (*pToken)->type to TT_empty
 *
 * @param pToken token to clean
 */
void cleanToken(Token **pToken);

#endif
