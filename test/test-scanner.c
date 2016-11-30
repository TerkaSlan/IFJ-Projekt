/*
* This test is created for testing the scanner module
* by Terka Slaninakova
*/

#include "../scanner.h"
#include "test.h"

// for string to enum conversion
const static struct {
    KeywordTokenType   val;
    const char        *str;
}
conversion [] = {
    {KTT_boolean, "boolean"},
    {KTT_break, "break"},
    {KTT_class, "class"},
    {KTT_continue, "continue"},
    {KTT_do, "do"},
    {KTT_double, "double"},
    {KTT_else, "else"},
    {KTT_false, "false"},
    {KTT_for, "for"},
    {KTT_if, "if"},
    {KTT_int, "int"},
    {KTT_return, "return"},
    {KTT_String, "String"},
    {KTT_static, "static"},
    {KTT_true, "true"},
    {KTT_void, "void"},
    {KTT_while, "while"},
    {KTT_none, "none"}
};
bool isStringValidEnum (const char *str){
   int j;
   for (j = 0;  j < sizeof (conversion) / sizeof (conversion[0]);  ++j)
       if (strcmp (str, conversion[j].str) == 0){
           printf("KTT found for %s\n", str);
           return true;
        }
  return false;
}

void TestKeyword(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, dtStr *string){
  strAddCStr(string, inputToTest);
  printf("\nTT_keyword test: %s\n", inputToTest);
  SHOULD_EQUAL("TT_keyword: retval", retval, ERR_OK);
  SHOULD_EQUAL("TT_keyword: type", token->type, tokenType);
  SHOULD_EQUAL("TT_keyword: KTT", isStringValidEnum(string->str), true);
  strClear(string);
  cleanToken(&token);
}

void Test_TT_string(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, dtStr *string){
  if (strGetLength(string) == 0)
    strAddCStr(string, inputToTest);
  printf("\nTT_string test: %s\n", inputToTest);
  SHOULD_EQUAL("TT_string: retval", retval, ERR_OK);
  SHOULD_EQUAL("TT_string: type", token->type, tokenType);
  SHOULD_EQUAL("TT_string: str", strCmpStr(string, token->str), 0);
  strClear(string);
  cleanToken(&token);
}

void Test_TT_id_fullId(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, dtStr *string){
  strAddCStr(string, inputToTest);
  printf("\nTT_identifier / TT_fullIdentifier test: %s\n", inputToTest);
  SHOULD_EQUAL("TT_identifier: retval", retval, ERR_OK);
  SHOULD_EQUAL("TT_identifier: type", token->type, tokenType);
  SHOULD_EQUAL("TT_identifier: str", strCmpStr(string, token->str), 0);
  strClear(string);
  cleanToken(&token);
}

void Test_TT_number(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, int32_t iNum){
  printf("\nTT_number test: %s\n", inputToTest);
  SHOULD_EQUAL("TT_number: retval", retval, ERR_OK);
  SHOULD_EQUAL("TT_number: type", token->type, tokenType);
  SHOULD_EQUAL("TT_number: iNum", token->iNum, iNum);
  cleanToken(&token);
}

void Test_TT_double(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, double dNum){
  printf("\nTT_double test: %s\n", inputToTest);
  SHOULD_EQUAL("TT_double: retval", retval, ERR_OK);
  SHOULD_EQUAL("TT_double: type", token->type, tokenType);
  SHOULD_EQUAL("TT_double: dNum", token->dNum, dNum);
  cleanToken(&token);
}

void TestOperator(char *inputToTest, TokenType tokenType, int32_t retval, Token *token){
  printf("\ntest: %s\n", inputToTest);
  SHOULD_EQUAL("retval:", retval, ERR_OK);
  SHOULD_EQUAL("type:", token->type, tokenType);
  cleanToken(&token);
}

TEST_SUITE_START(ScannerTest);
if (openFile("scanner-test-valid") != ERR_OK){
  return;
}

//                  VALID TOKENS

Token *token = newToken();
dtStr *string = strNew();

Test_TT_id_fullId("_name", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("_9999", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("_____", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("$$$$$", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("$name", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("$_name", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("aaaaabbbbcccc", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("veeeeeeeeeeeeeeeeeeeeeery_loooooooooooooooooooooooooong", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("q", TT_identifier, getToken(token), token, string);

// Numbers
Test_TT_number("0002 -> 2", TT_number, getToken(token), token, 2);
// boundary check
Test_TT_number("MAX_INT32 - 1", TT_number, getToken(token), token, 2147483647-1);
// BONUS: Support for hexa numbers
Test_TT_number("0x1", TT_number, getToken(token), token, 1);
Test_TT_number("0xFF", TT_number, getToken(token), token, 255);
Test_TT_number("0xE1C", TT_number, getToken(token), token, 3612);
// BONUS: Support for binary numbers
Test_TT_number("0b010001101101", TT_number, getToken(token), token, 1133);
Test_TT_number("0b1111", TT_number, getToken(token), token, 15);
Test_TT_number("0b0", TT_number, getToken(token), token, 0);
// BONUS: Support for octal numbers
Test_TT_number("01", TT_number, getToken(token), token, 1);
Test_TT_number("037", TT_number, getToken(token), token, 31);
Test_TT_number("000000077", TT_number, getToken(token), token, 63);

TestOperator("-42 -> -", TT_minus, getToken(token), token);
Test_TT_number("-42 -> 42", TT_number, getToken(token), token, 42);
// TT_fullIdentifier
Test_TT_id_fullId("_name._name", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("_9999.$999", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("sample.s$mple", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("Upper.Upper", TT_fullIdentifier, getToken(token), token, string);

cleanToken(&token);
printf("\nA lot of space right now, then a simple comment, then block comment\n");
// Doubles
Test_TT_double("0.0", TT_double, getToken(token), token, 0.0);
Test_TT_double("2.13", TT_double, getToken(token), token, 2.13);
Test_TT_double("0.0E-0", TT_double, getToken(token), token, 0);
TestOperator("-0.0E-0 -> -", TT_minus, getToken(token), token);
Test_TT_double("-0.0E-0 -> 0.0E-0", TT_double, getToken(token), token, 0);
Test_TT_double("123.456e-2", TT_double, getToken(token), token, 12345.6);
TestOperator("-123.456e-2 -> -", TT_minus, getToken(token), token);
Test_TT_double("-123.456e-2 -> 123.456e-2", TT_double, getToken(token), token, 12345.6);
cleanToken(&token);
Test_TT_number("1_000", TT_number, getToken(token), token, 1000);
// Strings
Test_TT_string("", TT_string, getToken(token), token, string);
Test_TT_string(".@#)(!*$#()@_!)", TT_string, getToken(token), token, string);
strAddCStr(string, "Ahoj\nSve'te\\\"");
// Octal escape
Test_TT_string("Ahoj\nSve'te\\\042", TT_string, getToken(token), token, string);
strAddCStr(string, "Ahoj\nSve'te\t\"m");
Test_TT_string("Ahoj\nSve'te\t\"\155", TT_string, getToken(token), token, string);
Test_TT_string("terka", TT_string, getToken(token), token, string);
cleanToken(&token);
// Keywords
TestKeyword("break", TT_keyword, getToken(token), token, string);
Test_TT_id_fullId("brea", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("reak", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("sbreaks", TT_identifier, getToken(token), token, string);
Test_TT_id_fullId("continuedo", TT_identifier, getToken(token), token, string);
TestKeyword("do", TT_keyword, getToken(token), token, string);
TestKeyword("boolean", TT_keyword, getToken(token), token, string);
TestKeyword("class", TT_keyword, getToken(token), token, string);
TestKeyword("return", TT_keyword, getToken(token), token, string);
TestKeyword("String", TT_keyword, getToken(token), token, string);
Test_TT_id_fullId("string", TT_identifier, getToken(token), token, string);
TestOperator(";", TT_semicolon, getToken(token), token);
TestKeyword("while", TT_keyword, getToken(token), token, string);
TestKeyword("void", TT_keyword, getToken(token), token, string);
Test_TT_id_fullId("None", TT_identifier, getToken(token), token, string);

//operators + others
TestOperator("&&", TT_and, getToken(token), token);
TestOperator("||", TT_or, getToken(token), token);
TestOperator("+", TT_plus, getToken(token), token);
TestOperator("==", TT_equal, getToken(token), token);
TestOperator("=", TT_assignment, getToken(token), token);

TestOperator("{", TT_leftCurlyBracket, getToken(token), token);
TestOperator("(", TT_leftRoundBracket, getToken(token), token);
TestOperator("*", TT_multiply, getToken(token), token);
TestOperator("/", TT_divide, getToken(token), token);
TestOperator(")", TT_rightRoundBracket, getToken(token), token);
TestOperator("}", TT_rightCurlyBracket, getToken(token), token);
TestOperator("<", TT_less, getToken(token), token);
TestOperator("!", TT_not, getToken(token), token);
TestOperator(">", TT_greater, getToken(token), token);
TestOperator("!=", TT_notEqual, getToken(token), token);
TestOperator("++", TT_increment, getToken(token), token);
TestOperator("--", TT_decrement, getToken(token), token);
TestOperator("!", TT_not, getToken(token), token);
TestOperator("<=", TT_lessEqual, getToken(token), token);
TestOperator(">=", TT_greaterEqual, getToken(token), token);
TestOperator("-", TT_minus, getToken(token), token);
cleanToken(&token);

Test_TT_double("3.1_415", TT_double, getToken(token), token, 3.1415);
cleanToken(&token);

printf("\nEOF:\n");
int32_t retval = getToken(token);
SHOULD_EQUAL("TT_identifier: retval", retval, ERR_OK);
SHOULD_EQUAL("TT_identifier: type", token->type, TT_EOF);
cleanToken(&token);
closeFile();

//          INVALID TOKENS

if (openFile("scanner-test-invalid") != ERR_OK){
  return;
}

retval = getToken(token);
SHOULD_NOT_EQUAL("\n.1* not identifier",TT_identifier, token->type);
SHOULD_EQUAL("\n.1* ERR_LEX", retval, ERR_LEX);
getc(fSourceFile); // skipping over 1
getc(fSourceFile); // skipping over *

cleanToken(&token);
retval = getToken(token);
SHOULD_NOT_EQUAL("\nsample.0999 not identifier",TT_fullIdentifier, token->type);
SHOULD_EQUAL("\nsample.0999 ERR_LEX",retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_NOT_EQUAL("\n0_ not identifier",TT_fullIdentifier, token->type);
SHOULD_EQUAL("\n_ cannot be at the end of number", retval, ERR_LEX);
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n0xGG - invalid hexa", retval, ERR_LEX);
cleanToken(&token);
getc(fSourceFile); // skipping over G
getc(fSourceFile); // skipping over G
retval = getToken(token);
SHOULD_EQUAL("\nb1 as an id (not binary)", token->type, TT_identifier)
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n08 -> conversion error", retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n1233333333333333333333333333 ERR_LEX",retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n> ASCII", retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\nnon-octal in octal escape", retval, ERR_LEX);
// skipping over invalid octal
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n2digits in octal escape", retval, ERR_LEX);
// skipping over invalid octal
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n2digits in octal escape", retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n4digits in octal escape", retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n9_$ ", retval, ERR_LEX);
getc(fSourceFile);
getc(fSourceFile);
getc(fSourceFile);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n3_.1", retval, ERR_LEX);
//skipping over '1'
getc(fSourceFile);
getc(fSourceFile);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n3._1", retval, ERR_LEX);
//skipping over '1'
getc(fSourceFile);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n0x52_", retval, ERR_LEX);

cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n+++ -> ++", token->type, TT_increment);
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n+++ -> +", token->type, TT_plus);

retval = getToken(token);
SHOULD_EQUAL("\n--- -> --", token->type, TT_decrement);
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n--- -> -", token->type, TT_minus);
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n9aaaa not a valid combination", retval, ERR_LEX);
cleanToken(&token);
retval = getToken(token);
SHOULD_EQUAL("\n EOF", retval, ERR_OK);

strFree(string);
freeToken(&token);
closeFile();

TEST_SUITE_END
