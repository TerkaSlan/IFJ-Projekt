/*
* This test is created for testing the scanner module
* by Terka Slaninakova
*/

#include "../scanner.h"
#include "test.h"

// TODO Keywords, another simple test mechanism: input: file, output: list of tokens, deal with scanner-test-invalid

void Test_errLex(char *inputToTest, int32_t retval, Token *token){
  printf("\nERR_LEX test: %s\n", inputToTest);
  SHOULD_EQUAL("ERR_LEX: retval", retval, ERR_LEX);
}

void Test_TT_string(char *inputToTest, TokenType tokenType, int32_t retval, Token *token, dtStr *string){
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

TEST_SUITE_START(ScannerTest);
if (openFile("scanner-test-file") != ERR_OK){
  return;
}

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

// '9aaaa' as an invalid input is spotted by syntactic analyzer. As far as scanner is concerned it's a number and an identifier
Test_TT_number("9aaaa -> 9", TT_number, getToken(token), token, 9);
Test_TT_id_fullId("aaaa", TT_identifier, getToken(token), token, string);


// '9_$' as an invalid input is spotted by syntactic analyzer. As far as scanner is concerned it's a number and an identifier
Test_TT_number("9_$ -> 9", TT_number, getToken(token), token, 9);
Test_TT_id_fullId("_$", TT_identifier, getToken(token), token, string);

Test_TT_number("0002 -> 2", TT_number, getToken(token), token, 2);
Test_TT_number("MAX_INT32 - 1", TT_number, getToken(token), token, 2147483647-1);
// TT_fullIdentifier
Test_TT_id_fullId("_name._name", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("_9999.$999", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("sample.s$mple", TT_fullIdentifier, getToken(token), token, string);
Test_TT_id_fullId("Upper.Upper", TT_fullIdentifier, getToken(token), token, string);

printf("\nA lot of space right now, then a simple comment, then block comment\n");

Test_TT_double("0.0", TT_double, getToken(token), token, 0.0);
Test_TT_double("2.13", TT_double, getToken(token), token, 2.13);
Test_TT_double("0.0E-0", TT_double, getToken(token), token, 0);
Test_TT_double("123.456e-2", TT_double, getToken(token), token, 12345.6);

Test_TT_string("\"\"", TT_string, getToken(token), token, string);
Test_TT_string("\".@#)(!*$#()@_!)\"", TT_string, getToken(token), token, string);

printf("\nEOF:\n");
int32_t retval = getToken(token);
SHOULD_EQUAL("TT_identifier: retval", retval, ERR_OK);
SHOULD_EQUAL("TT_identifier: type", token->type, TT_EOF);

strFree(string);
free(token);
TEST_SUITE_END
