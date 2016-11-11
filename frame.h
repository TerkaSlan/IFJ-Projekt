//
// Created by Charvin on 6. 11. 2016.
//

#ifndef FRAME_H
#define FRAME_H

#include "ial.h"

typedef struct {
	eSymbolType Type;
	bool Initialized;
	tSymbolData Data;
} tFrameItem, *tFrameItemPtr;

typedef struct {
	tFrameItem *symbolArray;
	uint32_t   ReturnInstruction;
	uint32_t   CallInstruction;
	uint32_t   Size;
} tFrame;


typedef struct {
	tFrame      *FrameArray;   //Dynamic array of frames (stack)
	int64_t     Top;           //Which frame index is on top, -1 if empty
	uint32_t    Size;            //Size of stack in frames
	uint32_t    ArgumentIndex; //On which index will be pushed next argument
	tFrame      Prepared;        //Frame being prepared for a func call
	eSymbolType ReturnType; ///Type of return data currently holding
	tSymbolData ReturnData; //Return data
} tFrameStack;


tFrame *frameBuild(tFrame *frame, const tSymbolPtr funcSymbol);

int32_t fstackInit(tFrameStack *stack);

void fstackDeinit(tFrameStack *stack);

tFrame *fstackPush(tFrameStack *stack, tFrame *frame);

tFrame *fstackPop(tFrameStack *stack);



inline tSymbolData *symbolGetData(tSymbolPtr symbol, tFrame *frame) {
	return (symbol->Const) ? &(symbol->Data) : &(frame->symbolArray[symbol->Index].Data);
}

inline tSymbolData *symbolSetData(tSymbolPtr symbol, tFrame *frame) {
	if (symbol->Const)
	{
		symbol->Defined = true;
		return &(symbol->Data);
	}
	else
	{
		frame->symbolArray[symbol->Index].Initialized = true;
		return &(frame->symbolArray[symbol->Index].Data);
	}
}

inline bool symbolIsInitialized(tSymbolPtr symbol, tFrame *frame) {
	return symbol->Const ? symbol->Defined : frame->symbolArray[symbol->Index].Initialized;
}



#endif