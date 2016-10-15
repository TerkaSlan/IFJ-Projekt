
#include <stdio.h>				//pre test #SK
#include <string.h>				//for strcmp
#include <inttypes.h>			//for "uint32_t"
#include <stdlib.h>				//for malloc
#include "str.h"
#include "error.h"

#define STR_INIT_LEN 32            //how many bits is allocated in initialization

/* TODO:
 * 	memset();
 * 	3 functions
 */

dtStr *strNew() {
	dtStr *s = NULL;
	if ((s = (dtStr*) malloc(sizeof(dtStr))) == NULL) {
		printError(ERR_LEX, "Couldn't allocate enough memory.");
		return NULL;
	}
	if (strInit(s) == STR_ERROR) {
		free(s);
		s = NULL;
		printError(ERR_LEX, "Couldn't initialize string.");
		return NULL;
	}
	return s;
}

int32_t strInit(dtStr *s) {
	if (s == NULL) {
		printError(ERR_LEX, "Parameter is NULL.");
		return STR_ERROR;
	}
	s->str = NULL;
	if ((s->str = (char *) malloc(STR_INIT_LEN)) == NULL) {
		printError(ERR_LEX, "Couldn't allocate enough memory.");
		return STR_ERROR;
	}
	s->str = '\0';
	s->uiLength = 0;
	s->uiAllocSize = STR_INIT_LEN;
	return STR_SUCCESS;
}


void strDeinit(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_LEX, "Parameter or its member is NULL.");
		return;
	}
	free(s->str);
	s->str = NULL;
	s->uiLength = 0;            //nula sa dava aj pri init kde je ale '\0'? #SK
	s->uiAllocSize = 0;
	
}

void strFree(dtStr *s) {
	if (s == NULL) {
		printError(ERR_LEX, "Parameter is NULL.");
		return;
	}
	strDeinit(s);
	free(s);
	s = NULL;
}


void strClear(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_LEX, "Parameter is NULL.");
		return;
	}
	s->str[0] = '\0';
	s->uiLength = 0;
}

int32_t strAddChar(dtStr *s, char c) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_LEX, "Parameter or its member is NULL");
		return STR_ERROR;
	}
	if (s->uiLength + 1 >= s->uiAllocSize) {		//Malo by sa to nanajvys rovnat, ale pre istotu...ok? #SK
		// pamet nestaci, je potreba provest realokaci #SK
    	if ((s->str = (char*) realloc(s->str, s->uiLength + STR_INIT_LEN)) == NULL) {
    		printError(ERR_LEX, "Couldn't reallocate memory.");
        	return STR_ERROR;
    	}
  		s->uiAllocSize = s->uiLength + STR_INIT_LEN;
	}
	s->str[s->uiLength] = c;
	s->uiLength++;
	s->str[s->uiLength] = '\0';
	return STR_SUCCESS;
}

int32_t strCopyStr(dtStr *s1, const dtStr *s2) {
	//S1 <- S2 | S1 destination, S2 source
	if (s1 == NULL || s2 == NULL || s2->str == NULL) {
		printError(ERR_LEX, "Parameter s1 or s2 or its member is NULL");
		return STR_ERROR;
	}
	uint32_t uiNewLength = s2->uiLength;
	if (uiNewLength >= s1->uiAllocSize) {
		//out of memory, need realloc
		if ((s1->str = (char *) realloc(s1->str, uiNewLength + 1)) == NULL) {
			printError(ERR_LEX, "Couldn't reallocate memory.");
			return STR_ERROR;
		}
		s1->uiAllocSize = uiNewLength + 1;
	}
	//memset();
	strcpy(s1->str, s2->str);		//copyruje '\0' znak? #SK
	s1->uiLength = uiNewLength;
	return STR_SUCCESS;
}

int32_t strCmpStr(dtStr *s1, dtStr *s2) {
	if (s1 == NULL || s2 == NULL || s1->str == NULL || s2->str == NULL) {
        printError(ERR_LEX, "Parameter s1 or s2 or theirs member is NULL");
		return STR_ERROR;
	}
	return strcmp(s1->str, s2->str);
}

int32_t strCmpCStr(dtStr *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL || s1->str == NULL) {
        printError(ERR_LEX, "Parameter s2 or s1 or its member is NULL");
		return STR_ERROR;
	}
	return strcmp(s1->str, s2);
}

char *strGetCStr(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_LEX, "Parameter s or its member is NULL");
		return '\0';
	}
	return s->str;
}

uint32_t strGetLength(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		printError(ERR_LEX, "Parameter s or its member is NULL");
		return '\0';
	}
	return s->uiLength;
}