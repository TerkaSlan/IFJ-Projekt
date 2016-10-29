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
// Failure expected
SHOULD_EQUAL("No support for unary minus (and negative ints)", integer, INT_CONVERSION_ERROR);

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

dbString = strNew();
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
TEST_SUITE_END
