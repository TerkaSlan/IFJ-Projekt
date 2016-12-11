/**
  *  \file str.c
  *  \brief Contains implementation of functions regarding the string abstraction
  *  \author Jakub
  *  \author Marek
  *
  * Project: IFJ
  * Implementace interpretu imperativního jazyka IFJ16.
  *
  * Description:
  * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2016.pdf
  *
  * Team:
  * Michal Charvát          (xcharv16)
  * Terézia Slanináková     (xslani06)
  * Katarína Grešová        (xgreso00)
  * Marek Schauer           (xschau00)
  * Jakub Handzuš           (xhandz00)
  */


#include "str.h"
#include <string.h>
#include <stdlib.h>

#define STR_INIT_LEN 32            //how many bytes is allocated in initialization

/* TODO:
 */

/**
 * This function initializes dtStr structure (local) and makes it empty with '\0' at the end. NOTE: Use for local objects only! NOT RECOMMENDED, DEPRECATED
 *
 * @param      s     dtStr, structure representing string
 *
 * @return     STR_SUCCESS or STR_ERROR according to initialization result
 */
static int32_t strInit(dtStr *s);

/**
 * This function deinitializes dtStr structure (local) and makes it empty with '\0' at the end. NOTE: Use for local objects only! NOT RECOMMENDED, DEPRECATED
 *
 * @param      s     dtStr, structure representing string
 *
 * @return     STR_SUCCESS or STR_ERROR according to initialization result
 */
static void strDeinit(dtStr *s);

dtStr *strNew() {
	dtStr *s = NULL;
	if ((s = (dtStr*) malloc(sizeof(dtStr))) == NULL) {
		return NULL;
	}
	if (strInit(s) == STR_ERROR) {
		free(s);
		s = NULL;
		return NULL;
	}
	return s;
}

int32_t strInit(dtStr *s) {
	if (s == NULL) {
		return STR_ERROR;
	}
	s->str = NULL;
	if ((s->str = (char *) malloc(STR_INIT_LEN)) == NULL) {
		return STR_ERROR;
	}
	s->str[0] = '\0';
	s->uiLength = 0;
	s->uiAllocSize = STR_INIT_LEN;
	return STR_SUCCESS;
}


void strDeinit(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		return;
	}
	free(s->str);
	s->str = NULL;
	s->uiLength = 0;
	s->uiAllocSize = 0;

}

void strFree(dtStr *s) {
	if (s == NULL) {
		return;
	}
	strDeinit(s);
	free(s);
}


void strClear(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		return;
	}
	s->str[0] = '\0';
	s->uiLength = 0;
}

int32_t strAddChar(dtStr *s, char c) {
	if (s == NULL || s->str == NULL) {
		return STR_ERROR;
	}
	if (s->uiLength + 1 >= s->uiAllocSize) {
		// out of alocated memory, need realocation
    	if ((s->str = (char*) realloc(s->str, s->uiAllocSize * 2)) == NULL) {
        	return STR_ERROR;
    	}
  		s->uiAllocSize = s->uiAllocSize * 2;
	}
	s->str[s->uiLength] = c;
	s->uiLength++;
	s->str[s->uiLength] = '\0';
	return STR_SUCCESS;
}

int32_t strAddCStr(dtStr *s, char *str) {
	if (s == NULL || s->str == NULL || str == NULL) {
		return STR_ERROR;
	}
	uint32_t uiStrLength = strlen(str);
	if (s->uiAllocSize <= s->uiLength + uiStrLength) {
		// out of alocated memory, need realocation
		if ((s->str = (char*) realloc(s->str, s->uiAllocSize + (((uiStrLength / STR_INIT_LEN) + 1 ) * STR_INIT_LEN ) )) == NULL) {
        	return STR_ERROR;
        }
        s->uiAllocSize = s->uiAllocSize + (((uiStrLength / STR_INIT_LEN) + 1 ) * STR_INIT_LEN );
	}
	strcpy(&(s->str[s->uiLength]), str);
	s->uiLength = s->uiLength + uiStrLength;
	s->str[s->uiLength] = '\0';

	return STR_SUCCESS;
}

int32_t strCopyStr(dtStr *s1, const dtStr *s2) {
	//S1 <- S2 | S1 destination, S2 source
	if (s1 == NULL || s2 == NULL || s2->str == NULL) {
		return STR_ERROR;
	}
	uint32_t uiNewLength = s2->uiLength;
	if (uiNewLength >= s1->uiAllocSize) {
		//out of memory, need realloc
		if ((s1->str = (char *) realloc(s1->str, s2->uiAllocSize )) == NULL) {
			return STR_ERROR;
		}
		s1->uiAllocSize = s2->uiAllocSize;
	}
	strcpy(s1->str, s2->str);
	s1->uiLength = uiNewLength;
	return STR_SUCCESS;
}

dtStr *strNewFromStr(dtStr *s) {
	if (s == NULL) {
		return NULL;
	}
	dtStr *newStr = NULL;
	newStr = strNew();
	if (newStr == NULL) {
		return NULL;
	}
	if (strCopyStr(newStr, s) == STR_ERROR) {
		strFree(newStr);
		return NULL;
	}
	return newStr;
}

dtStr *strNewFromCStr(char *str) {
	if (str == NULL) {
		return NULL;
	}
	dtStr *newStr = NULL;
	newStr = strNew();
	if (newStr == NULL) {
		return NULL;
	}
	if (strAddCStr(newStr, str) == STR_ERROR) {
		strFree(newStr);
		return NULL;
	}
	return newStr;
}

int32_t strCmpStr(dtStr *s1, dtStr *s2) {
	if (s1 == NULL || s2 == NULL || s1->str == NULL || s2->str == NULL) {
		return STR_ERROR;
	}
	int32_t result = 0;
	result = strcmp(s1->str, s2->str);
	if (result > 0) {
		result = 1;
	}
	else if (result < 0) {
		result = -1;
	}
	return result;
}

int32_t strCmpCStr(dtStr *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL || s1->str == NULL) {
		return STR_ERROR;
	}
	int32_t result = 0;
	result = strcmp(s1->str, s2);
	if (result > 0) {
		result = 1;
	}
	else if (result < 0) {
		result = -1;
	}
	return result;
}

char *strGetCStr(dtStr *s) {
	if (s == NULL || s->str == NULL) {
		return NULL;
	}
	return s->str;
}

int32_t strGetLength(dtStr *s) {
	if (s == NULL) {
		return STR_ERROR;
	}
	return s->uiLength;
}

int32_t strCharPos(dtStr *s, const char c) {
	if (s == NULL) {
		return -1;
	}
	for (uint32_t i = 0; i < s->uiLength; i++)
	{
		if (s->str[i] == c) {
			return i;
		}
	}
	return -1;
}
