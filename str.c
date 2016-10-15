
#include <stdio.h>				//pre test #SK
#include <string.h>				//for strcmp
#include <inttypes.h>			//for "uint32_t"
#include <stdlib.h>				//for malloc
#include "str.h"

#define STR_INIT_LEN 32			//how many bits is allocated in inicialisation
#define STR_ERROR   1
#define STR_SUCCESS 0


dtStr *strNew() {
	if ((dtStr *s = (dtStr*) malloc(sizeof(dtStr))) == NULL) {
		//!!!!!!!!!!!!!! ERROR (Nevieme co teraz) #SK
	}
	if (strInit(s) == STR_ERROR) {
		free(s);
		//!!!!!!!!!!!!!! ERROR (Nevieme co teraz) #SK
	}
	return s;
}

int32_t strInit(dtStr *s) {
	if (s == NULL) {
								// Doplnit error #SK
	}
	if ((s->str = (char*) malloc(STR_INIT_LEN)) == NULL) {
		return STR_ERROR;
	}
	s->str = '\0';
	s->uiLength = 0;
	s->uiAllocSize = STR_INIT_LEN;
	return STR_SUCCESS;
}


void strFree(dtStr *s) {
	if (s == NULL || s->str == NULL) {
								// Doplnit error #SK
	}
	else {
		free(s->str);
		s->uiLength = 0;			//nula sa dava aj pri init kde je ale \0 #SK
		s->uiAllocSize = 0;
	}
}


void strClear(dtStr *s) {
	if (s == NULL || s->str == NULL) {
								// Doplnit error #SK
	}
	s->str[0] = '\0';
	s->uiLength = 0;
}

int32_t strAddChar(dtStr *s, char c) {
	if (s == NULL || s->str == NULL) {
								// Doplnit error #SK
	}
	if (s->uiLength + 1 >= s->uiAllocSize) {		//Malo by sa to nanajvys rovnat, ale pre istotu...ok? #SK
		// pamet nestaci, je potreba provest realokaci #SK
    	if ((s->str = (char*) realloc(s->str, s->uiLength + STR_INIT_LEN)) == NULL)
        return STR_ERROR;
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
                        // Doplnit error #SK
   }
   uint32_t uiNewLength = s2->uiLength;
   if (uiNewLength >= s1->uiAllocSize) {
      //out of memory, need realloc
      if ((s1->str = (char*) realloc(s1->str, uiNewLength + 1)) == NULL) {
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
                        // Doplnit error #SK
	}
	return strcmp(s1->str, s2->str);
}

int32_t strCmpCStr(dtStr *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL || s1->str == NULL) {
                        // Doplnit error #SK
	}
	return strcmp(s1->str, s2);
}

char *strGetStr(dtStr *s) {
	if (s == NULL || s->str == NULL) {
						// Doplnit error #SK
	}
	return s->str;
}

uint32_t strGetLength(dtStr *s) {
	if (s == NULL || s->str == NULL) {
						// Doplnit error #SK
	}
	return s->uiLength;
}