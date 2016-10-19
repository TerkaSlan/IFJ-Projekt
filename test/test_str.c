#include "../str.h"
#include "test.h"

TEST_SUITE_START(StringTest);

dtStr *string = strNew();

SHOULD_EQUAL("strNew()", strcmp(string->str, "\0"), 0);

strFree(string);


TEST_SUITE_END
