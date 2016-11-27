#ifndef PARSER2_H
#define PARSER2_H

#include "error.h"

/**
 *  \brief Entry point of second parsing, prepared source file and initializes global variables
 *  \return ERR_OK on success, any other eError on failure
 */
eError generateInstructions();

#endif
