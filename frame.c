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



#include "frame.h"
#include "ial.h"
#include <stdlib.h>
#include <string.h>

#define STACK_DEFAULT_SIZE 128

int32_t fstackInit(tFrameStack *stack) {
	stack->Top            = -1;
	stack->Size           = 0;
	stack->ArgumentIndex  = 0;
	stack->ReturnData     = (const tSymbolData) {{0}};
	stack->ReturnType     = eNULL;
	stack->Prepared       = (const tFrame){NULL, 0, 0, 0};
	if((stack->FrameArray = calloc(STACK_DEFAULT_SIZE, sizeof(tFrame))) == NULL)
		return 0;

	stack->Size = STACK_DEFAULT_SIZE;
	return 1;
}

void fstackDeinit(tFrameStack *stack) {
	if(stack->FrameArray) {
		for(int64_t i = 0; i <= stack->Top; i++) {
			if(stack->FrameArray[i].symbolArray) {
				//check for strings in frame
				for(uint32_t j = 0; j < stack->FrameArray[i].Size; j++)
				{
					if(stack->FrameArray[i].symbolArray[j].Type == eSTRING &&
					   stack->FrameArray[i].symbolArray[j].Data.String != NULL)
						strFree(stack->FrameArray[i].symbolArray[j].Data.String);
				}
			}
			//free frame
			free(stack->FrameArray[i].symbolArray);


		}

		//if string in return val, free aswell
		if(stack->ReturnType == eSTRING && stack->ReturnData.String != NULL)
			strFree(stack->ReturnData.String);

		free(stack->FrameArray);
		stack->FrameArray = NULL;
	}


	stack->Top  = -1;
	stack->Size = 0;
}

tFrame *fstackPush(tFrameStack *stack, tFrame *frame) {
	if(!frame || !stack)
		return NULL;


	memcpy(&(stack->FrameArray[++(stack->Top)]), frame, sizeof(tFrame));


	//clear pointer to symbolarray in source frame preventing multiple frees
	frame->symbolArray = NULL;

	//if full then realloc
	if(stack->Top >= stack->Size - 1) {
		size_t newSize        = stack->Size * 2;
		tFrame *newFrameArray = realloc(stack->FrameArray, newSize);
		if(!newFrameArray)
			return NULL;

		stack->FrameArray = newFrameArray;
		stack->Size       = newSize;
	}

	return &(stack->FrameArray[stack->Top]);
}

tFrame *fstackPop(tFrameStack *stack) {
	if(!stack)
		return NULL;

	if(stack->Top >= 0) {
		if(stack->FrameArray[stack->Top].symbolArray) {

			//check for strings in frame
			for(uint32_t j = 0; j <stack->FrameArray[stack->Top].Size; j++)
			{
				if(stack->FrameArray[stack->Top].symbolArray[j].Type == eSTRING && stack->FrameArray[stack->Top].symbolArray[j].Data.String != NULL)
					strFree(stack->FrameArray[stack->Top].symbolArray[j].Data.String);
			}



			free(stack->FrameArray[stack->Top].symbolArray);
			stack->FrameArray[stack->Top].symbolArray = NULL;
		}

		return (--(stack->Top) < 0) ? NULL : &(stack->FrameArray[stack->Top]);
	}

	stack->Top = -1;
	return NULL;
}

tFrame *frameBuild(tFrame *frame, const tSymbolPtr funcSymbol) {
	if(funcSymbol->Type == eFUNCTION) {
		uint32_t  frameSize = funcSymbol->Data.FunctionData.LocalSymbolTable->NumberOfItems;
		frame->symbolArray = calloc(frameSize, sizeof(tFrameItem));
		if(!frame->symbolArray)
			return NULL;

		frame->Size = frameSize;
		frame->ReturnInstruction = 0;
	} else
		abort(); //Something is really wrong.

	return frame;
}
