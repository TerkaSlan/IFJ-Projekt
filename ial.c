#include "ial.h"
#include <stdio.h>
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

tHashTablePtr htabCopy(tHashTablePtr table) {
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


tSymbolPtr htabAddSymbol(tHashTablePtr table, const tSymbolPtr symbol) {
	if(!table || !symbol)
		return NULL;

	//check if element exists
	if(htabGetSymbol(table, symbol->Name) != NULL)
		return NULL;

	//create a copy of a symbol
	tSymbolPtr symbolCopy = symbolNewCopy(symbol);

	//hash name to get index into the hash table
	uint32_t index = htabHashFunc(strGetCStr(symbolCopy->Name), table->Size);


	//first item in the list will become second
	symbolCopy->Next = table->Data[index];


	//insert new item to the beginning of the list	
	table->Data[index] = symbolCopy;
	table->NumberOfItems++;

	return symbolCopy;

}

tSymbolPtr htabGetSymbol(tHashTablePtr table, dtStrPtr name) {
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

void htabForEach(tHashTablePtr table, void (*func)(tSymbolPtr)) {
	if(!table || !func)
		return;

	for(uint32_t htabIndex = 0; htabIndex < table->Size; htabIndex++) {

		for(tSymbolPtr symbolIterator = table->Data[htabIndex];
		    symbolIterator != NULL; symbolIterator = symbolIterator->Next) {
			//call a func for each item
			func(symbolIterator);
		}

	}

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

void htabFree(tHashTablePtr table) {
	if(!table)
		return;

	htabClear(table);
	table->Size = 0;
	free(table);
}


uint32_t htabHashFunc(char *name, uint32_t htabSize) {
	uint32_t h = 0;
	const unsigned char *p;
	for(p = (const unsigned char *) name; *p != '\0'; p++)
		h = 65599 * h + *p;

	return h % htabSize;
}


tSymbolPtr symbolNew() {
	tSymbolPtr ret = calloc(1, sizeof(*ret));

	return ret;
}

tSymbolPtr symbolNewCopy(tSymbolPtr symbol) {
	if(!symbol)
		return NULL;

	tSymbolPtr ret = malloc(sizeof(*ret));
	if(ret == NULL)
		return NULL;

	//copy symbol
	if(memcpy(ret, symbol, sizeof(tSymbol)) == NULL)
		goto ERRORsymbolNewCopy;

	if((ret->Name = strNew()) == NULL)
		goto ERRORsymbolNewCopy;

	if(strCopyStr(ret->Name, symbol->Name) == STR_SUCCESS)
		return ret;

	free(ret->Name);

	ERRORsymbolNewCopy:
	free(ret);
	return NULL;
}



void symbolFree(tSymbolPtr symbol) {
	if(!symbol)
		return;

	//dealloc name
	if(symbol->Name)
		strFree(symbol->Name);

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