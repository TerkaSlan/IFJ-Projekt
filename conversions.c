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
  int32_t decimalNumber = 0;
  int32_t octLen = strGetLength(octalString);

  for(int32_t i = octLen-1; i >= 0 ; i--){
    if ((octalString->str[i] - '0') > 7)
      return INT_CONVERSION_ERROR;
    decimalNumber += (octalString->str[i] - '0') * pow(8, octLen - i - 1);
  }
  return decimalNumber;
}

int32_t stringToInt(const dtStr *string) {
  const char *stringData = string->str;
  while (isspace(*stringData))
      stringData++;

  // TODO: add support to unary minus (Bonus)
  if (*stringData == '-' || *stringData == '+')
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
