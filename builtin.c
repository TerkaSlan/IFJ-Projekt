
#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

eError substr(const dtStr *s, int32_t beginIndex, int32_t endIndex, dtStrPtr *subStr) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_INTERN, "In substr: Parameter or its member is NULL\n");
		return ERR_INTERN;
	}

	if (endIndex > s->uiLength) {
		printError(ERR_OTHER, "In substr: String index is out of range\n");
		return ERR_OTHER;
	}
	
	if (beginIndex > endIndex) {
		printError(ERR_OTHER, "In substr: String index is out of range\n");
		return ERR_OTHER;
	}

	if (beginIndex < 0) {
		printError(ERR_OTHER, "In substr: String index is out of range\n");
		return ERR_OTHER;
	}

	*subStr = strNew();
	if (subStr == NULL) {
		printError(ERR_INTERN, "In substr: Cannot create string. Out of memory.\n");
		return ERR_INTERN;
	}
	for (uint32_t i = beginIndex; i < endIndex; i++) {
		strAddChar(*subStr, s->str[i]);
	}
	return ERR_OK;
}

eError readData(tSymbolPtr symbol, tSymbolData* data) {
	int32_t c;
	char tmp;
	switch (symbol->Type) {
		case eINT:
			fscanf(stdin, "%" SCNd32 "%c" , &(data->Integer), &tmp);
			if (tmp != '\n'&& tmp != EOF ) {
				printError(ERR_RUN_INPUT, "Error while reading from stdin: unexpected data (expected integer)\n");
				return ERR_RUN_INPUT;
			}
			break;

		case eDOUBLE:
			fscanf(stdin, "%lf%c", &(data->Double), &tmp);
			if (tmp != '\n'&& tmp != EOF ) {
				printError(ERR_RUN_INPUT, "Error while reading from stdin: unexpected data (expected double)\n");
				return ERR_RUN_INPUT;
			}
			break;

		case eSTRING:
			if (strNew(data->String) == NULL) {
				printError(ERR_INTERN, "In readData: Cannot create string. Out of memory.\n");
				return ERR_INTERN;
			}
			while ((c = getc(stdin)) != EOF && c != '\n') {
				strAddChar(data->String, c);
			}
			break;

		default:
			printError(ERR_RUN_INPUT, "Error while reading from stdin: unexpected data\n");
			return ERR_RUN_INPUT;
	}
	return ERR_OK;	
}
