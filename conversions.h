/*
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


#ifndef CONVERSIONS_H
#define CONVERSIONS_H
#include <stdbool.h> // for fequal()
#include <float.h> // DBL_MAX_10_EXP, -DBL_MAX
#include "str.h"
#include "ial.h"
#include "error.h"
#include "token.h"

// TODO Improvement: Get rid of number contants denoting error and create a better mechanism so that 1 number from a given data type is not reserved as an error indicator and can be used
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

/**
* Converts binary number stored as a dtStr to integer
* @param    binaryString   String containing the binary number to be converted
* @return   Converted int
*/
int32_t binaryToInt(const dtStr *binaryString);

/**
* Converts hexadecimal number stored as a dtStr to integer
* @param    hexadecimalString   String containing the hexadecimal number to be converted
* @return   Converted int
*/
int32_t hexToInt(const dtStr *hexadecimalString);

/**
* Converts an integer to string (type dtStrPtr)
* @param    number   An int to-be converted
* @return   Converted string
*/
dtStrPtr intToString(int32_t number);

/**
* Converts a double to string (type dtStrPtr)
* @param    number   A double to-be converted
* @return   Converted string
*/
dtStrPtr doubleToString(double number);

/**
* Converts the data portion of symbol to an integer
* @param    symbol   Pointer to the symbol located in Symbol table
* @param    data     The actual data that need to be converted
* @return   Converted int, ERR_INTERN on failure
*/
int32_t *symbolToInt(const tSymbolPtr symbol, const tSymbolData *data, int32_t *convertedInt);

/**
* Converts the data portion of symbol to a double
* @param    symbol   Pointer to the symbol located in Symbol table
* @param    data     The actual data that need to be converted
* @return   Converted double, ERR_INTERN on failure
*/
double *symbolToDouble(const tSymbolPtr symbol, const tSymbolData *data, double *convertedDouble);

/**
* Converts the bool portion of symbol to a double
* @param    symbol   Pointer to the symbol located in Symbol table
* @param    data     The actual data that need to be converted
* @return   Converted bool, ERR_INTERN on failure
*/
bool *symbolToBool(const tSymbolPtr symbol, const tSymbolData *data, bool *convertedBool);

/**
* Converts the symbol to a newly created string (type dtStrPtr)
* @param    symbol   Pointer to the symbol located in Symbol table
* @param    data     The actual data that need to be converted
* @return   Converted string on success, NULL on failure
*/
dtStrPtr symbolToString(const tSymbolPtr symbol, const tSymbolData *data);

#endif
