
typedef struct {
	char* str;					//array of chars which ends with '\0'
	uint32_t uiLength;			//actual length of string
	uint32_t uiAllocSize;		//size of allocated memory
} dtStr;

/**
 * This function allocates space for string and makes empty string with '\0' at the end
 *
 * @param      s     dtStr, structure representing string
 *
 * @return     STR_SUCCES or STR_ERROR according to initialisation result
 */
int32_t strInit(dtStr *s);

/**
 * @brief      This function deallocates memory allocated for string
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
 * @brief      Copies s2 to s1
 *
 * @param      s1    dtStr, destination
 * @param      s2    dtStr, source
 *
 * @return     STR_SUCCES or STR_ERROR according to the success of copying
 */
int32_t strCopyString(dtStr *s1, dtStr *s2);

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
int32_t strCmpString(dtStr *s1, dtStr *s2);

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
int32_t strCmpConstStr(dtStr *s1, char *s2);

/**
 * @param      s     dtStr
 *
 * @return     string
 */
char *strGetStr(dtStr *s);

/**
 * @param      s     dtStr
 *
 * @return     Length of string
 */
uint32_t strGetLength(dtStr *s);