
#ifndef STR_H
#define STR_H

#define STR_ERROR   1
#define STR_SUCCESS 0

typedef struct {
	char *str;					//array of chars which ends with '\0'
	uint32_t uiLength;			//actual length of string
	uint32_t uiAllocSize;		//size of allocated memory
} dtStr, *dtStrPtr;


/**
 * @brief      Creates and initialises new dtStr
 *
 * @return     Pointer to new dtStr string structure on heap
 */
dtStr *strNew();

/**
 * This function initializes dtStr structure (local) and makes it empty with '\0' at the end. NOTE: Use for local objects only! NOT RECOMMENDED, DEPRECATED
 *
 * @param      s     dtStr, structure representing string
 *
 * @return     STR_SUCCES or STR_ERROR according to initialization result
 */
int32_t strInit(dtStr *s);


/**
 * This function deinitializes dtStr structure (local) and makes it empty with '\0' at the end. NOTE: Use for local objects only! NOT RECOMMENDED, DEPRECATED
 *
 * @param      s     dtStr, structure representing string
 *
 * @return     STR_SUCCES or STR_ERROR according to initialization result
 */
void strDeinit(dtStr *s);


/**
 * @brief      This function deallocates memory allocated for dtStr s
 *
 * @param      s     dtStr, structure representing string
 */
void strFree(dtStr *s);

/**
 * @brief      Sets length of dtStr *s to 0 and first value in array of chars sets to '\0'
 *
 * @param      s     dtStr, structure representing string
 */
void strClear(dtStr *s);

/**
 * @brief      Adds char to the end of given dtStr
 *
 * @param      s    dtStr, structure representing string
 * @param[in]  c    char to be added
 *
 * @return     STR_SUCCES or STR_ERROR according to the result of adding char
 */
int32_t strAddChar(dtStr *s, char c);

/**
 * @brief      Adds given string to the end of s->str
 *
 * @param      s     dtStr, destination
 * @param      str   The string to be added
 *
 * @return     STR_SUCCES or STR_ERROR according to the result of adding string
 */
int32_t strAddCStr(dtStr *s, char *str);

/**
 * @brief      Copies s2 to s1
 *
 * @param      s1    dtStr, destination
 * @param      s2    dtStr, source
 *
 * @return     STR_SUCCES or STR_ERROR according to the success of copying
 */
int32_t strCopyStr(dtStr *s1, const dtStr *s2);

/**
 * @brief      Creates and initialises new dtStr and sets it to given s->str.
 *
 * @param      s     The dtStr, where s->str is source
 *
 * @return     Pointer to the new dtStr
 */
dtStr *strNewFromStr(dtStr *s);

/**
 * @brief      Creates and initialises new dtStr and sets it to given *str.
 *
 * @param      str   The source string
 *
 * @return     Pointer to the new dtStr
 */
dtStr *strNewFromCStr(char *str);

/**
 * @brief      compares the string pointed to, by s1->str to the string pointed to by s2->str
 * 
 *
 * @param      s1    dtStr
 * @param      s2    dtStr
 *
 * @return     if Return value < 0 then it indicates s1 is less than s2.
 *             if Return value > 0 then it indicates s2 is less than s1.
 *             if Return value = 0 then it indicates s1 is equal to s2.
 */
int32_t strCmpStr(dtStr *s1, dtStr *s2);

/**
 * @brief       compares the string pointed to, by s1->str to the string pointed to by s2
 *
 * @param      s1    dtStr
 * @param      s2    char *s2
 *
 * @return     if Return value < 0 then it indicates s1 is less than s2.
 *             if Return value > 0 then it indicates s2 is less than s1.
 *             if Return value = 0 then it indicates s1 is equal to s2.
 */
int32_t strCmpCStr(dtStr *s1, const char *s2);

/**
 * @param      s     dtStr
 *
 * @return     string
 */
char *strGetCStr(dtStr *s);

/**
 * @param      s     dtStr
 *
 * @return     Length of string
 */
uint32_t strGetLength(dtStr *s);

#endif