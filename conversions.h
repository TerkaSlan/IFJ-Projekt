#ifndef CONVERSIONS_H
#define CONVERSIONS_H
#include <stdbool.h> // for fequal()
#include <float.h> // for -DBL_MAX
#include "str.h"

#define DOUBLE_CONVERSION_ERROR -DBL_MAX
#define INT_CONVERSION_ERROR    INT32_MIN

/**
* Converts incoming string to 32-bit integer
* @param    string    String that'll be converted
* @return   Converted integer in case of success, INT_CONVERSION_ERROR in case of failure
*/
int32_t stringToInt(const dtStr *string);

/**
* Converts incoming string to double
* @param    string    String that'll be converted
* @return   Converted double in case of success, DOUBLE_CONVERSION_ERROR in case of failure
*/
double stringToDouble(const dtStr *string);

/**
* Determines whether 2 doubles are equal
* @param  a   first double
* @param  b   second double
* @return     Truth value of the comparison
*/
bool fequal(double a, double b);

/**
* Converts octal number stored as a dtStr to integer
* @param    octalString   String containing the octal number to be converted
* @return   Converted int
*/
int32_t octalToInt(dtStr *octalString);
#endif
