#include <stdlib.h>
#include <ctype.h>
#include <math.h> // fabs()
#include <stdio.h> // sprintf()
#include "conversions.h"

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

int32_t hexToInt(const dtStr *hexadecimalString) {
  char* hexString = &(hexadecimalString->str[2]);
  int32_t hexLen = hexadecimalString->uiLength - 2;
  int32_t decimalNumber = 0;
  for(int32_t i = hexLen-1; i >= 0 ; i--){
    if (isdigit(hexString[i]) || ((hexString[i] >= 'A') && (hexString[i] <= 'F'))){
      if(isdigit(hexString[i]))
        decimalNumber += (hexString[i] - '0') * pow(16, hexLen - i - 1);
      else{
        // 'A' is 65 in ASCII table, getting 10 out of it this way, similar for B, ..., F
        decimalNumber += (hexString[i] - 55) * pow(16, hexLen - i - 1);
      }
    }
    else{
      return INT_CONVERSION_ERROR;
    }
  }
  return decimalNumber;
}

int32_t stringToInt(const dtStr *string) {
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
    dtStrPtr string = strNew();
    strAddCStr(string, temp);
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
  if (sprintf(temp, "%lf", number) > 0){
    dtStrPtr string = strNew();
    strAddCStr(string, temp);
    return string;
  }
  else{
    return NULL;
  }
}

double stringToDouble(const dtStr *string) {
  uint8_t convertState = DSStart;
  for (uint32_t counter = 0; counter <= string->uiLength; counter++){
    int8_t iCurrentSymbol = string->str[counter];
    switch (convertState) {
      case DSStart: {
        if (isdigit(iCurrentSymbol) || iCurrentSymbol == '-')
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
          return strtod(string->str, NULL);
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
          return strtod(string->str, NULL);
        }
        break;
      }
      case DSExponent: {
        if (iCurrentSymbol == '+' || iCurrentSymbol == '-')
          convertState = DSExponentSign;
        else if (isdigit(iCurrentSymbol))
          return strtod(string->str, NULL);
        else{
          return DOUBLE_CONVERSION_ERROR;
        }
        break;
      }
      case DSExponentSign: {
        if (isdigit(iCurrentSymbol))
          return strtod(string->str, NULL);
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

int32_t *symbolToInt(const tSymbolPtr symbol, int32_t *convertedInt) {
  switch (symbol->Type) {
    case eNULL:
      *convertedInt = 0;
      break;
    case eINT:
      *convertedInt = symbol->Data.Integer;
      break;
    case eDOUBLE:
      *convertedInt = (int32_t)symbol->Data.Double;
      break;
    case eBOOL:
      *convertedInt = symbol->Data.Bool;
      break;
    case eSTRING:
      *convertedInt = stringToInt(symbol->Data.String);
      break;
    default:
      convertedInt = NULL;
  }
  return convertedInt;
}

double *symbolToDouble(const tSymbolPtr symbol, double *convertedDouble) {
  switch (symbol->Type) {
    case eNULL:
      *convertedDouble = 0.0;
      break;
    case eINT:
      *convertedDouble = (double)symbol->Data.Integer;
      break;
    case eDOUBLE:
      *convertedDouble = symbol->Data.Double;
      break;
    case eBOOL:
      *convertedDouble = (double)symbol->Data.Bool;
      break;
    case eSTRING:
      *convertedDouble = stringToDouble(symbol->Data.String);
      break;
    default:
      convertedDouble = NULL;
  }
  return convertedDouble;
}

bool *symbolToBool(const tSymbolPtr symbol, bool *convertedBool) {
  switch (symbol->Type) {
    case eNULL:
      *convertedBool = false;
      break;
    case eINT:
      *convertedBool = symbol->Data.Integer != 0;
      break;
    case eDOUBLE:
      *convertedBool = symbol->Data.Double != 0.0;
      break;
    case eBOOL:
      *convertedBool = symbol->Data.Bool;
      break;
    case eSTRING:
      *convertedBool = symbol->Data.String->uiLength != 0;
      break;
    default:
      convertedBool = NULL;
  }
  return convertedBool;
}

dtStrPtr symbolToString(const tSymbolPtr symbol) {
  dtStrPtr string = NULL;
  switch (symbol->Type) {
    case eNULL:
      string = strNew();
      break;
    case eINT:
      if ((string = intToString(symbol->Data.Integer)) == NULL)
        handleConversionError(string);
      break;
    case eDOUBLE:
      if ((string = doubleToString(symbol->Data.Double)) == NULL)
        handleConversionError(string);
      break;
    case eBOOL:
      string = strNew();
      if (strAddChar(string,symbol->Data.Bool + '0') == STR_ERROR)
        handleConversionError(string);
      break;
    case eSTRING:
      string = strNew();
      if(strCopyStr(string, symbol->Data.String) == STR_ERROR)
        handleConversionError(string);
      break;
    default:
      break;
  }
  return string;
}
