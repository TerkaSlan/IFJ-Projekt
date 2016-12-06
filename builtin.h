/**
  *  \file builtin.h
  *  \brief Contains declarations structures and functions regarding builtin functions
  *  \author Jakub
  *  \author Marek
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
#include <stdint.h>				//for "uint32_t"
#include "str.h"
#include "error.h"
#include "ial.h"

#ifndef BUILTIN_H
#define BUILTIN_H


/**
 * 	Creates a new dtStr which contains substring of parameter 's' starting from 'beginIndex' with length 'length'.
 * 	WARNING - after using, must be freed returned dtStr.
 * 	Example s->str="abcde", substring(s,2,3) = "cde"
 *
 * @param      s           dtStr
 * @param[in]  beginIndex  The begin index
 * @param[in]  length      Length of substring
 * @param      subStr      New substring (uses strNew())
 *
 * @return     Error code
 */
eError substr(const dtStr *s, int32_t beginIndex, int32_t length, dtStrPtr *subStr);

/**
 * Reads data from stdin, and saves it to 'data' according to data type on 'symbol->Type'
 *
 * @param[in]  symbol  tSymbolPtr
 * @param      data    tSymbolData*
 *
 * @return     Error code
 */
eError readData(tSymbolPtr symbol, tSymbolData* data);

#endif
