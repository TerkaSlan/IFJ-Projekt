//
// Created by Charvin on 6. 11. 2016.
//

#include "frame.h"
#include <stdlib.h>
#include <string.h>

#define STACK_DEFAULT_SIZE 128

int32_t fstackInit(tFrameStack *stack) {
	stack->Top            = -1;
	stack->Size           = 0;
	stack->Poped          = {NULL, 0, {0}};
	if((stack->FrameArray = calloc(STACK_DEFAULT_SIZE, sizeof(tFrame))))
		return 0;

	stack->Size = STACK_DEFAULT_SIZE;
	return 1;
}

void fstackDeinit(tFrameStack *stack) {

	if(stack->FrameArray) {
		for(uint32_t i = 0; i < stack->Top; i++) {
			if(stack->FrameArray[i].symbolArray)
				free(stack->FrameArray[i].symbolArray);
		}
		free(stack->FrameArray);
	}


	stack->Top  = 0;
	stack->Size = 0;
}

tFrame *fstackPush(tFrameStack *stack, tFrame *frame) {
	if(!frame || !stack)
		return NULL;

	memcpy(&(stack->FrameArray[stack->Top++]), frame, sizeof(tFrame));

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
		frame->symbolArray = calloc(funcSymbol->Data.FunctionData.LocalSymbolTable->NumberOfItems, sizeof(tFrameItem));
		if(!frame->symbolArray)
			return NULL;

		frame->Return = {0};
	} else
		abort(); //Something is really wrong.

	return frame;
}