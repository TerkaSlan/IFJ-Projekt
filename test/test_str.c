
#include "../str.h"
#include "test.h"

TEST_SUITE_START(StringTest);


dtStr *string = strNew();
SHOULD_EQUAL("strNew()", strcmp(string->str, "\0"), 0);

strAddChar(string, 'A');
SHOULD_EQUAL("strAddChar()", (strcmp(string->str, "A") || (string->uiLength != 1) ), 0);
strAddChar(string, 'H');
SHOULD_EQUAL("strAddChar()", (strcmp(string->str, "AH") || strcmp( &(string->str[string->uiLength]), "\0") || (string->uiLength != 2) ), 0);
bool isIn = false;
isIn = strCharPos(string, 'H');

dtStr *string2 = strNewFromCStr("String c2.");
SHOULD_EQUAL("strNewFromCStr()", (strcmp( &(string2->str[10]), "\0")  || (string2->uiLength != 10) ), 0);

dtStr *string2cpy = strNewFromCStr("String c2.");
SHOULD_EQUAL("strCmpStr()", strCmpStr(string2, string2cpy), 0);
strFree(string2cpy);

SHOULD_EQUAL("strCmpCStr()", strCmpCStr(string2, "String c2."), 0);

char *cStr = strGetCStr(string2);
SHOULD_EQUAL("strGetCStr()", strcmp(cStr, "String c2."), 0);

char *LoremIpsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec libero felis, porttitor ac porttitor sit amet, pharetra id sem. Sed vel dolor sem. Ut scelerisque et elit et posuere. Nam eros elit, placerat at varius vitae, bibendum in velit. Nam condimentum velit ac porttitor laoreet. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Etiam elementum vel massa sed dictum.";
strAddCStr(string, LoremIpsum);
SHOULD_EQUAL("strAddCStr()", (strcmp( &(string->str[string->uiLength]), "\0") || (string->uiLength != 410) ), 0);
isIn = isIn && (strCharPos(string, 'z') == -1);
isIn = isIn && (strCharPos(string, 'f') != -1);
SHOULD_EQUAL("strIsCharIn()", isIn, true);

SHOULD_EQUAL("strGetLength()", strGetLength(string), 410);

strCopyStr(string2, string);
SHOULD_EQUAL("strCopyStr()", (strcmp( &(string2->str[string2->uiLength]), "\0") || (string2->uiLength != 410) ), 0);

dtStr *string3 = strNewFromStr(string2);
SHOULD_EQUAL("strCopyStr()", (strcmp( &(string3->str[string3->uiLength]), "\0") || (string3->uiLength != 410) ), 0);

strClear(string);
SHOULD_EQUAL("strClear()", (strcmp(string->str, "\0") || (string->uiLength != 0) ), 0);

strFree(string3);
strFree(string2);
strFree(string);


TEST_SUITE_END
