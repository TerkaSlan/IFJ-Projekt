/**
  *  \file parser_second.h
  *  \brief Contains declarations structures and functions regarding the second run of parser
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

#ifndef PARSER2_H
#define PARSER2_H

#include "error.h"

/**
 *  \brief Entry point of second parsing, prepared source file and initializes global variables
 *  \return ERR_OK on success, any other eError on failure
 */
eError generateInstructions();

#endif
