
#include "../ial.h"
#include "test.h"

TEST_SUITE_START(ialTest);

dtStr *unord = strNew();
char *ord = "abcdefghijklmnopqrstuvwxyz";
strAddCStr(unord, "ofvimgwknbhjxclsrteqyapuzd");

unord = sort(unord);
	printf("%s\n", unord->str);
	printf("abcdefghijklmnopqrstuvwxyz\n");

SHOULD_EQUAL("QuickSort unorder alphabet", strCmpCStr(unord, ord), 0);

dtStr *empty = strNew();
empty = sort(empty);
SHOULD_EQUAL("QuickSort on empty str", strCmpCStr(empty, ""), 0);

strFree(unord);
strFree(empty);


TEST_SUITE_END
