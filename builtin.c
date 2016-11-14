
#include "builtin.h"
#include <stdlib.h>

eError substr(const dtStr *s, int32_t beginIndex, int32_t endIndex, dtStrPtr *subStr) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_OTHER, "Parameter or its member is NULL");
		return ERR_OTHER;
	}

	if (endIndex > s->uiLength) {
		printError(ERR_OTHER, "String index is out of range");
		return ERR_OTHER;
	}
	
	if (beginIndex > endIndex) {
		printError(ERR_OTHER, "String index is out of range");
		return ERR_OTHER;
	}

	if (beginIndex < 0) {
		printError(ERR_OTHER, "String index is out of range");
		return ERR_OTHER;
	}

	*subStr = strNew();
	if (subStr == NULL) {
		return ERR_INTERN;
	}
	for (uint32_t i = beginIndex; i < endIndex; i++) {
		strAddChar(*subStr, s->str[i]);
	}
	return ERR_OK;
}
