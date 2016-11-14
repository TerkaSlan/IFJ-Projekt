
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

dtStr* string = strNew();
dtStr* substring = strNew();
SHOULD_EQUAL("Find empty string in empty string", find(string, substring), 0);
strAddCStr(substring, "JKLM");
SHOULD_EQUAL("Find non-empty substring in empty string", find(string, substring), -1);
strClear(substring);

strAddCStr(string, "ABABABCB");
SHOULD_EQUAL("Find, empty substring", find(string, substring), 0);

strAddChar(substring, 'A');
SHOULD_EQUAL("Find A in ABABABCB", find(string, substring), 0);
strAddChar(substring, 'B');
SHOULD_EQUAL("Find AB in ABABABCB", find(string, substring), 0);
strAddChar(substring, 'C');
SHOULD_EQUAL("Find ABC in ABABABCB", find(string, substring), 4);
strAddChar(substring, 'D');
SHOULD_EQUAL("Find ABCD in ABABABCB", find(string, substring), -1);

strFree(string);
strFree(substring);


TEST_SUITE_END
