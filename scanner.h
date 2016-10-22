#ifndef SCANNER_H
#define SCANNER_H
#include <ctype.h>
#include "token.h"
#include "conversions.h"
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
 * @param      string      string portion of the analyzed token
 * @return     Type of keyword
 */
int8_t getToken(Token *token);

/**
 * Handles any error encountered during lexical analysis
 */
#define handleError(token, iCurrentSymbol, errorType)             \
do {                                                              \
    freeToken(token);                                             \
    printError(ERR_INTERN, " in %s at %s", __FILE__, __LINE__);   \
    return errorType;                                             \
} while (0)
#endif
