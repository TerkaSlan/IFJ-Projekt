
#include <stdint.h>				//for "uint32_t"
#include "str.h"
#include "error.h"

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
 * @param      subStr      New substring (uses strNew())
 *
 * @return     Error values
 */
eError substr(const dtStr *s, int32_t beginIndex, int32_t endIndex, dtStrPtr *subStr);

#endif
