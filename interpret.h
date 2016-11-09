//
// Created by Charvin on 29. 10. 2016.
//

#ifndef INTERPRET_H
#define INTERPRET_H


#include "error.h"
#include "ial.h"
#include "instruction.h"
#include "constants.h"


eError Interpret(tHashTablePtr globalClassTable, tConstContainerPtr constContainer, tInstructionListPtr instrList);

#endif //INTERPRET_H
