//
// Created by Charvin on 16. 10. 2016.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H


#include <stdint.h>

/*
 * Typedef enum of instruction types.
 * Arguments have structure arg1, arg2, dst (destination)
 */
typedef enum {
	iSTOP = 0,  ///End of program ///TODO::maybe magic with return, who knows
	iMOV,       ///Move                 dst = arg1
	iINC,       ///Increment            dst = arg1 + 1
	iDEC,       ///Decrement            dst = arg1 - 1
	iADD,       ///Add                  dst = arg1 + arg2
	iSUB,       ///Subtract             dst = arg1 - arg2
	iMUL,       ///Multiply             dst = arg1 * arg2
	iDIV,       ///Divide               dst = arg1 / arg2
	iMOD,       ///Modulo               dst = arg1 % arg2
	iNEG,       ///Negate               dst = -arg1
	iNOT,       ///Bit inverse          dst = ~arg1
	iAND,       ///Bit And              dst = arg1 & arg2
	iOR,        ///Bit Or               dst = arg1 | arg2
	iXOR,       ///Bit Xor              dst = arg1 ^ arg2
	iLEQ,       ///Logical Equality     dst = arg1 == arg2
	iLNEQ,      ///Logical Inequality   dst = arg1 != arg2
	iLAND,      ///Logical And          dst = arg1 && arg2
	iLOR,       ///Logical Or           dst = arg1 || arg2
	iLNOT,      ///Logical Not          dst = !arg1
	iGOTO,      ///Goto                 goto dst
	iCALL,      ///Call                 call dst
	iRET,       ///Return
	iIFGOTO,    ///if(arg1) goto dst
	iIFNGOTO,   ///if(!arg1) goto dst
	iLABEL      ///Label for gotos
} eInstructionType;

/*
 * Typedef struct of Instruction
 */
typedef struct {
	eInstructionType type;
	void             *arg1;
	void             *arg2;
	void             *dst;
} tInstruction, *tInstructionPtr;

/*
 * Typedef struct of Instruction List
 */
typedef struct {
	int64_t         firstInstruction;   ///pointer to the entry point instruction
	int64_t         activeInstruction;  ///pointer to the instruction currently being processed
	uint32_t        usedSize;           ///number of instructions currently in use
	uint32_t        allocatedSize;      ///maximum number of instructions
	tInstructionPtr instructionArray;   ///pointer to the array of instructions
} tInstructionList, *tInstructionListPtr;

/**
 *  \brief Allocates and initilizes new instruction list
 *  
 *  \return tInstructionListPtr, Returns pointer to the new instruction list, if an error occures, NULL is returned
 *  
 *  \details Allocates tInstructionList structure and internal structures. Sets all instructionArray to NULL aswell as first instruction and allocated instruction.
 */
tInstructionListPtr instrListNew();

/**
 *  \brief Gets pointer to the next instruction in the instruction list.
 *  
 *  \param [in] list tInstructionListPtr, Pointer to the instruction list to be read from
 *  \return tInstructionPtr, Returns pointer to the next instruction in the instruction list. On error NULL is returned.
 *
 */
tInstructionPtr instrListGetNextInstruction(tInstructionListPtr list);

/**
 *  \brief Gets pointer to the active instruction in the instruction list.
 *
 *  \param [in] list tInstructionListPtr, Pointer to the instruction list to be read from
 *  \return tInstructionPtr, Returns pointer to the instruction being processed in the instruction list. On error NULL is returned.
 *
 */
tInstructionPtr instrListGetActiveInstruction(tInstructionListPtr list);

/**
 *  \brief Inserts instruction into the instruction list (at the end)
 *  
 *  \param [in] list tInstructionListPtr, Pointer to the instruction list to be written to
 *  \param [in] instr tInstruction, instruction to be inserted
 *  \return tInstructionPtr, Returns pointer to the instruction newly copied in the instruction list. On error NULL is returned.
 *  
 *  \details Function copies instruction passed as a value into the instruction list after the last inserted instruction.
 */
tInstructionPtr instrListInsertInstruction(tInstructionListPtr list, tInstruction instr);

/**
 *  \brief Makes the next instruction in the instruction list the instruction specified.
 *  
 *  \param [in] list tInstructionListPtr, Pointer to the instruction list
 *  \param [in] instructionIndex uint32_t, index of the instruction to be jumped to
 *  \return void
 */
void instrListGoto(tInstructionListPtr list, uint32_t instructionIndex);

/**
 *  \brief Deallocates instruction list including its inner structures.
 *  
 *  \param [in] list tInstructionListPtr, Pointer to the instruction list to be deallocated
 *  \return void
 *
 */
void instrListFree(tInstructionListPtr list);


#endif //INSTRUCTION_H
