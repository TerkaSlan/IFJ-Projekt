/**
  *  \file constants.h
  *  \brief Contains declarations structures and functions regarding the handling of constants
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "ial.h"

typedef struct {
	uint32_t   usedSize;           ///number of constants
	uint32_t   allocatedSize;      ///maximum number of constants
	tSymbolPtr *constArray;        ///pointer to the array of pointers
} tConstContainer, *tConstContainerPtr;


/**
 * \brief Creates a new constant container
 * @return Pointer to a new constant container. On error returns NULL
 */
tConstContainerPtr constNew();


/**
 * \brief Inserts symbol into the constant container
 * @param container Pointer to the container
 * @param symbol Symbol to be copied and stored in the container. NOTE: makes a deep copy of Name (should be NULL tho) and does NOT make a deep copy of Data structure of the symbol.
 * @return Pointer to a copied symbol stored in the container.
 */
tSymbolPtr constInsertSymbol(tConstContainerPtr container, tSymbol symbol);

/**
 * \brief Clears container and frees all the symbols, including itself.
 * @param container Container to be freed.
 */
void constFree(tConstContainerPtr container);

#endif //CONSTANTS_H
