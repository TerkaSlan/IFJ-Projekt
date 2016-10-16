//
// Created by Charvin on 15. 10. 2016.
//

#include "test.h"
#include "../ial.h"


TEST_SUITE_START(SymbolHashTableTest);


	tHashTablePtr table = htabInit();
	SHOULD_NOT_EQUAL("Table ptr is not null", table, NULL);

	tSymbolPtr symbol = symbolNew();
	SHOULD_NOT_EQUAL("New Symbol is not null", symbol, NULL);

	symbol->Initialized  = true;
	symbol->Static       = false;
	symbol->Type         = eINT;
	symbol->Data.Integer = 42;
	symbol->Name         = strNew();
	strAddCStr(symbol->Name, "SWAGZ");

	tSymbolPtr anothersymbol = symbolNewCopy(symbol);

	SHOULD_EQUAL("symbol copy", anothersymbol->Type, eINT);
	SHOULD_EQUAL("symbol copy", anothersymbol->Static, false);
	SHOULD_EQUAL("symbol copy", anothersymbol->Data.Integer, 42);
	SHOULD_EQUAL_STR("symbol copy", strGetCStr(anothersymbol->Name), "SWAGZ");

	htabAddSymbol(table, symbol);
	dtStrPtr string = strNewFromCStr("SWAGZ");
	SHOULD_EQUAL_STR("strCmpCStr", strGetCStr(string), "SWAGZ")
	SHOULD_EQUAL("strCmpStr", strCmpStr(string, symbol->Name), 0)


	SHOULD_EQUAL("adding symbol", htabGetSymbol(table, string), symbol)

	strClear(string);
	SHOULD_EQUAL_STR("strClear", strGetCStr(string), "")
	strAddCStr(string, "ASDASDASDASDASDASDASDASDASDASDYOYLO420BLAZEITNOSCOPE");
	SHOULD_EQUAL_STR("strClear", strGetCStr(string), "ASDASDASDASDASDASDASDASDASDASDYOYLO420BLAZEITNOSCOPE")
	SHOULD_EQUAL("get symbol that is not there", htabGetSymbol(table, string), NULL)

	strFree(anothersymbol->Name);
	anothersymbol->Name = string;

	anothersymbol->Data.Integer = 1337;
	htabAddSymbol(table, anothersymbol);
	SHOULD_EQUAL("get symbol that is there", htabGetSymbol(table, string), anothersymbol)

	tHashTablePtr newtable = htabCopy(table);
	SHOULD_EQUAL("get symbol that is there after copy", htabGetSymbol(newtable, string), anothersymbol)

	htabRemoveSymbol(table, string);
	SHOULD_EQUAL("get symbol that is not there", htabGetSymbol(table, string), NULL)

	dtStrPtr newstring = strNewFromStr(string);
	SHOULD_EQUAL_STR("string new copy from str", strGetCStr(newstring), strGetCStr(string))

	htabClear(newtable);
	SHOULD_EQUAL("get symbol that is not there", htabGetSymbol(newtable, newstring), NULL)

	htabFree(table);
	htabFree(newtable);
	strFree(newstring);


TEST_SUITE_END