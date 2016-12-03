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



#ifndef FRAME_H
#define FRAME_H

#include "ial.h"

/**
 * Structure of item representing a local variable inside a function frame
 */
typedef struct {
	eSymbolType Type;   ///type of local variable
	bool Initialized;   ///initialized flag
	tSymbolData Data;   ///Data of the variable
} tFrameItem, *tFrameItemPtr;


/**
 * Structure of function frame being created at each function call
 */
typedef struct {
	tFrameItem *symbolArray;        ///array of frame items representing a local variable
	uint32_t   ReturnInstruction;   ///Instruction index to be returned to after iRET instruction
	uint32_t   CallInstruction;     ///Instruction index of the first instruction in the function - loaded from the symboltable
	uint32_t   Size;                ///Number of frame items in the symbolArray (also size)
	eSymbolType ReturnType;
} tFrame;


/**
 * Structure representing a stack of function frames
 */
typedef struct {
	tFrame      *FrameArray;    ///Dynamic array of frames (stack)
	int64_t     Top;            ///Which frame index is on top, -1 if empty
	uint32_t    Size;           ///Size of stack in frames
	uint32_t    ArgumentIndex;  ///On which index will be pushed next argument
	tFrame      Prepared;       ///Frame being prepared for a func call
	eSymbolType ReturnType;     ///Type of return data currently holding
	tSymbolData ReturnData;     ///Return data
} tFrameStack;

/**
 * \brief Builds a function frame of corresponding function from the symbol table
 * @param frame Pointer to the frame to be constructed
 * @param funcSymbol Function symbol of the function whose frame is to be constructed
 * @return Returns pointer to the frame. On error returns NULL.
 */
tFrame *frameBuild(tFrame *frame, const tSymbolPtr funcSymbol);


/**
 * \brief Initializes stack of frames for the first use
 * @param stack Pointer to the stack to be initialized
 * @return 1 on success, 0 on error
 */
int32_t fstackInit(tFrameStack *stack);

/**
 * \brief Clears and frees all resources being used by the stack.
 * @param stack Pointer to the stack to be cleared.
 */
void fstackDeinit(tFrameStack *stack);

/**
 * \brief Copies and pushes a already built frame on the top of the stack.
 * @param stack Pointer to the stack the frame shall be pushed to.
 * @param frame Pointer to the built frame to be copied and pushed to the top of the stack.
 * @return Returns pointer to the frame newly pushed to the stack. On error returns NULL.
 */
tFrame *fstackPush(tFrameStack *stack, tFrame *frame);


/**
 * \brief Pops a frame from the stack.
 * @param stack Pointer to the stack from which a top frame shall be poped.
 * @return Returns pointer to the current top frame on the stack
 */
tFrame *fstackPop(tFrameStack *stack);


/**
 * Returns a pointer to the data of the symbol from current context.
 * @param symbol Pointer to the symbol of which data are being requested
 * @param frame Pointer to the current frame context (top frame)
 * @return Returns a pointer to the symbol data
 */
static inline tSymbolData *symbolGetData(tSymbolPtr symbol, tFrame *frame) {
	return (symbol->Const) ? &(symbol->Data) : &(frame->symbolArray[symbol->Index].Data);
}


/**
 * Returns a pointer to the data of the symbol from current context and sets its Initialized flag to true.
 * @param symbol Pointer to the symbol of which data are being requested
 * @param frame Pointer to the current frame context (top frame)
 * @return Returns a pointer to the symbol data
 */
static inline tSymbolData *symbolSetData(tSymbolPtr symbol, tFrame *frame) {
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


/**
 * Returns a pointer to the data of the symbol from current context, sets its Initialized flag to true and type of local variable in use to string to prevent memory leaks.
 * @param symbol Pointer to the symbol of which data are being requested
 * @param frame Pointer to the current frame context (top frame)
 * @return Returns a pointer to the symbol data
 */
static inline tSymbolData *symbolSetDataString(tSymbolPtr symbol, tFrame *frame) {
	if (symbol->Const)
	{
		symbol->Defined = true;
		return &(symbol->Data);
	}
	else
	{
		frame->symbolArray[symbol->Index].Initialized = true;
		frame->symbolArray[symbol->Index].Type = eSTRING;
		return &(frame->symbolArray[symbol->Index].Data);
	}
}

/**
 * \brief Checks if symbol is initialized in current context.
 * @param symbol Pointer to the symbol being examined.
 * @param frame Pointer to the current frame context (top frame)
 * @return Returns true if variable represented by the symbol is initialized in the current context. False if not.
 */
static inline bool symbolIsInitialized(tSymbolPtr symbol, tFrame *frame) {
	return symbol->Const ? symbol->Defined : frame->symbolArray[symbol->Index].Initialized;
}



#endif
