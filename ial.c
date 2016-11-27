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

#include "ial.h"
#include <stdlib.h>
#include <string.h>


static uint32_t htabHashFunc(char *name, uint32_t htabSize);
static void partition(dtStr *s, int32_t low, int32_t high);


tHashTablePtr htabInit(uint32_t size) {
	tHashTablePtr ret = calloc(1, sizeof(*ret) + size * sizeof(tSymbolPtr));

	if(!ret)
		return NULL;

	ret->Size          = size;
	ret->NumberOfItems = 0;

	return ret;

}

tHashTablePtr htabCopy(const tHashTablePtr table) {
	if(!table)
		return NULL;

	tHashTablePtr ret = htabInit(table->Size);

	if(!ret)
		return NULL;

	ret->NumberOfItems = table->NumberOfItems;


	//deep data copy
	for(uint32_t index = 0; index < table->Size; index++) {
		for(tSymbolPtr symbolIterator = table->Data[index], symbolPrevious = NULL;
		    symbolIterator != NULL; symbolIterator = symbolIterator->Next) {
			//create new symbol
			tSymbolPtr symbolNew = malloc(sizeof(tSymbol));

			if(!symbolNew) {
				htabFree(ret);
				return NULL;
			}

			//copy symbol
			memcpy(symbolNew, symbolIterator, sizeof(tSymbol)); //use memcpy to allow changes in tSymbol structure
			symbolNew->Next = NULL;

			//new space for a key
			symbolNew->Name = strNew();

			if(!(symbolNew->Name)) {
				//whoops cant alloc a new key
				htabFree(ret);
				return NULL;
			}

			strCopyStr(symbolNew->Name, symbolIterator->Name);


			//insert into the table
			if(symbolPrevious == NULL)
				ret->Data[index] = symbolNew;
			else
				symbolPrevious->Next = symbolNew;

			symbolPrevious = symbolNew;
		}
	}

	return ret;

}



static tSymbolPtr indexGenerator(tSymbolPtr sym, void* param)
{
	int *counter = (int*) param;
	if(sym->Index == -1)
		sym->Index = (*counter)++;

	return sym;
}
void htabGenerateIndices(tHashTablePtr table)
{
	int32_t counter = 0;

	//For function local table
	if(table->Parent->Type == eFUNCTION)
	{
		//lets start with arguments
		for(tArgumentListItem* argItem = table->Parent->Data.FunctionData.ArgumentList; argItem != NULL; argItem = argItem->Next)
		{
			if(argItem->Symbol->Const)
				abort(); //if this happens, something is really wrong with the code.

			//give index
			if(argItem->Symbol->Index == -1)
				argItem->Symbol->Index = counter++;
		}

		//fill the rest
		htabForEach(table, indexGenerator, &counter); //pointer to local var, but should be safe
	}

}


tSymbolPtr htabAddSymbol(tHashTablePtr table, const tSymbolPtr symbol, bool overwrite) {
	if(!table || !symbol)
		return NULL;

	//check if element exists
	tSymbolPtr foundSymbol = htabGetSymbol(table, symbol->Name);

	if(foundSymbol)
	{
		if(!overwrite)
			return NULL;

		//overwriting found symbol
		tSymbolPtr nextBkup = foundSymbol->Next;
		dtStrPtr nameBkup = foundSymbol ->Name;
		memcpy(foundSymbol, symbol, sizeof(tSymbol));
		foundSymbol->Next = nextBkup;
		foundSymbol->Name = nameBkup;

		return foundSymbol;
	}
	else {

		//create a copy of a symbol
		tSymbolPtr symbolCopy = symbolNewCopy(symbol);

		if(!symbolCopy)
			return NULL;

		//hash name to get index into the hash table
		uint32_t index = htabHashFunc(strGetCStr(symbolCopy->Name), table->Size);

		//insert new item to the beginning of the list
		symbolCopy->Next = table->Data[index];
		table->Data[index] = symbolCopy;
		table->NumberOfItems++;

		return symbolCopy;
	}
}

tSymbolPtr htabGetSymbol(const tHashTablePtr table, dtStrPtr name) {
	if(!table || !name)
		return NULL;

	//hash name to get index into the hash table
	uint32_t index = htabHashFunc(strGetCStr(name), table->Size);
	tSymbolPtr ret = table->Data[index];

	//is it in the list?
	for(; ret != NULL; ret = ret->Next) {
		if(!strCmpStr(name, ret->Name))
			return ret;

	}

	return NULL;

}

bool htabForEach(tHashTablePtr table, tSymbolPtr (*func)(tSymbolPtr, void*), void* param) {
	if(!table || !func)
		return false;


	bool ret = true;
	for(uint32_t htabIndex = 0; htabIndex < table->Size; htabIndex++) {

		for(tSymbolPtr symbolIterator = table->Data[htabIndex];
		    symbolIterator != NULL; symbolIterator = symbolIterator->Next) {
			//call a func for each item
			if(func(symbolIterator, param) == NULL)
				ret = false;
		}

	}

	return ret;
}

void htabRemoveSymbol(tHashTablePtr table, dtStrPtr name) {
	//ptr check
	if(!table || !name)
		return;

	uint32_t index = htabHashFunc(strGetCStr(name), table->Size);

	//if hash table is empty
	if(!(table->Data[index]))
		return;

	tSymbolPtr symbolIterator = table->Data[index];

	//found, it is the first entry
	if(!strCmpStr(symbolIterator->Name, name)) {
		//backup next
		tSymbolPtr symbolNext = symbolIterator->Next;

		//free item
		symbolFree(table->Data[index]);

		//reconnect the list
		table->Data[index] = symbolNext;

		table->NumberOfItems--;
		return;
	}

	//not first entry lets loop
	for(; symbolIterator != NULL; symbolIterator = symbolIterator->Next) {
		//is it the one after iter?
		if(symbolIterator->Next && !strCmpStr(symbolIterator->Next->Name, name)) {
			//make a backup
			tSymbolPtr symbolNext = symbolIterator->Next->Next;
			//free item
			symbolFree(symbolIterator->Next);
			//reconnect the list
			symbolIterator->Next = symbolNext;

			table->NumberOfItems--;
			return;
		}
	}
}

void htabClear(tHashTablePtr table) {
	if(!table)
		return;

	//for each line in the hash table, dump all items in the list
	for(uint32_t htabIndex = 0; htabIndex < table->Size; htabIndex++) {

		while(table->Data[htabIndex]) {
			//note what item follows in the list
			tSymbolPtr followingItem = table->Data[htabIndex]->Next;

			//dump first item in the list
			symbolFree(table->Data[htabIndex]);
			//reconnect the list
			table->Data[htabIndex] = followingItem;
		}

	}

	table->NumberOfItems = 0;

}

static tSymbolPtr recursiveDelete(tSymbolPtr symbol, void* param)
{
	(void)param;
	switch(symbol->Type)
	{
		case eCLASS:
			htabRecursiveFree(symbol->Data.ClassData.LocalSymbolTable);
			break;
		case eFUNCTION:
			htabFree(symbol->Data.FunctionData.LocalSymbolTable);
			break;
		default:
			break;
	}
	return symbol;
}

void htabRecursiveFree(tHashTablePtr table)
{
	if(!table)
		return;

	htabForEach(table, recursiveDelete, NULL);
	htabFree(table);
}

void htabFree(tHashTablePtr table) {
	if(!table)
		return;

	htabClear(table);
	table->Size = 0;
	free(table);
}

//CREDIT IJC 2015/2016
uint32_t htabHashFunc(char *name, uint32_t htabSize) {
	uint32_t h = 0;
	const unsigned char *p;
	for(p = (const unsigned char *) name; *p != '\0'; p++)
		h = 65599 * h + *p;

	return h % htabSize;
}


tSymbolPtr symbolNew() {
	tSymbolPtr ret = calloc(1, sizeof(*ret));
	ret->Index = -1;
	return ret;
}

tSymbolPtr symbolNewCopy(const tSymbolPtr symbol) {
	if(!symbol)
		return NULL;

	tSymbolPtr ret = malloc(sizeof(*ret));
	if(ret == NULL)
		return NULL;

	//copy symbol
	if(memcpy(ret, symbol, sizeof(tSymbol)) == NULL)
		goto ERRORsymbolNewCopy;


	//if has name, copy
	if(symbol->Name) {

		if((ret->Name = strNewFromStr(symbol->Name)) == NULL)
			goto ERRORsymbolNewCopy;

	}

	return ret;

	ERRORsymbolNewCopy:
	free(ret);
	return NULL;
}

tSymbolPtr symbolFuncAddArgument(tSymbolPtr symbolFunc, const tSymbolPtr argument)
{
	if(symbolFunc->Type != eFUNCTION)
		return NULL;

	//new argument item
	tArgumentListItem *newItem = calloc(1, sizeof(tArgumentListItem));
	if(!newItem)
		return NULL;

	//insert argument item into funcdata
	if(symbolFunc->Data.FunctionData.ArgumentList == NULL)
		symbolFunc->Data.FunctionData.ArgumentList = newItem; //first argument

	else
	{
		tArgumentListItem *lastItem = symbolFunc->Data.FunctionData.ArgumentList;
		//get to the last argument and add new one
		for(; lastItem->Next != NULL; lastItem = lastItem->Next);
		lastItem->Next = newItem;
	}

	newItem->Symbol = argument;
	symbolFunc->Data.FunctionData.NumberOfArguments++;
	return argument;

}


void symbolFree(tSymbolPtr symbol) {
	if(!symbol)
		return;

	//dealloc name
	if(symbol->Name)
		strFree(symbol->Name);

	if(symbol->Type == eFUNCTION)
	{
		while(symbol->Data.FunctionData.ArgumentList)
		{
			tArgumentListItem *nextArg = symbol->Data.FunctionData.ArgumentList->Next;
			free(symbol->Data.FunctionData.ArgumentList);
			symbol->Data.FunctionData.ArgumentList = nextArg;
		}

	}
	if(symbol->Type == eSTRING && symbol->Data.String)
		strFree(symbol->Data.String);

	//dealloc symbol
	free(symbol);
}

//-------------------------------------------------------------------
//-----------------------------QuickSort-----------------------------
//-------------------------------------------------------------------

dtStr *sort(dtStr *s) {
	if (s == NULL) {
		return NULL;
	}
	dtStr *sortStr = strNewFromStr(s);
	if (sortStr == NULL) {
		return NULL;
	}
	if (sortStr->uiLength == 0) {
		return sortStr;
	}
	partition(sortStr, 0, sortStr->uiLength - 1);
	return sortStr;
}

void partition(dtStr *s, int32_t low, int32_t high) {
	int32_t i = low;
	int32_t j = high;
	char pMedian = s->str[(i+j) / 2];
	char tmpChar;
	//pMedian is pseudo median
	do	{
		while (s->str[i] < pMedian)
			i++;
		while (s->str[j] > pMedian)
			j--;
		if (i <= j) {
			tmpChar = s->str[i];
			s->str[i] = s->str[j];
			s->str[j] = tmpChar;
			i++;
			j--;
		}
	} while (i <= j);
	if (i < high)
		partition(s, i, high);
	if (j > low)
		partition(s, low, j);
}


//-------------------------------------------------------------------
//-----------------------------Find----------------------------------
//-------------------------------------------------------------------

int32_t find(dtStr* s, dtStr* search) {
	if (s == NULL || search == NULL) {
		return -1;
	}

	// At first we must get a vector fail, wchich represents targets of back arrows
	uint32_t fail[s->uiLength+1];
	uint32_t r;
	fail[0] = 0;
	for (uint32_t k = 1; k < s->uiLength; k++) {
		r = fail[k-1];
		while ((r>0) && (s->str[r-1] != s->str[k-1])) {
			r = fail[r-1];
		}
		fail[k] = r + 1;
	}


	uint32_t searchIndex = 1;
	uint32_t sIndex = 1;
	while ((sIndex <= s->uiLength) && searchIndex <= search->uiLength) {
		if ((searchIndex == 0) || (s->str[sIndex-1] == search->str[searchIndex-1])) {
			sIndex++;
			searchIndex++;
		} else {
			searchIndex = fail[searchIndex-1];
		}
	}
	if (searchIndex > search->uiLength) {
		return sIndex - search->uiLength - 1;
	}
	// s doesn't contains substring search, so return -1 according to documentation
	return -1;
}
