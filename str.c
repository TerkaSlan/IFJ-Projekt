
#include <stdio.h>				//pre test #SK
#include <string.h>				//for strcmp
#include <inttypes.h>			//for "uint32_t"
#include <stdlib.h>				//for malloc
#include "str.h"

#define STR_INIT_LEN 32            //how many bits is allocated in initialization

dtStr *strNew() {
	dtStrPtr ret = malloc(sizeof(dtStr));
	if(!ret) {
		// Doplnit error #SK
		return NULL;
	}

	if(strInit(ret) == STR_ERROR) {
		// Doplnit error #SK

		free(ret);
		return STR_ERROR;
	}

	return ret;
}


int32_t strInit(dtStr *s) {
	if(s == NULL) {
		// Doplnit error #SK
	}
	if((s->str = (char *) malloc(STR_INIT_LEN)) == NULL) {
		return STR_ERROR;
	}
	s->str = '\0';
	s->uiLength = 0;
	s->uiAllocSize = STR_INIT_LEN;
	return STR_SUCCESS;
}


void strDeinit(dtStr *s) {
	if(s == NULL || s->str == NULL) {
		// Doplnit error #SK
	} else {
		free(s->str);
		s->uiLength = 0;            //nula sa dava aj pri init kde je ale \0 #SK
		s->uiAllocSize = 0;
	}
}

void strFree(dtStr *s) {
	if(s == NULL) {
		// Doplnit error #SK
		return;
	} else {
		strDeinit(s);
		free(s);
	}
}


void strClear(dtStr *s) {
	if(s == NULL || s->str == NULL) {
		// Doplnit error #SK
	}
	s->str[0] = '\0';
	s->uiLength = 0;
}

int32_t strAddChar(dtStr *s, char c) {
	if(s == NULL || s->str == NULL) {
		// Doplnit error #SK
	}
	if(s->uiLength + 1 >= s->uiAllocSize) {        //Malo by sa to nanajvys rovnat, ale pre istotu...ok? #SK
		// pamet nestaci, je potreba provest realokaci #SK
		if((s->str = (char *) realloc(s->str, s->uiLength + STR_INIT_LEN)) == NULL)
			return STR_ERROR;
		s->uiAllocSize = s->uiLength + STR_INIT_LEN;
	}
	s->str[s->uiLength] = c;
	s->uiLength++;
	s->str[s->uiLength] = '\0';
	return STR_SUCCESS;
}

int32_t strCopyString(dtStr *s1, dtStr *s2) {
	//S1 <- S2 | S1 destination, S2 source
	if(s1 == NULL || s2 == NULL || s2->str == NULL) {
		// Doplnit error #SK
	}
	uint32_t uiNewLength = s2->uiLength;
	if(uiNewLength >= s1->uiAllocSize) {
		//out of memory, need realloc
		if((s1->str = (char *) realloc(s1->str, uiNewLength + 1)) == NULL) {
			return STR_ERROR;
		}
		s1->uiAllocSize = uiNewLength + 1;
	}
	strcpy(s1->str, s2->str);
	s1->uiLength = uiNewLength;
	return STR_SUCCESS;
}

int32_t strCmpString(dtStr *s1, dtStr *s2) {
	if(s1 == NULL || s2 == NULL || s1->str == NULL || s2->str == NULL) {
		// Doplnit error #SK
	}
	return strcmp(s1->str, s2->str);
}

int32_t strCmpConstStr(dtStr *s1, char *s2) {
	if(s1 == NULL || s2 == NULL || s1->str == NULL) {
		// Doplnit error #SK
	}
	return strcmp(s1->str, s2);
}

char *strGetCStr(dtStr *s) {
	if(s == NULL || s->str == NULL) {
		// Doplnit error #SK
	}
	return s->str;
}

uint32_t strGetLength(dtStr *s) {
	if(s == NULL || s->str == NULL) {
		// Doplnit error #SK
	}
	return s->uiLength;
}