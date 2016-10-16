#include "ial.h"
#include <stdlib.h>
#include <string.h>

static uint32_t htabHashFunc(char *name, uint32_t htabSize);


#define HTAB_DEFAULT_SIZE 4099

tHashTablePtr htabInit() {
	tHashTablePtr ret = malloc(sizeof(*ret) + HTAB_DEFAULT_SIZE * sizeof(tSymbolPtr));

	if(!ret)
		return NULL;

	ret->Size = HTAB_DEFAULT_SIZE;
	ret->NumberOfItems = 0;

	//make table NULL
	memset(ret->Data, 0, HTAB_DEFAULT_SIZE * sizeof(tSymbolPtr));

	return ret;

}

tHashTablePtr htabCopy(tHashTablePtr table) {
	if(!table)
		return NULL;

	tHashTablePtr ret = malloc(sizeof(*ret) + table->Size * sizeof(tSymbolPtr));

	if(!ret)
		return NULL;

	ret->Size = table->Size;
	ret->NumberOfItems = table->NumberOfItems;

	//make table NULL
	memset(ret->Data, 0, ret->Size * sizeof(tSymbolPtr));


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


tSymbolPtr htabAddSymbol(tHashTablePtr table, tSymbolPtr symbol) {
	if(!table || !symbol)
		return NULL;

	//check if element exists
	if(htabGetSymbol(table, symbol->Name) != NULL)
		return NULL;

	//hash name to get index into the hash table
	uint32_t index = htabHashFunc(strGetCStr(symbol->Name), table->Size);


	//first item in the list will become second
	symbol->Next = table->Data[index];


	//insert new item to the beginning of the list	
	table->Data[index] = symbol;
	table->NumberOfItems++;

	return symbol;

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
	tSymbolPtr ret = malloc(sizeof(*ret));
	if(ret == NULL)
		return NULL;

	//make everything NULL
	memset(ret, 0, sizeof(*ret));
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

	ERRORsymbolNewCopy:
	free(ret);
	return NULL;
}



void symbolFree(tSymbolPtr symbol) {
	if(!symbol)
		return;

	//dealloc name
	if(symbol->Name)
		strFree(symbol->Name); ///TODO::make sure they fixed it!!!!

	//dealloc symbol
	free(symbol);
}