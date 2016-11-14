//
// Created by Charvin on 29. 10. 2016.
//

#include "interpret.h"
#include "frame.h"
#include "instruction.h"
#include "ial.h"
#include <assert.h>
#include <stdlib.h>


//use assert for debugging, for speed without checks use compile flag NDEBUG
#ifndef NDEBUG
#define ASSERT(c) assert(c)
#else
#define ASSERT(c) ;
#endif


eError Interpret(tHashTablePtr globalClassTable, tInstructionListPtr instrList) {
	//lets start this shit
#define EXIT(err, ...) do{ret = err; printError(err, __VA_ARGS__); goto lbFinish;}while(0)
	eError ret = ERR_OK;


	//create frame stack
	tFrameStack frames;
	fstackInit(&frames);

	///TODO::loop through classes and functions and build indexes or maybe parser do it after finishing definition?

	//do we have main?
	dtStrPtr   string = strNewFromCStr("Main");
	tSymbolPtr mainSymbol;
	if((mainSymbol = htabGetSymbol(globalClassTable, string)) == NULL || mainSymbol->Type != eCLASS ||
	   !mainSymbol->Defined)
		EXIT(ERR_SEM, "Class Main does not exist.\n"); //3?

	//do we have function run?
	strClear(string);
	strAddCStr(string, "Run");
	if((mainSymbol = htabGetSymbol(mainSymbol->Data.ClassData.LocalSymbolTable, string)) == NULL ||
	   mainSymbol->Type != eFUNCTION || !mainSymbol->Defined)
		EXIT(ERR_SEM, "In class Main there is no Run method.\n"); //3?

	//current frame
	tFrame *curFrame;

	//create local main frame
	frameBuild(&frames.Prepared, mainSymbol);
	if((curFrame = fstackPush(&frames, &frames.Prepared)) == NULL)
		EXIT(ERR_INTERN, "Cannot create frame stack. Out of memory.\n");

	//set this as first instruction
	instrListSetFirstInstruction(instrList, mainSymbol->Data.FunctionData.InstructionIndex);
	ASSERT(mainSymbol->Data.FunctionData.InstructionIndex > 0);

	//get instruction and do shit, loop
	tInstructionPtr i = NULL;
	while(1) {
		if(!(i = instrListGetNextInstruction(instrList)))
			EXIT(ERR_OTHER, "Error getting next instruction.\n");

#define GD symbolGetData        //Gets pointer to tSymbolData where data of the symbol are stored
#define SD symbolSetData        //Gets pointer to tSybmolData where data of the symbol are stored and sets it as initialized
#define SDS SD                  //TODO::cant remember why i needed to add it, for strings specifically, if I don't find out, I will delete this
#define CHECK_INIT(symbol, frame) do{if(!symbolIsInitialized((symbol), (frame))) EXIT(ERR_RUN_UNINITIALIZED, "Accessing uninitialized variable %s.\n", strGetCStr(((tSymbolPtr)(symbol))->Name)); }while(0) //checks if variable is initialized


		//INTERPRET!
		switch(i->type) {

			case iMOV: {
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);

				CHECK_INIT(i->arg1, curFrame);

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double;
						break;

					case eBOOL:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool;
						break;

					case eSTRING: {
						dtStrPtr tmpDst = SDS(i->dst, curFrame)->String;
						dtStrPtr tmpSrc = GD(i->arg1, curFrame)->String;
						if(tmpDst) {
							strClear(tmpDst);
							if(strCopyStr(tmpDst, tmpSrc) == STR_ERROR)
								EXIT(ERR_INTERN, "Cannot copy string. Out of memory.\n");
						} else {
							if((tmpDst = strNewFromStr(tmpSrc)) == NULL)
								EXIT(ERR_INTERN, "Cannot copy string. Out of memory.\n");
						}
						break;
					}
					default:
						EXIT(ERR_OTHER,
						     "Trying to assign class or func.\n");
				}
				break;
			}


			case iFRAME: {
				//check if function being called is ok
				tSymbolPtr func = (tSymbolPtr)(i->dst);
				if(!func->Defined || func->Type != eFUNCTION)
					EXIT(ERR_RUN_UNINITIALIZED, "Calling undefined function %s.\n", strGetCStr(func->Name));

				//create new prepared frame
				if(frameBuild(&(frames.Prepared), func) == NULL)
					EXIT(ERR_INTERN, "Cant create new function frame. Out of memory.\n");

				ASSERT(func->Data.FunctionData.InstructionIndex != 0);
				frames.Prepared.CallInstruction = func->Data.FunctionData.InstructionIndex;

				//reset push index counter counter
				frames.ArgumentIndex = 0;
				break;
			}

			case iPUSH:{
				CHECK_INIT(i->arg1, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						frames.Prepared.symbolArray[frames.ArgumentIndex].Data.Integer = GD(i->arg1, curFrame)->Integer;
						frames.Prepared.symbolArray[frames.ArgumentIndex++].Initialized = true;
						break;

					case eDOUBLE:
						frames.Prepared.symbolArray[frames.ArgumentIndex].Data.Double = GD(i->arg1, curFrame)->Double;
						frames.Prepared.symbolArray[frames.ArgumentIndex++].Initialized = true;
						break;

					case eBOOL:
						frames.Prepared.symbolArray[frames.ArgumentIndex].Data.Bool = GD(i->arg1, curFrame)->Bool;
						frames.Prepared.symbolArray[frames.ArgumentIndex++].Initialized = true;
						break;

					case eSTRING: {
						tFrameItemPtr tmpDst = &(frames.Prepared.symbolArray[frames.ArgumentIndex++]);
						dtStrPtr tmpSrc = GD(i->arg1, curFrame)->String;

						if((tmpDst->Data.String = strNewFromStr(tmpSrc)) == NULL)
							EXIT(ERR_INTERN, "Cannot copy string. Out of memory.\n");

						tmpDst->Initialized = true;
						tmpDst->Type = eSTRING;
						break;
					}
					default:
						EXIT(ERR_OTHER,
						     "Trying to pass class or func into the function.\n");
				}
				break;
			}
			case iCALL:
				//push frame
				if((curFrame = fstackPush(&frames, &(frames.Prepared))) == NULL)
					EXIT(ERR_INTERN, "Cant create new function frame. Out of memory.\n");

				//set next instruction
				instrListGoto(instrList, curFrame->CallInstruction);
				break;

			case iRET:{
				eSymbolType type = ((tSymbolPtr) i->arg1)->Type;

				//if replacing string, free first
				if(frames.ReturnData.String && frames.ReturnData.String != NULL)
				{
					strFree(frames.ReturnData.String);
					frames.ReturnData.String = NULL;
				}

				CHECK_INIT(i->arg1, curFrame);
				switch(type) {
					case eINT:
						frames.ReturnData.Integer = GD(i->arg1, curFrame)->Integer;
						break;

					case eDOUBLE:
						frames.ReturnData.Double = GD(i->arg1, curFrame)->Double;
						break;

					case eBOOL:
						frames.ReturnData.Bool = GD(i->arg1, curFrame)->Bool;
						break;

					case eSTRING: {
						tSymbolData *tmpStr = GD(i->arg1, curFrame);

						frames.ReturnData.String = tmpStr->String;
						tmpStr->String = NULL; //prevent deleting on frame destruction
						break;
					}
					default:
						EXIT(ERR_OTHER, "Trying to return class or func.\n");
				}

				//set return type
				frames.ReturnType = type;


				instrListGoto(instrList, curFrame->ReturnInstruction);
				curFrame = fstackPop(&frames);

				break;
			}

			case iGETRETVAL:{
				eSymbolType type = ((tSymbolPtr) i->dst)->Type;

				ASSERT(type == frames.ReturnType);

				switch(type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = frames.ReturnData.Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = frames.ReturnData.Double;
						break;

					case eBOOL:
						SD(i->dst, curFrame)->Bool = frames.ReturnData.Bool;
						break;

					case eSTRING: {
						SDS(i->dst, curFrame)->String = frames.ReturnData.String;
						frames.ReturnData.String = NULL; //prevent multiple frees
						break;
					}
					default:
						EXIT(ERR_OTHER,
						     "Trying to assign class or func.\n");
				}
				break;
			}

			case iINC:{
				CHECK_INIT(i->dst, curFrame);

				tSymbolPtr tmp = (tSymbolPtr) (i->dst);
				if(tmp->Type == eINT)
					GD(tmp, curFrame)->Integer++;
				else
					EXIT(ERR_OTHER,"Trying to increment non int.\n");
			}

			case iDEC:{
				CHECK_INIT(i->dst, curFrame);

				tSymbolPtr tmp = (tSymbolPtr) (i->dst);
				if(tmp->Type == eINT)
					GD(tmp, curFrame)->Integer--;
				else
					EXIT(ERR_OTHER,"Trying to decrement non int.\n");
			}

			case iADD:{
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer + GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double + GD(i->arg2, curFrame)->Double;
						break;

					case eSTRING: {
						dtStrPtr tmpDst = SDS(i->dst, curFrame)->String;
						dtStrPtr tmpSrc1 = GD(i->arg1, curFrame)->String;
						dtStrPtr tmpSrc2 = GD(i->arg2, curFrame)->String;

						dtStrPtr newString;
						if((newString = strNewFromStr(tmpSrc1)) == NULL)
							EXIT(ERR_INTERN, "Cannot copy string. Out of memory.\n");

						if(strAddCStr(newString, strGetCStr(tmpSrc2)) != STR_SUCCESS)
							EXIT(ERR_INTERN, "Cannot copy string. Out of memory.\n");

						if(tmpDst)
							strFree(tmpDst);

						tmpDst = newString;

						break;
					}
					default:
						EXIT(ERR_OTHER, "Operands %s and %s doesnt support addition.\n", strGetCStr(((tSymbolPtr)(i->arg1))->Name),strGetCStr(((tSymbolPtr)(i->arg2))->Name) );
				}
				break;
			}
			case iSUB:{
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer - GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double - GD(i->arg2, curFrame)->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operands %s and %s doesnt support subtraction.\n", strGetCStr(((tSymbolPtr)(i->arg1))->Name),strGetCStr(((tSymbolPtr)(i->arg2))->Name) );
				}
				break;

			}
			case iMUL:{
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer * GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double * GD(i->arg2, curFrame)->Double;
						break;


					default:
						EXIT(ERR_OTHER, "Operands %s and %s doesnt support multiplication.\n", strGetCStr(((tSymbolPtr)(i->arg1))->Name),strGetCStr(((tSymbolPtr)(i->arg2))->Name) );
				}
				break;


			}
			case iDIV:{
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				tSymbolData *divisor = GD(i->arg2, curFrame);
				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						if(divisor->Integer == 0)
							EXIT(ERR_RUN_ZERODIV, "Trying to divide by 0. Operand %s\n", strGetCStr(((tSymbolPtr)(i->arg2))->Name));

						SD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer / divisor->Integer;
						break;

					case eDOUBLE:
						if(divisor->Double == 0.0) //TODO:: use different comp method for double?
							EXIT(ERR_RUN_ZERODIV, "Trying to divide by 0. Operand %s\n", strGetCStr(((tSymbolPtr)(i->arg2))->Name));

						SD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double / divisor->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operands %s and %s doesnt support division.\n", strGetCStr(((tSymbolPtr)(i->arg1))->Name),strGetCStr(((tSymbolPtr)(i->arg2))->Name) );
				}
				break;

			}
			case iNEG:{
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);

				CHECK_INIT(i->arg1, curFrame);

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Integer = -(GD(i->arg1, curFrame)->Integer);
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Double = -(GD(i->arg1, curFrame)->Double);
						break;

					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support arithmetic negation.\n", strGetCStr(((tSymbolPtr)(i->arg1))->Name));
				}
				break;
			}

			case iLE: {
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer <= GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double <= GD(i->arg2, curFrame)->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
						break;
				}
				break;
			}
			case iLT: {
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);


				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer < GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double < GD(i->arg2, curFrame)->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
						break;
				}
				break;
			}

			case iGE: {
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer >= GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double >= GD(i->arg2, curFrame)->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
						break;
				}
				break;
			}
			case iGT: {
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer > GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double > GD(i->arg2, curFrame)->Double;
						break;

					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
						break;
				}
				break;
			}
			case iEQ:{
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer == GD(i->arg2, curFrame)->Integer;
						break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double == GD(i->arg2, curFrame)->Double;
						break;

					case eBOOL:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool == GD(i->arg2, curFrame)->Bool;
						break;

					case eSTRING: {
						int32_t result =  strCmpStr(GD(i->arg1, curFrame)->String, GD(i->arg2, curFrame)->String);

						SDS(i->dst, curFrame)->Bool = (result == 0);
						break;
					}
					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
						break;
				}
				break;
			}

			case iNEQ:{
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				switch(((tSymbolPtr) i->arg1)->Type) {
					case eINT:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Integer != GD(i->arg2, curFrame)->Integer;
					break;

					case eDOUBLE:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Double != GD(i->arg2, curFrame)->Double;
					break;

					case eBOOL:
						SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool != GD(i->arg2, curFrame)->Bool;
					break;

					case eSTRING: {
						int32_t result =  strCmpStr(GD(i->arg1, curFrame)->String, GD(i->arg2, curFrame)->String);
						if(result == STR_ERROR)
							EXIT(ERR_INTERN, "Error occured during string comparasion.\n");

						SDS(i->dst, curFrame)->Bool = (result != 0);
						break;
					}
					default:
						EXIT(ERR_OTHER, "Operand %s doesnt support logic comparision.\n",
						     strGetCStr(((tSymbolPtr) (i->arg1))->Name));
					break;
				}
				break;

			}

			case iLAND:
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool && GD(i->arg2, curFrame)->Bool;
				break;
			case iLOR:
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == ((tSymbolPtr) i->arg2)->Type);
				ASSERT(((tSymbolPtr) i->dst)->Type == eBOOL);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				SD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool || GD(i->arg2, curFrame)->Bool;
				break;
			case iLNOT:
				ASSERT(((tSymbolPtr) i->dst)->Type == ((tSymbolPtr) i->arg1)->Type);
				ASSERT(((tSymbolPtr) i->arg1)->Type == eBOOL);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				SD(i->dst, curFrame)->Bool = !(GD(i->arg1, curFrame)->Bool);
				break;
			case iGOTO:
				ASSERT((uint32_t)(i->dst) != NULL);

				instrListGoto(instrList, (uint32_t)(i->dst));
				break;
			case iIFGOTO:
				ASSERT((uint32_t)(i->dst) != NULL);
				ASSERT((uint32_t)(i->arg1) != NULL);
				ASSERT(((tSymbolPtr)i->arg1)->Type == eBOOL);

				CHECK_INIT(i->arg1, curFrame);

				if(GD(i->arg1, curFrame)->Bool)
					instrListGoto(instrList, (uint32_t)(i->dst));
				break;
			case iIFNGOTO:
				ASSERT((uint32_t)(i->dst) != NULL);
				ASSERT((uint32_t)(i->arg1) != NULL);
				ASSERT(((tSymbolPtr)i->arg1)->Type == eBOOL);

				CHECK_INIT(i->arg1, curFrame);

				if(!(GD(i->arg1, curFrame)->Bool))
					instrListGoto(instrList, (uint32_t)(i->dst));
				break;

			case iCONV2STR: {
				dtStrPtr tmp = GD(i->dst, curFrame)->String; //TODO:REDO!
				if(((tSymbolPtr)i->dst)->Type == eSTRING && tmp);
				{
					strFree(tmp);
					tmp = NULL;
				}
					///TODO::tmp = call(SD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			}
			case iCONV2INT:
				///TODO::SD(i->dst, curFrame)->Integer = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iCONV2BOOL:
				///TODO::SD(i->dst, curFrame)->Boolean = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iCONV2DOUBLE:
				///TODO::SD(i->dst, curFrame)->Double = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iPRINT:
				//call
				break;
			case iREAD:
				//call
				break;
			case iLEN:
				ASSERT(((tSymbolPtr) i->dst)->Type == eINT);
				ASSERT(((tSymbolPtr) i->arg1)->Type == eSTRING);

				CHECK_INIT(i->arg1, curFrame);

				SD(i->dst, curFrame)->Integer = strGetLength(GD(i->arg1, curFrame)->String);
				break;
			case iCOMPARE:
				ASSERT(((tSymbolPtr) i->dst)->Type == eINT);
				ASSERT(((tSymbolPtr) i->arg1)->Type == eSTRING);
				ASSERT(((tSymbolPtr) i->arg2)->Type == eSTRING);

				CHECK_INIT(i->arg1, curFrame);
				CHECK_INIT(i->arg2, curFrame);

				SD(i->dst, curFrame)->Integer = strCmpStr(GD(i->arg1, curFrame)->String, GD(i->arg2, curFrame)->String);
				break;
			case iFIND:
				//call
				break;
			case iSORT:
				//call
				break;
			case iSUBSTR:
				//call
				break;

			case iSTOP:
				goto lbFinish;
			default:
				EXIT(ERR_INTERN, "Invalid instruction.\n");
		}

	}

	lbFinish:
	//free stuff
	strFree(string);
	fstackDeinit(&frames);

	return ret;
}

