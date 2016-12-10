/**
  *  \file scanner.h
  *  \brief Contains declarations structures and functions regarding lexical analysis
  *  \author Terka
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

#ifndef SCANNER_H
#define SCANNER_H
#include <ctype.h>
#include "token.h"
#include "conversions.h"
#include "error.h"
#include "str.h"
#include <stdio.h> // FILE* TO ERASE: Just for testing purposes, won't be included in final build
extern FILE* fSourceFile; // TODO:: Just for testing purposes, won't be included in final build
// From scanner.c counting occurences of '\n'
extern uint32_t LineCounter;
/**
 * Opens the file located at sFileLocation and loads global variable
 * fSourceFile with the filestream
 *
 * @param      sFileLocation      Input file location
 * @return     ERR_OTHER if an error was encountered, ERR_OK otherwise
 */
eError openFile(const char *sFileLocation);
/**
 * Closes the file loaded in global variable fSourceFile
 */
void closeFile();
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
eError getToken(Token *token);

#endif
