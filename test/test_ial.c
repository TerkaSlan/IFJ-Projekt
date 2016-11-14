
#include "../ial.h"
#include "test.h"

TEST_SUITE_START(ialTest);

dtStr *unord = strNew();
char *ord = "abcdefghijklmnopqrstuvwxyz";
strAddCStr(unord, "ofvimgwknbhjxclsrteqyapuzd");
dtStr *unordSort = sort(unord);
SHOULD_EQUAL("QuickSort (unordered alphabet)", strCmpCStr(unordSort, ord), 0);

dtStr *empty = strNew();
dtStr *emptySort = sort(empty);
SHOULD_EQUAL("QuickSort on empty str", strCmpCStr(emptySort, ""), 0);

strFree(unord);
strFree(unordSort);
strFree(empty);
strFree(emptySort);


TEST_SUITE_END
