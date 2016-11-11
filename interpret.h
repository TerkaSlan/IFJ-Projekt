//
// Created by Charvin on 29. 10. 2016.
//

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
