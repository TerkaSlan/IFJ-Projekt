/**
  *  \file parser.h
  *  \brief Contains declarations structures and functions regarding the first run of parser
  *  \author Terka
  *  \author Katka
  *  \author Michal
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


#ifndef PARSER_H
#define PARSER_H

#include "constants.h"
#include "ial.h"
#include "error.h"
#include <stdint.h>

/*
*  Converts between token type and symbol type
*/
#define TTtoeSymbolType(ktt, eSymbol)\
do{                                  \
  switch (ktt){                      \
    case KTT_int:                    \
    eSymbol = eINT; break;         \
    case KTT_double:                 \
    eSymbol = eDOUBLE; break;      \
    case KTT_boolean:                \
    eSymbol = eBOOL; break;        \
    case KTT_String:                 \
    eSymbol = eSTRING; break;      \
    default:                         \
    eSymbol = eNULL;               \
  }                                  \
}while(0)

#define createFunctionVariable(type, defined, name, isArgument)                                        \
do{                                                                                                     \
 tSymbolPtr currentVariable = symbolNew();                                                             \
 tSymbolPtr addedVar;\
 if (currentVariable == NULL) {INTERN(); break;}                                                  \
 currentVariable->Type = type;																                                          \
 currentVariable->Const = false;																                                        \
 currentVariable->Defined = defined;															                                      \
 currentVariable->Name = name;                                                                        \
 if((currentFunction != NULL && htabGetSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, name)) || (currentClass != NULL && (addedVar = htabGetSymbol(currentClass->Data.ClassData.LocalSymbolTable, name)) != NULL && addedVar->Type == eFUNCTION))\
   {EXIT(ERR_SEM, "Redefining symbol.\n"); symbolFree(currentVariable); name = NULL; break;  }			\
                                       \
 if ((addedVar = htabAddSymbol(currentFunction->Data.FunctionData.LocalSymbolTable, currentVariable, false)) == NULL) \
     {INTERN(); symbolFree(currentVariable); name = NULL; break;}\
\
 if (isArgument){                                                                                      \
       if(!symbolFuncAddArgument(currentFunction, addedVar))\
           {INTERN(); symbolFree(currentVariable); name = NULL; break;}\
 }\
\
   symbolFree(currentVariable);\
 name = NULL;\
} while (0)

#define INTERN() EXIT(ERR_INTERN, "Error allocating new space. Out of memory.\n")

eError fillSymbolTable();
eError skipFunctionCall(eError errCode);
#endif
