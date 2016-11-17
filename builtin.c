
#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

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

eError readData(tSymbolPtr symbol, tSymbolData* data) {
	int32_t c;
	char next, tmp;
	switch (symbol->Type) {
		case eINT:
			fscanf(stdin, "%" SCNd32 "%c" , &(data->Integer), &next);
			tmp = next;
			while (tmp != '\n' && tmp != EOF) {
				tmp = getchar();
			}		
			if (next != '\n' && next != EOF) {
				printError(ERR_RUN_INPUT, "Unexpected data (expected integer)");
				return ERR_RUN_INPUT;
			}
			break;

		case eDOUBLE:
			fscanf(stdin, "%lf%c", &(data->Double), &next);
			tmp = next;
			while (tmp != '\n' && tmp != EOF) {
				tmp = getchar();
			}
			if (next != '\n'&& next != EOF) {
				printError(ERR_RUN_INPUT, "Unexpected data (expected double)");				
				return ERR_RUN_INPUT;
			}
			break;

		case eSTRING:
			if ((data->String = strNew()) == NULL) {
				return ERR_INTERN;
			}
			while ((c = getc(stdin)) != EOF && c != '\n') {
				strAddChar(data->String, c);
			}
			break;

		default:
			printError(ERR_RUN_INPUT, "Unexpected data");
			return ERR_RUN_INPUT;
	}
	return ERR_OK;
}
