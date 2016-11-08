#include <stdlib.h>
#include <ctype.h>
#include <math.h> // for fabs()
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
