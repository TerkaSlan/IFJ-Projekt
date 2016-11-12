/*
* This test is created for testing conversion module
* by Terka Slaninakova
*/

#include "../conversions.h"
#include "test.h"

TEST_SUITE_START(ConversionsTest);

// Precision:  0.00000001
dtStr *dbString = strNew();
strAddCStr(dbString, "3.14");
double db = stringToDouble(dbString);
double db2 = 3.14000001;
SHOULD_EQUAL("Simple double", fequal(db, db2), 1); //true
db2 = 3.1400001;
SHOULD_EQUAL("Simple double", fequal(db, db2), 0); //false

// INTS
dtStr *string = strNew();
strAddCStr(string, "12234"); // Basic int
int32_t integer = stringToInt(string);
SHOULD_EQUAL("Simple integer", integer, 12234);

strClear(string);
strAddCStr(string, "+12234");
integer = stringToInt(string);
// Failure expected
SHOULD_EQUAL("No support for unary plus", integer, INT_CONVERSION_ERROR);

// Note: Unary minus support is a Bonus feature that'll be implemented in the future
strClear(string);
strAddCStr(string, "-12234");
integer = stringToInt(string);
SHOULD_NOT_EQUAL("Support for unary minus", integer, INT_CONVERSION_ERROR);

strClear(string);
strAddCStr(string, "2222222222222222222222222222222222"); // Very long int
integer = stringToInt(string);
// Failure expected
SHOULD_EQUAL("Int overflow", integer, INT_CONVERSION_ERROR);

strClear(string);
strAddCStr(string, "0");
integer = stringToInt(string);
SHOULD_EQUAL("Zero", integer, 0);
SHOULD_NOT_EQUAL("Zero", integer, INT_CONVERSION_ERROR);

// DOUBLES

strClear(dbString);
strAddCStr(dbString, "3.14");
db = stringToDouble(dbString);
// One does not simply rely on == with doubles, using my double comparison function
SHOULD_EQUAL("Simple double", fequal(db, 3.14), 1);

strClear(dbString);
strAddCStr(dbString, "0.14");
db = stringToDouble(dbString);
SHOULD_EQUAL("0. double", fequal(db, 0.14), 1);

strClear(dbString);
strAddCStr(dbString, "0.0");
db = stringToDouble(dbString);
SHOULD_EQUAL("0.0 double", fequal(db, 0.0), 1);

strClear(dbString);
strAddCStr(dbString, "0");
db = stringToDouble(dbString);
SHOULD_EQUAL("No decimal point", fequal(db, 0.0), 1);

strClear(dbString);
strAddCStr(dbString, "0.0E0");
db = stringToDouble(dbString);
SHOULD_EQUAL("Zero exponent", fequal(db, 0.0), 1);

strClear(dbString);
strAddCStr(dbString, "0.0E-0");
db = stringToDouble(dbString);
SHOULD_EQUAL("Zero negative exponent", fequal(db, 0.0), 1);

strClear(dbString);
strAddCStr(dbString, "0.0E+-0");
db = stringToDouble(dbString);
// Failure expected
SHOULD_EQUAL("+- exponent", fequal(db, DOUBLE_CONVERSION_ERROR), 1);

strClear(dbString);
strAddCStr(dbString, "0.0E--0");
db = stringToDouble(dbString);
// Failure expected
SHOULD_EQUAL("-- exponent", fequal(db, DOUBLE_CONVERSION_ERROR), 1);

strClear(dbString);
strAddCStr(dbString, ".0E0");
db = stringToDouble(dbString);
// Failure expected
SHOULD_EQUAL(". as the first symbol", fequal(db, DOUBLE_CONVERSION_ERROR), 1);

strClear(dbString);
strAddCStr(dbString, "+0");
db = stringToDouble(dbString);
// Failure expected
SHOULD_EQUAL("+ as the first symbol", fequal(db, DOUBLE_CONVERSION_ERROR), 1);

strClear(dbString);
strAddCStr(dbString, "E4");
db = stringToDouble(dbString);
// Failure expected
SHOULD_EQUAL("E as the first symbol", fequal(db, DOUBLE_CONVERSION_ERROR), 1);

strClear(dbString);
strAddCStr(dbString, "123.456e2");
db = stringToDouble(dbString);
SHOULD_EQUAL("lowercase e", fequal(db, 12345.6), 1);

strClear(dbString);
strAddCStr(dbString, "123.456e-2");
db = stringToDouble(dbString);
SHOULD_EQUAL("lowercase e -", fequal(db, 1.23456), 1);

strClear(dbString);
strAddCStr(dbString, "12322222222222222222222222222222222.456e-2");
db = stringToDouble(dbString);
// Failure NOT expected even though the value is incorrect - will behave as a double in C
SHOULD_EQUAL("Whole part overflow", fequal(db, 12322222222222222222222222222222222.456e-2), 1);

strClear(dbString);
strAddCStr(dbString, "12322222222.222222222222222222222222222222456e-2");
db = stringToDouble(dbString);
SHOULD_EQUAL("Decimal part overflow", fequal(db, 123222222.222222222222), 1);

strClear(dbString); // zeros in whole part and exponent ignored
strAddCStr(dbString, "00001232002.00034e0003"); //  1232002000.340000
db = stringToDouble(dbString);
SHOULD_EQUAL("Decimal part overflow", fequal(db, 1232002000.34), 1);

strFree(dbString);
strFree(string);

// octalToInt
dtStr *dbStringOctal = strNew();
strAddCStr(dbStringOctal, "000"); // NULL
SHOULD_EQUAL("Empty string", (unsigned char)octalToInt(dbStringOctal), 0);
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "011"); // tab
SHOULD_EQUAL("Tab", (unsigned char)octalToInt(dbStringOctal), '\t');
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "040"); // space
SHOULD_EQUAL("Space", (unsigned char)octalToInt(dbStringOctal), ' ');
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "044"); // $
SHOULD_EQUAL("Dollar sign", (unsigned char)octalToInt(dbStringOctal), '$');
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "057"); // /
SHOULD_EQUAL("Slash", (unsigned char)octalToInt(dbStringOctal), '/');
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "160"); // symbol
SHOULD_EQUAL("p", (unsigned char)octalToInt(dbStringOctal), 'p');
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "200");
SHOULD_EQUAL("Extended ASCII: (200)8->(128)10", (unsigned char)octalToInt(dbStringOctal), 128);
strClear(dbStringOctal);
strAddCStr(dbStringOctal, "888"); // not valid
SHOULD_EQUAL("Not valid", octalToInt(dbStringOctal), INT_CONVERSION_ERROR);
strFree(dbStringOctal);

// [#16]
tSymbolPtr symbol = symbolNew();
// symbolToInt
int32_t IntForConversion;
symbol->Type = eINT;
symbol->Data.Integer = 2;
SHOULD_EQUAL("symbolToInt() Simple symbol to int", *symbolToInt(symbol, &IntForConversion), 2);
symbol->Type = 100;
SHOULD_EQUAL("symbolToInt() Unknown eTYPE", symbolToInt(symbol, &IntForConversion), NULL);
symbol->Type = eDOUBLE;
symbol->Data.Double = -123.456e-2;
SHOULD_EQUAL("symbolToInt() Simple double", *symbolToInt(symbol, &IntForConversion), -1);
symbol->Type = eBOOL;
symbol->Data.Bool = false;
SHOULD_EQUAL("symbolToInt() Simple double", *symbolToInt(symbol, &IntForConversion), 0);
symbol->Type = eSTRING;
symbol->Data.String = strNewFromCStr("-1231");
SHOULD_EQUAL("symbolToInt() Simple string", *symbolToInt(symbol, &IntForConversion), -1231);
strFree(symbol->Data.String);
// symbolToDouble
double DoubleForConversion;
symbol->Type = eDOUBLE;
symbol->Data.Double = 123.456e2;
SHOULD_EQUAL("symbolToDouble() Symbol with 'e'", *symbolToDouble(symbol, &DoubleForConversion), 12345.6);
symbol->Data.Double = -123.456e-2;
SHOULD_EQUAL("symbolToDouble() Symbol with '-' and 'e'", *symbolToDouble(symbol, &DoubleForConversion), -1.23456);
symbol->Data.Double = 0;
SHOULD_EQUAL("symbolToDouble() Symbol with 0", *symbolToDouble(symbol, &DoubleForConversion), 0.0);
symbol->Type = 100;
SHOULD_EQUAL("symbolToInt() Unknown eTYPE", symbolToDouble(symbol, &DoubleForConversion), NULL);
symbol->Type = eINT;
symbol->Data.Integer = -2147483647;
SHOULD_EQUAL("symbolToDouble() INT_MIN", *symbolToDouble(symbol, &DoubleForConversion), -2147483647.0);
symbol->Type = eBOOL;
symbol->Data.Bool = true;
SHOULD_EQUAL("symbolToDouble() INT_MIN", *symbolToDouble(symbol, &DoubleForConversion), 1.0);
symbol->Type = eSTRING;
symbol->Data.String = strNewFromCStr("-1231.1e-2");
SHOULD_EQUAL("symbolToInt() Simple string", *symbolToDouble(symbol, &DoubleForConversion), -12.311);
strFree(symbol->Data.String);
// symbolToBool
bool BoolForConversion;
symbol->Type = eBOOL;
symbol->Data.Bool = true;
SHOULD_EQUAL("symbolToBool() True", *symbolToBool(symbol, &BoolForConversion), true);
symbol->Data.Bool = false;
SHOULD_EQUAL("symbolToBool() False", *symbolToBool(symbol, &BoolForConversion), false);
symbol->Type = 100;
SHOULD_EQUAL("symbolToInt() Unknown eTYPE", symbolToBool(symbol, &BoolForConversion), NULL);
symbol->Type = eDOUBLE;
symbol->Data.Double = -123.456e-2;
SHOULD_EQUAL("symbolToBool() Double", *symbolToBool(symbol, &BoolForConversion), true);
symbol->Data.Double = 0.0;
SHOULD_EQUAL("symbolToBool() Double", *symbolToBool(symbol, &BoolForConversion), false);
symbol->Type = eINT;
symbol->Data.Integer = 2147483647;
SHOULD_EQUAL("symbolToBool() Int", *symbolToBool(symbol, &BoolForConversion), true);
symbol->Data.Integer = 0;
SHOULD_EQUAL("symbolToBool() Int", *symbolToBool(symbol, &BoolForConversion), false);
symbol->Type = eSTRING;
symbol->Data.String = strNew();
SHOULD_EQUAL("symbolToBool() String", *symbolToBool(symbol, &BoolForConversion), false);
strFree(symbol->Data.String);
// toString
symbol->Type = eSTRING;
symbol->Data.String = strNewFromCStr("12");
dtStrPtr returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() STRING simple", strcmp(returnedString->str, "12"), 0);
strClear(symbol->Data.String);
strFree(returnedString);

strAddCStr(symbol->Data.String, "-}?{>!@:!@#>?>A{dwianaaaaaaaaaaaaaaaaa}}");
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() STRING junk", strcmp(returnedString->str, "-}?{>!@:!@#>?>A{dwianaaaaaaaaaaaaaaaaa}}"), 0);
strFree(symbol->Data.String);
strFree(returnedString);

symbol->Type = eBOOL;
symbol->Data.Bool = true;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() BOOL true", strcmp(returnedString->str, "1"), 0);
strFree(returnedString);

symbol->Data.Bool = false;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() BOOL false", strcmp(returnedString->str, "0"), 0);
strFree(returnedString);

symbol->Type = eNULL;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() eNULL", returnedString->uiLength, 0);
strFree(returnedString);

symbol->Type = eDOUBLE;
symbol->Data.Double = -12.982301;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() eDOUBLE negative", strcmp(returnedString->str, "-12.982301"), 0);
strFree(returnedString);

symbol->Data.Double = -123.456e-2;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() eDOUBLE negative with e", strcmp(returnedString->str, "-1.234560"), 0);
strFree(returnedString);

symbol->Type = eINT;
symbol->Data.Integer = 2147483647;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() eINT INT_MAX", strcmp(returnedString->str, "2147483647"), 0);
strFree(returnedString);

symbol->Type = eINT;
symbol->Data.Integer = -2147483647;
returnedString = symbolToString(symbol);
SHOULD_EQUAL("symbolToString() eINT INT_MIN", strcmp(returnedString->str, "-2147483647"), 0);

symbol->Type = 100;
SHOULD_EQUAL("symbolToInt() Unknown eTYPE", symbolToString(symbol), NULL);

strFree(returnedString);
//strFree(symbol->Data.String);
symbolFree(symbol);
TEST_SUITE_END
