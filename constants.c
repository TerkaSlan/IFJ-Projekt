/*
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


#include <stdlib.h>
#include <string.h>
#include "constants.h"


#define CONST_CONTAINER_DEFAULT_SIZE 128

tConstContainerPtr constNew() {
	tConstContainerPtr ret = malloc(sizeof(tConstContainer));
	if(!ret)
		return NULL;

	//allocate array of instructions
	ret->constArray = calloc(CONST_CONTAINER_DEFAULT_SIZE, sizeof(tSymbolPtr));
	if(!ret->constArray) {
		free(ret);
		return NULL;
	}

	//set to default values
	ret->usedSize      = 0;
	ret->allocatedSize = CONST_CONTAINER_DEFAULT_SIZE;

	return ret;
}

tSymbolPtr constInsertSymbol(tConstContainerPtr container, tSymbol symbol) {
	if(!container || !container->constArray)
		return NULL;

	//add instruction
	tSymbolPtr newSymbol = symbolNewCopy(&symbol);
	if(!newSymbol)
		return NULL;

	//implicitly this is const. value - its real value is stored inside the symbol. Not in local frame.
	newSymbol->Const = true;

	container->constArray[container->usedSize] = newSymbol;


	//check if still in the array - if not, realloc
	if(++(container->usedSize) >= container->allocatedSize) {
		uint32_t newAllocSize = container->allocatedSize * 2;
		void *newConstArray = realloc(container->constArray, sizeof(tSymbolPtr) * newAllocSize);
		if(!newConstArray)
			return NULL;

		container->constArray    = newConstArray;
		container->allocatedSize = newAllocSize;
	}

	return newSymbol;
}

void constFree(tConstContainerPtr container) {
	if(!container)
		return;

	if(container->constArray) {
		for(uint32_t i = 0; i < container->usedSize; i++) {
			symbolFree(container->constArray[i]);
		}

		free(container->constArray);
	}

	free(container);
}
