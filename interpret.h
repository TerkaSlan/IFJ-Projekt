/**
  *  \file interpret.h
  *  \brief Contains declarations structures and functions regarding the interpretation
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



#ifndef INTERPRET_H
#define INTERPRET_H


#include "error.h"
#include "ial.h"
#include "instruction.h"
#include "constants.h"

/**
 *
 * @param globalClassTable Pointer to the hash table with class symbols
 * @param instrList Instruction list to be interpreted
 * @return Error enum
 */
eError Interpret(tHashTablePtr globalClassTable, tInstructionListPtr instrList);

#endif //INTERPRET_H
