//
// Created by Charvin on 6. 11. 2016.
//

#ifndef FRAME_H
#define FRAME_H

#include "ial.h"

typedef struct {
	tSymbolData Data;
	bool Initialized;
} tFrameItem, *tFrameItemPtr;

typedef struct {
	tFrameItem  *symbolArray;
	uint32_t    ReturnInstruction;
	uint32_t    CallInstruction;
	tSymbolData Return;
} tFrame;


typedef struct {
	int64_t  Top;
	uint32_t Size;
	tFrame   Poped;
	tFrame   *FrameArray;
} tFrameStack;


tFrame *frameBuild(tFrame *frame, const tSymbolPtr funcSymbol);

int32_t fstackInit(tFrameStack *stack);

void fstackDeinit(tFrameStack *stack);

tFrame *fstackPush(tFrameStack *stack, tFrame *frame);

tFrame *fstackPop(tFrameStack *stack);

inline tSymbolData *symbolGetData(tSymbolPtr symbol, tFrame *frame) {
	return (symbol->Const) ? &(symbol->Data) : &(frame->symbolArray[symbol->Index].Data);
}


#endif //FRAME_H
