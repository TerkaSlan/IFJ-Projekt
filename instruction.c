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
#include <stdio.h>
#include <inttypes.h>
#include "instruction.h"
#include "ial.h"

// Colors
#define KNRM  "\x1B[0m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"



#define INSTRUCTION_LIST_DEFAULT_SIZE 4096


tInstructionListPtr instrListNew() {
	tInstructionListPtr ret = malloc(sizeof(tInstructionList));
	if(!ret)
		return NULL;

	//allocate array of instructions
	ret->instructionArray = calloc(INSTRUCTION_LIST_DEFAULT_SIZE, sizeof(tInstruction));
	if(!ret->instructionArray) {
		free(ret);
		return NULL;
	}

	//set to default values
	ret->activeInstruction = -1;
	ret->firstInstruction  = -1;
	ret->usedSize          = 0;
	ret->allocatedSize     = INSTRUCTION_LIST_DEFAULT_SIZE;

	//add iStop instruction on index 0
	tInstruction stop = {iSTOP, NULL, NULL, NULL};
	if(instrListInsertInstruction(ret, stop) != 0) {
		free(ret->instructionArray);
		free(ret);
		return NULL;
	}

	return ret;

}

tInstructionPtr instrListGetNextInstruction(tInstructionListPtr list) {
	if(!list || list->firstInstruction <
	            0) //not checking for array != null, interpreting a corrupted instruction list should not happen
		return NULL;

	//increases active instruction index and returns pointer to the instruction
	return &(list->instructionArray[++(list->activeInstruction)]);
}


tInstructionPtr instrListGetActiveInstruction(tInstructionListPtr list) {
	if(!list || list->activeInstruction < 0 || !list->instructionArray)
		return NULL;

	return &(list->instructionArray[list->activeInstruction]);
}




int64_t instrListGetActiveInstructionIndex(tInstructionListPtr list)
{
	if(!list)
		return -2;
	return list->activeInstruction;
}


int64_t instrListInsertInstruction(tInstructionListPtr list, tInstruction instr) {
	if(!list || !list->instructionArray)
		return -1;

	//add instruction
	int64_t index = list->usedSize;

	memcpy(&(list->instructionArray[index]), &instr, sizeof(tInstruction));


	//check if still in the array - if not, realloc
	if(++(list->usedSize) >= list->allocatedSize) {
		uint32_t        newAllocSize        = list->allocatedSize * 2;
		tInstructionPtr newInstructionArray = realloc(list->instructionArray, sizeof(tInstruction) * newAllocSize);
		if(!newInstructionArray)
			return -1;

		list->instructionArray = newInstructionArray;
		list->allocatedSize    = newAllocSize;
	}

	return index;
}

void instrListSetFirstInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->allocatedSize)
		return;

	list->firstInstruction  = (int64_t) index;
	list->activeInstruction = ((int64_t) index) - 1;

}


tInstructionPtr instrListGetInstruction(tInstructionListPtr list, uint32_t index) {
	if(!list || index >= list->usedSize || !list->instructionArray)
		return NULL;

	return &(list->instructionArray[index]);
}


void instrListGoto(tInstructionListPtr list, uint32_t instructionIndex) {
	if(!list)
		return;

	list->activeInstruction = (int64_t) instructionIndex - 1;
}

void instrListFree(tInstructionListPtr list) {
	free(list->instructionArray);
	free(list);

}

void instrListPrint(tInstructionListPtr list) {
	if (!list) {
		printf("Pointer to instruction list is NULL\n");
		return;
	}
	static const char *types[] = {"iSTOP", "iMOV", "iFRAME", "iPUSH", "iCALL", "iRET", "iGETRETVAL", "iINC", "iDEC", "iADD", "iSUB", "iMUL", "iDIV", "iNEG", "iLE", "iLT", "iGE", "iGT", "iEQ", "iNEQ", "iLAND", "iLOR", "iLNOT", "iGOTO", "iIFGOTO", "iIFNGOTO", "iCONV2STR", "iCONV2INT", "iCONV2BOOL", "iCONV2DOUBLE", "iPRINT", "iREAD", "iLEN", "iCOMPARE", "iFIND", "iSORT", "iSUBSTR", "Unknown instr"};
	uint32_t index = 0;

	const char ESC = 27;
	while (index < list->usedSize) {
		printf("%4d.: ", index);
		printf("%s%c[1m%13s%c[0m%s ", KYEL, ESC, types[list->instructionArray[index].type], ESC, KNRM);

		printf("  |   %sDst ", KMAG);
		if(list->instructionArray[index].type == iGOTO || list->instructionArray[index].type == iIFGOTO || list->instructionArray[index].type == iIFNGOTO)
			printf("idx: %lu", (unsigned long)list->instructionArray[index].dst);
		else {
			if(list->instructionArray[index].dst != NULL) {
				if(((tSymbolPtr) list->instructionArray[index].dst)->Name != NULL &&
				   ((tSymbolPtr) list->instructionArray[index].dst)->Name->str != NULL) {
					printf("Name: %s ", ((tSymbolPtr) list->instructionArray[index].dst)->Name->str);
				} else {
					if(&(((tSymbolPtr) list->instructionArray[index].dst)->Data) != NULL ||
					   ((tSymbolPtr) list->instructionArray[index].dst)->Type == eNULL) {
						switch(((tSymbolPtr) list->instructionArray[index].dst)->Type) {
							case eNULL:
								printf("eNULL ");
								break;
							case eINT:
								printf("eINT: ""%" SCNd32 " ",
								       (int32_t) ((tSymbolPtr) list->instructionArray[index].dst)->Data.Integer);
								break;
							case eDOUBLE:
								printf("eDOUBLE: %lf ", ((tSymbolPtr) list->instructionArray[index].dst)->Data.Double);
								break;
							case eBOOL:
								printf("eBOOL: %s ",
								       ((tSymbolPtr) list->instructionArray[index].dst)->Data.Bool ? "true" : "false");
								break;
							case eSTRING:
								if(((tSymbolPtr) list->instructionArray[index].dst)->Data.String != NULL) {
									printf("eSTRING: %s ",
									       ((tSymbolPtr) list->instructionArray[index].dst)->Data.String->str);
								} else {
									printf("eSTRING: NULL ");
								}
								break;
							default:
								printf("*UnknownType* ");
						}
					} else {
						printf("Data = NULL ");
					}
				}
			} else {
				printf("= NULL ");
			}
		}

		printf("%s  |  %s Arg1 ", KNRM, KGRN);
		if (list->instructionArray[index].arg1 != NULL) {
			if (((tSymbolPtr)list->instructionArray[index].arg1)->Name != NULL && ((tSymbolPtr)list->instructionArray[index].arg1)->Name->str != NULL) {
				printf("Name: %s ", ((tSymbolPtr)list->instructionArray[index].arg1)->Name->str);
			} else {
				if ( &(((tSymbolPtr)list->instructionArray[index].arg1)->Data) != NULL || ((tSymbolPtr)list->instructionArray[index].arg1)->Type == eNULL) {
					switch (((tSymbolPtr)list->instructionArray[index].arg1)->Type) {
						case eNULL:
							printf("eNULL ");
							break;
						case eINT:
							printf( "eINT: ""%" SCNd32 " ", (int32_t)((tSymbolPtr)list->instructionArray[index].arg1)->Data.Integer );
							break;
						case eDOUBLE:
							printf( "eDOUBLE: %lf ", ((tSymbolPtr)list->instructionArray[index].arg1)->Data.Double );
							break;
						case eBOOL:
							printf("eBOOL: %s ", ((tSymbolPtr)list->instructionArray[index].arg1)->Data.Bool ? "true" : "false");
							break;
						case eSTRING:
							if (((tSymbolPtr)list->instructionArray[index].arg1)->Data.String != NULL) {
								printf("eSTRING: %s ", ((tSymbolPtr)list->instructionArray[index].arg1)->Data.String->str );
							} else {
								printf("eSTRING: NULL ");
							}
							break;
						default:
							printf("*UnknownType* ");
					}
				} else {
					printf("Data = NULL ");
				}
			}
		} else {
			printf("= NULL ");
		}

		printf("%s  |  %s Arg2 ", KNRM, KCYN);
		if (list->instructionArray[index].arg2 != NULL) {
			if (((tSymbolPtr)list->instructionArray[index].arg2)->Name != NULL && ((tSymbolPtr)list->instructionArray[index].arg2)->Name->str != NULL) {
				printf("Name: %s ", ((tSymbolPtr)list->instructionArray[index].arg2)->Name->str);
			} else {
				if ( &(((tSymbolPtr)list->instructionArray[index].arg2)->Data) != NULL || ((tSymbolPtr)list->instructionArray[index].arg2)->Type == eNULL) {
					switch (((tSymbolPtr)list->instructionArray[index].arg2)->Type) {
						case eNULL:
							printf("eNULL ");
							break;
						case eINT:
							printf( "eINT: ""%" SCNd32 " ", (int32_t)((tSymbolPtr)list->instructionArray[index].arg2)->Data.Integer );
							break;
						case eDOUBLE:
							printf( "eDOUBLE: %lf ", ((tSymbolPtr)list->instructionArray[index].arg2)->Data.Double );
							break;
						case eBOOL:
							printf("eBOOL: %s ", ((tSymbolPtr)list->instructionArray[index].arg2)->Data.Bool ? "true" : "false");
							break;
						case eSTRING:
							if (((tSymbolPtr)list->instructionArray[index].arg2)->Data.String != NULL) {
								printf("eSTRING: %s ", ((tSymbolPtr)list->instructionArray[index].arg2)->Data.String->str );
							} else {
								printf("eSTRING: NULL ");
							}
							break;
						default:
							printf("*UnknownType* ");
					}
				} else {
					printf("Data = NULL ");
				}
			}
		} else {
			printf("= NULL ");
		}
		printf("%s\n", KNRM);
		index++;
	}
}
