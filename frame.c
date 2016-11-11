//
// Created by Charvin on 6. 11. 2016.
//

#include "frame.h"
#include "ial.h"
#include <stdlib.h>
#include <string.h>

#define STACK_DEFAULT_SIZE 128

int32_t fstackInit(tFrameStack *stack) {
	stack->Top            = -1;
	stack->Size           = 0;
	stack->ArgumentIndex  = 0;
	stack->ReturnData     = {0};
	stack->ReturnType     = eNULL;
	stack->Prepared       = {NULL, 0, 0};
	if((stack->FrameArray = calloc(STACK_DEFAULT_SIZE, sizeof(tFrame))))
		return 0;

	stack->Size = STACK_DEFAULT_SIZE;
	return 1;
}

void fstackDeinit(tFrameStack *stack) {
	if(stack->FrameArray) {
		for(uint32_t i = 0; i < stack->Top; i++) {
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
	}


	stack->Top  = 0;
	stack->Size = 0;
}

tFrame *fstackPush(tFrameStack *stack, tFrame *frame) {
	if(!frame || !stack)
		return NULL;

	memcpy(&(stack->FrameArray[stack->Top++]), frame, sizeof(tFrame));
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

	if(stack->Top > 0) {
		if(stack->FrameArray[stack->Top].symbolArray) {

			//check for strings in frame
			for(uint32_t j = 0; j <stack->FrameArray[stack->Top].Size; j++) ///TODO::shit we need to fill dis
			{
				if(stack->FrameArray[stack->Top].symbolArray[j].Type == eSTRING && stack->FrameArray[stack->Top].symbolArray[j].Data.String != NULL)
					strFree(stack->FrameArray[stack->Top].symbolArray[j].Data.String);
			}

			free(stack->FrameArray[stack->Top].symbolArray);
			stack->FrameArray[stack->Top].symbolArray = NULL;
		}

		return &(stack->FrameArray[--(stack->Top)]);
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