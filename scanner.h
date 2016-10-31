#ifndef SCANNER_H
#define SCANNER_H
#include <ctype.h>
#include "token.h"
#include "conversions.h"
#include "error.h"
#include "str.h"

typedef enum
{
    SEmpty,
    SId,
    SFullId,
    SEqual,
    SNotEqual,
    SNumber,
    SDecimal,
    SDouble,
    SDoubleDecimalPart,
    SDoubleExponentPart,
    SDoubleExponentSign,
    SDoubleExponent,
    SAssignment,
    SGreater,
    SLess,
    SDivide,
    SBlockCommentStart,
    SBlockCommentFinish,
    SComment,
    SExclamation,
    SString,
    SEscape,
    SOctal
} tFSMState;

#define MIN_ASCII_VALUE 32  // min value to be recognized as an ASCII assocring to specs
/**
 * Opens the file located at sFileLocation and loads global variable
 * fSourceFile with the filestream
 *
 * @param      sFileLocation      Input file location
 * @return     ERR_OTHER if an error was encountered, ERR_OK otherwise
 */
int8_t openFile(const char *sFileLocation);
/**
 * Decides whether a token adept ready to be an identifier isn't a keyword
 *
 * @param      string      string portion of the analyzed token
 * @return     Type of keyword
 */
KeywordTokenType keywordType(const dtStr *string);
/**
 * Analyzes incoming characters from fSourceFile and by the use of FSM logic
 * manufactures the tokens needed by syntactic analyzer
 *
 * @param      token     Token to be filled
 * @return     ERR_OK in the case of success, ERR_INTERN or ERR_LEX in the case of failure
 */
int8_t getToken(Token *token);

/**
 * Handles any error encountered during lexical analysis
 */
#define handleLexError(token, errorType)                                \
do {                                                                    \
    if (token->str != NULL)   strFree(token->str);                      \
    freeToken(&token);                                                  \
    printError(ERR_LEX, " in %s at line: %d", __FILE__, __LINE__);      \
    return errorType;                                                   \
} while (0)
#endif
