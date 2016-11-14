
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

TEST_SUITE_END
