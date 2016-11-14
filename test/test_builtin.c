
#include "../builtin.h"
#include "test.h"

TEST_SUITE_START(BuiltinTest);

dtStr *string = strNewFromCStr("0123456789");
dtStr *substring1 = substr(string, 1, 2);
SHOULD_EQUAL("SubString1", strCmpCStr(substring1, "1"), 0);
strFree(substring1);

dtStr *substring2 = substr(string, 0, 5);
SHOULD_EQUAL("SubString2 (beginIndex = 0)", strCmpCStr(substring2, "01234"), 0);
strFree(substring2);

dtStr *substring3 = substr(string, 5, 5);
SHOULD_EQUAL("SubString3 (beginIndex = endIndex)", strCmpCStr(substring3, ""), 0);
strFree(substring3);	

dtStr *substring4 = substr(string, 6, 2);
SHOULD_EQUAL("SubString4 (beginIndex > endIndex)", (substring4 == NULL), 1);
strFree(substring4);

dtStr *substring5 = substr(string, 1, 11);
SHOULD_EQUAL("SubString5 (endIndex > s->uiLength)", (substring5 == NULL), 1);
strFree(substring5);

dtStr *substring6 = substr(string, -1, 5);
SHOULD_EQUAL("SubString6 (beginIndex < 0)", (substring6 == NULL), 1);
strFree(substring6);

strFree(string);

TEST_SUITE_END
