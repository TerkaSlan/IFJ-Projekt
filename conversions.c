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

#include <stdlib.h>
#include <ctype.h>
#include <math.h> // fabs()
#include <stdio.h> // sprintf()
#include "conversions.h"
#include "builtin.h"

typedef enum {
  DSStart,
  DSWholePart,
  DSExponent,
  DSExponentSign,
  DSDecimalPart,
  DSPoint
} tDoubleState;

bool fequal(double a, double b) {
    double epsilon = 0.00000001;
    return fabs(a-b) < epsilon;
}

#define handleConversionError(string)                                   \
do {                                                                    \
    if (string != NULL)   strFree(string);                              \
    return NULL;                                                        \
} while (0)

eError convertHexToDecimal(char *hexString, int32_t hexLen, int32_t *decimalNumber){
  int32_t dotPresent = 0;
  for(int32_t i = hexLen-1; i >= 0 ; i--){
    // Hack: I need to adjust 2nd arg in pow if dot is present so that i won't fuck up my calculations
    if (hexString[i] == '.'){
      dotPresent = 1;
      continue;
    }
    if (isdigit(hexString[i]) || ((hexString[i] >= 'A') && (hexString[i] <= 'F'))){
      if(isdigit(hexString[i]))
        *decimalNumber += (hexString[i] - '0') * pow(16, hexLen - i - dotPresent - 1);
      else{
        // 'A' is 65 in ASCII table, getting 10 out of it this way, similar for B, ..., F
        *decimalNumber += (hexString[i] - 55) * pow(16, hexLen - i - dotPresent - 1);
      }
    }
    else{
      return ERR_INTERN;
    }
  }
  return ERR_OK;
}

int32_t octalToInt(dtStr *octalString){
  char *octString;
  int32_t octLen;
  /* have to specifically check for 0 as the first char, since this
     function is used in constants (0 as first char)
     as well as escape sequences (0-3 as first char) */
  if (strCharPos(octalString, '0') == 0){
    octString = &(octalString->str[1]);
    octLen = octalString->uiLength - 1;
  }
  else{
    octString = octalString->str;
    octLen = octalString->uiLength;
  }

  int32_t decimalNumber = 0;
  for(int32_t i = octLen-1; i >= 0 ; i--){
    if ((octString[i] - '0') > 7)
      return INT_CONVERSION_ERROR;
    decimalNumber += (octString[i] - '0') * pow(8, octLen - i - 1);
  }
  return decimalNumber;
}

int32_t binaryToInt(const dtStr *binaryString) {
  // skipping over '0b'
  char* binString = &(binaryString->str[2]);
  int32_t binLen = binaryString->uiLength - 2;
  int32_t decimalNumber = 0;
  for(int32_t i = binLen-1; i >= 0 ; i--){
    if ((binString[i] - '0') > 1 || (binString[i] - '0') < 0)
      return INT_CONVERSION_ERROR;
    decimalNumber += (binString[i] - '0') * pow(2, binLen - i - 1);
  }
  return decimalNumber;
}

int32_t hexToInt(dtStr *hexadecimalString) {
  int32_t decimalNumber = 0;
  if (strCharPos(hexadecimalString, '.') != (-1))
    return INT_CONVERSION_ERROR;
  if (convertHexToDecimal(&(hexadecimalString->str[2]), hexadecimalString->uiLength - 2, &decimalNumber) == ERR_INTERN)
    return INT_CONVERSION_ERROR;
  return decimalNumber;
}

double hexToDouble(dtStr *hexDoubleString){
  // 0xFF.FFp-1 je 65535 / 16^2 * 2^(-1)
  int32_t p_position = strCharPos(hexDoubleString, 'p') == (-1) ? strCharPos(hexDoubleString, 'P') : strCharPos(hexDoubleString, 'p');
  dtStrPtr upToExponentPart;
  if(substr(hexDoubleString, 2, p_position - 2, &upToExponentPart) == ERR_INTERN){
    strFree(upToExponentPart);
    return DOUBLE_CONVERSION_ERROR;
  }
  int32_t decimalNumber = 0;
  int32_t dot_position = strCharPos(hexDoubleString, '.');
  if (convertHexToDecimal(&(upToExponentPart->str[0]), upToExponentPart->uiLength, &decimalNumber) == ERR_INTERN){
    strFree(upToExponentPart);
    return DOUBLE_CONVERSION_ERROR;
  }
  int32_t fromDotToP;
  fromDotToP = (dot_position == (-1)) ? 0 : p_position - dot_position - 1;
  dtStrPtr exponentString;
  int32_t exponent = 0 ;
  if(substr(hexDoubleString, p_position + 1, hexDoubleString->uiLength - p_position - 1, &exponentString) == ERR_INTERN){
    strFree(exponentString);
    strFree(upToExponentPart);
    return DOUBLE_CONVERSION_ERROR;
  }
  if ((exponent = stringToInt(exponentString)) == INT_CONVERSION_ERROR){
    strFree(exponentString);
    strFree(upToExponentPart);
    return DOUBLE_CONVERSION_ERROR;
  }
  double result = (decimalNumber/pow(16, fromDotToP)) * pow(2, exponent);
  strFree(exponentString);
  strFree(upToExponentPart);
  return result;
}

int32_t stringToInt(dtStr *string) {
  if (strCharPos(string, '0') == 0 && strCharPos(string, 'x') == 1)
    return hexToInt(string);
  if (strCharPos(string, '0') == 0 && strCharPos(string, 'b') == 1)
    return binaryToInt(string);
  if (strCharPos(string, '0') == 0 && string->uiLength > 1)
    return octalToInt(string);
  const char *stringData = string->str;
  while (isspace(*stringData))
      stringData++;

  if (*stringData == '+')
    return INT_CONVERSION_ERROR;
  // checking char pointer if the conversion fails
  char *end;
  long tempLong = strtol(stringData, &end, 10);
  if (tempLong > INT32_MAX || *end != '\0')
    return INT_CONVERSION_ERROR;

  int32_t newInt = (int32_t)tempLong;
  return newInt;
}

dtStrPtr intToString(int32_t number) {
  // int32 has 32 bytes - 10 for digits, 1 for sign, 1 for null terminator
  char temp[12];
  if (sprintf(temp, "%d", number) > 0){
    dtStrPtr string = strNewFromCStr(temp);
    return string;
  }
  else{
    return NULL;
  }
}

dtStrPtr doubleToString(double number) {
  /*DBL_MAX_10_EXP = the largest power-of-10 exponent needed to represent
                     all double values*/
  char temp[DBL_MAX_10_EXP + 2];
  if (sprintf(temp, "%g", number) > 0){
    dtStrPtr string = strNewFromCStr(temp);
    return string;
  }
  else{
    return NULL;
  }
}

double stringToDouble(dtStr *string) {
  if (strCharPos(string, '0') == 0 && strCharPos(string, 'x') == 1){
    if (strCharPos(string, 'p') == (-1) && strCharPos(string, 'P') == (-1))
      return DOUBLE_CONVERSION_ERROR;
    return hexToDouble(string);
  }
  uint8_t convertState = DSStart;
  for (uint32_t counter = 0; counter <= string->uiLength; counter++){
    int8_t iCurrentSymbol = string->str[counter];
    switch (convertState) {
      case DSStart: {
        if (isdigit(iCurrentSymbol))
          convertState = DSWholePart;
        else if (isspace(iCurrentSymbol))
          ;
        else{
          return DOUBLE_CONVERSION_ERROR;
        }
        break;
      }
      case DSWholePart: {
        if (iCurrentSymbol == '.')
          convertState = DSPoint;
        else if (isdigit(iCurrentSymbol))
          ;
        else if (iCurrentSymbol == 'e' || iCurrentSymbol == 'E')
          convertState = DSExponent;
        else{
          char *end;
          double tempDouble = strtod(string->str, &end);
          if (*end != '\0')
            return DOUBLE_CONVERSION_ERROR;
          return tempDouble;
        }
        break;
      }
      case DSPoint: {
        if (isdigit(iCurrentSymbol))
          convertState = DSDecimalPart;
        else{
          return DOUBLE_CONVERSION_ERROR;
        }
        break;
      }
      case DSDecimalPart: {
        if (iCurrentSymbol == 'e' || iCurrentSymbol == 'E')
          convertState = DSExponent;
        else if (isdigit(iCurrentSymbol))
          ;
        else{
          char *end;
          double tempDouble = strtod(string->str, &end);
          if (*end != '\0')
            return DOUBLE_CONVERSION_ERROR;
          return tempDouble;
        }
        break;
      }
      case DSExponent: {
        if (iCurrentSymbol == '+')
          convertState = DSExponentSign;
        else if (isdigit(iCurrentSymbol)){
          char *end;
          double tempDouble = strtod(string->str, &end);
          if (*end != '\0')
            return DOUBLE_CONVERSION_ERROR;
          return tempDouble;
        }
        else{
          return DOUBLE_CONVERSION_ERROR;
        }
        break;
      }
      case DSExponentSign: {
        if (isdigit(iCurrentSymbol)){
          char *end;
          double tempDouble = strtod(string->str, &end);
          if (*end != '\0')
            return DOUBLE_CONVERSION_ERROR;
          return tempDouble;
        }
        else{
          return DOUBLE_CONVERSION_ERROR;
        }
        break;
      }
      default: {
        return DOUBLE_CONVERSION_ERROR;
      }
    }
  }
  return DOUBLE_CONVERSION_ERROR;
}

int32_t *symbolToInt(const tSymbolPtr symbol, const tSymbolData *data, int32_t *convertedInt) {
  switch (symbol->Type) {
    case eINT:
      *convertedInt = data->Integer;
      break;
    case eDOUBLE:
      *convertedInt = (int32_t)data->Double;
      break;
    case eBOOL:
      *convertedInt = data->Bool;
      break;
    case eSTRING:
      *convertedInt = stringToInt(data->String);
      break;
    default:
      convertedInt = NULL;
  }
  return convertedInt;
}

double *symbolToDouble(const tSymbolPtr symbol, const tSymbolData *data, double *convertedDouble) {
  switch (symbol->Type) {
    case eINT:
      *convertedDouble = (double)data->Integer;
      break;
    case eDOUBLE:
      *convertedDouble = data->Double;
      break;
    case eBOOL:
      *convertedDouble = (double)data->Bool;
      break;
    case eSTRING:
      *convertedDouble = stringToDouble(data->String);
      break;
    default:
      convertedDouble = NULL;
  }
  return convertedDouble;
}

bool *symbolToBool(const tSymbolPtr symbol, const tSymbolData *data, bool *convertedBool) {
  switch (symbol->Type) {
    case eINT:
      *convertedBool = data->Integer != 0;
      break;
    case eDOUBLE:
      *convertedBool = data->Double != 0.0;
      break;
    case eBOOL:
      *convertedBool = data->Bool;
      break;
    case eSTRING:
      *convertedBool = data->String->uiLength != 0;
      break;
    default:
      convertedBool = NULL;
  }
  return convertedBool;
}

dtStrPtr symbolToString(const tSymbolPtr symbol, const tSymbolData *data) {
  dtStrPtr string = NULL;
  switch (symbol->Type) {
    case eINT:
      if ((string = intToString(data->Integer)) == NULL)
        handleConversionError(string);
      break;
    case eDOUBLE:
      if ((string = doubleToString(data->Double)) == NULL)
        handleConversionError(string);
      break;
    case eBOOL:
      if ((string = strNewFromCStr(data->Bool ? "true" : "false")) == NULL)
        handleConversionError(string);
      break;
    case eSTRING:
      string = strNew();
      if(strCopyStr(string, data->String) == STR_ERROR)
        handleConversionError(string);
      break;
    default:
      break;
  }
  return string;
}
