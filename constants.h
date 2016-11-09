//
// Created by Charvin on 29. 10. 2016.
//

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "ial.h"

typedef struct {
	uint32_t   usedSize;           ///number of constants
	uint32_t   allocatedSize;      ///maximum number of constants
	tSymbolPtr *constArray;        ///pointer to the array of pointers
} tConstContainer, *tConstContainerPtr;

tConstContainerPtr constNew();

tSymbolPtr constInsertSymbol(tConstContainerPtr container, tSymbol symbol);

void constFree(tConstContainerPtr container);

#endif //CONSTANTS_H
