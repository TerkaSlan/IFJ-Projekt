
#include <stdint.h>				//for "uint32_t"
#include "str.h"

#ifndef BUILTIN_H
#define BUILTIN_H



/**
 * 	Creates a new dtStr which contains substring of parameter s starting from beginIndex to endIndex -1.
 * 	WARNING - after using, must be freed returned dtStr.
 * 	Example s->str="abcde", substring(s,2,3) = "c"
 *
 * @param      s           dtStr
 * @param[in]  beginIndex  The begin index
 * @param[in]  endIndex    The end index
 *
 * @return     Pointer to a new dtStr, or NULL if something went wrong.
 */
dtStr *substr(dtStr *s, uint32_t beginIndex, uint32_t endIndex);

#endif
