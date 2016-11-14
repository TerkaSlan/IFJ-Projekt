
#include "builtin.h"
#include "error.h"
#include <stdlib.h>

dtStr *substr(dtStr *s, uint32_t beginIndex, uint32_t endIndex) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_OTHER, "Parameter or its member is NULL");
		return NULL;
	}

	if (endIndex > s->uiLength) {
		printError(ERR_OTHER, "String index is out of range");
		return NULL;
	}
	
	if (beginIndex > endIndex) {
		printError(ERR_OTHER, "String index is out of range");
		return NULL;
	}

	if (beginIndex < 0) {
		printError(ERR_OTHER, "String index is out of range");
		return NULL;
	}

	dtStr *subStr = strNew();
	for (uint32_t i = beginIndex; i < endIndex; i++) {
		strAddChar(subStr, s->str[i]);
	}
	return subStr;	
}
