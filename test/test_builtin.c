
#include "../builtin.h"
#include "test.h"

TEST_SUITE_START(BuiltinTest);

dtStr *string = strNewFromCStr("0123456789");
dtStr *substring1;
substr(string, 1, 2, &substring1);
SHOULD_EQUAL("SubString1", strCmpCStr(substring1, "1"), 0);
strFree(substring1);

dtStr *substring2;
substr(string, 0, 5, &substring2);
SHOULD_EQUAL("SubString2 (beginIndex = 0)", strCmpCStr(substring2, "01234"), 0);
strFree(substring2);

dtStr *substring3;
substr(string, 5, 5, &substring3);
SHOULD_EQUAL("SubString3 (beginIndex = endIndex)", strCmpCStr(substring3, ""), 0);
strFree(substring3);	

dtStr *substring4;
SHOULD_EQUAL("SubString4 (beginIndex > endIndex)", (substr(string, 6, 2, &substring4) == ERR_OTHER), 1);

dtStr *substring5;
SHOULD_EQUAL("SubString5 (endIndex > s->uiLength)", (substr(string, 1, 11, &substring5) == ERR_OTHER), 1);

dtStr *substring6;
SHOULD_EQUAL("SubString6 (beginIndex < 0)", (substr(string, -1, 5, &substring6) == ERR_OTHER), 1);

strFree(string);

tSymbolPtr symbol_1 = symbolNew();
symbol_1->Type = eINT; 
printf("readInt()\n");
if (readData(symbol_1, &symbol_1->Data) != ERR_OK) {
	printf("Invalid input\n");
}
else printf("Integer on stdin\n");
symbolFree(symbol_1);

tSymbolPtr symbol_2 = symbolNew();
symbol_2->Type = eDOUBLE; 
printf("readDouble()\n");
if (readData(symbol_2, &symbol_2->Data) != ERR_OK) {
	printf("Invalid input\n");
}
else printf("Double on stdin\n");
symbolFree(symbol_2);

tSymbolPtr symbol_3 = symbolNew();
symbol_3->Type = eSTRING; 
printf("readString()\n");
if (readData(symbol_3, &symbol_3->Data) != ERR_OK) {
	printf("Invalid input\n");
}
else printf("String on stdin:\n");
printf("%s\n", symbol_3->Data.String->str);

strFree(symbol_3->Data.String);
symbolFree(symbol_3);


TEST_SUITE_END
