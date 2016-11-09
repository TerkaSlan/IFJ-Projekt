//
// Created by Charvin on 29. 10. 2016.
//

#include "interpret.h"
#include "frame.h"
#include "ial.h"
#include "instruction.h"
#include <stdlib.h>


eError Interpret(tHashTablePtr globalClassTable, tConstContainerPtr constContainer, tInstructionListPtr instrList) {
	//lets start this shit
#define EXIT(err, msg) do{ret = err; printError(err, msg); goto lbFinish;}while(0)
	eError ret = ERR_OK;

	//create frame stack
	tFrameStack frames;
	fstackInit(&frames);

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
	tFrame mainFrame;
	frameBuild(&mainFrame, mainSymbol);
	mainFrame.ReturnInstruction = 0;
	if((curFrame = fstackPush(&frames, &mainFrame)) == NULL)
		EXIT(ERR_INTERN, "Cannot create frame stack. Out of memory.\n");

	//set this as first instruction
	instrListSetFirstInstruction(instrList, mainSymbol->Data.FunctionData.InstructionIndex);

	//get instruction and do shit, loop
	tInstructionPtr i = NULL;
	while(1) {
		if(!(i = instrListGetNextInstruction(instrList)))
			EXIT(ERR_OTHER, "Error getting next instruction.\n");

#define GD symbolGetData
		//INTERPRET!
		switch(i->type) {
			///TODO::all switch cases :)

			case iMOV: {

				switch(((tSymbolPtr) i->dst)->Type) {
					case eINT:
						GD(i->dst, curFrame)->Integer = GD(i->arg1, curFrame)->Integer;
						break;

					case eDOUBLE:
						GD(i->dst, curFrame)->Double = GD(i->arg1, curFrame)->Double;
						break;

					case eBOOL:
						GD(i->dst, curFrame)->Bool = GD(i->arg1, curFrame)->Bool;
						break;

					case eSTRING: {
						dtStrPtr tmpDst = GD(i->dst, curFrame)->String;
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
						     "Trying to assign class or func.\n"); //we have to think if assigning classes is possible...
				}
				break;
			}


			case iFRAME: {


				break;
			}

			case iPUSH:

				break;
			case iCALL:

				break;
			case iRET:

				break;
			case iGETRETVAL:

				break;
			case iINC:

				break;
			case iDEC:

				break;
			case iADD:

				break;
			case iSUB:

				break;
			case iMUL:

				break;
			case iDIV:

				break;
			case iNEG:

				break;
			case iLEQ:

				break;
			case iLNEQ:

				break;
			case iLAND:

				break;
			case iLOR:

				break;
			case iLNOT:

				break;
			case iGOTO:

				break;
			case iIFGOTO:

				break;
			case iIFNGOTO:

				break;
			case iCONV2STR: {
				dtStrPtr tmp = GD(i->dst, curFrame)->String;
				if(tmp)
					strFree(tmp);
				///TODO::tmp = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			}
			case iCONV2INT:
				///TODO::GD(i->dst, curFrame)->Integer = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iCONV2BOOL:
				///TODO::GD(i->dst, curFrame)->Boolean = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iCONV2DOUBLE:
				///TODO::GD(i->dst, curFrame)->Double = call(GD(i->arg1, curFrame), ((tSymbolPtr)(i->arg1))->Type);
				break;
			case iCAT:
				//call
				break;
			case iPRINT:
				//call
				break;
			case iREAD:
				//call
				break;
			case iLEN:
				GD(i->dst, curFrame)->Integer = strGetLength(GD(i->arg1, curFrame)->String);
				break;
			case iCOMPARE:
				GD(i->dst, curFrame)->Integer = strCmpStr(GD(i->arg1, curFrame)->String, GD(i->arg2, curFrame)->String);
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


				//special end program
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


