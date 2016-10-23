/*
 * Testing module borrowed from https://github.com/metthal/IFJ-Projekt
 *
 * This test is created for testing token module
 * by Katarina Gresova
 */

#include "test.h"
#include "../token.h"

TEST_SUITE_START(TokenTest);

// newToken()
Token *token = newToken();
SHOULD_EQUAL("newToken() token->type == TT_empty", token->type, TT_empty);

// token with strings
token->type = TT_string;
strAddCStr(token->str, "string");

// freeToken() with strings
freeToken(&token);
SHOULD_EQUAL("freeToken(&token) with str", token, NULL);

// token with double
token = newToken();
token->dNum = 42.0;

// freeToken() with double
freeToken(&token);
SHOULD_EQUAL("freeToken(&token) with double", token, NULL);

// freeToken() with just greed token
freeToken(&token);
SHOULD_EQUAL("freeToken(&token) with just freed token", token, NULL);

// new token with string data
token = newToken();
token->type = TT_string;
strAddCStr(token->str, "string");
// just clean, able to use again
cleanToken(&token);
SHOULD_EQUAL("cleanToken(&token) token->type test", token->type, TT_empty);
SHOULD_EQUAL("cleanToken(&token) token->str test", token->str, NULL);

freeToken(&token);

// cleaning freed token
cleanToken(&token);
SHOULD_EQUAL("cleanToken(&token) with just freed token", token, NULL);

TEST_SUITE_END