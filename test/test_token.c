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
SHOULD_EQUAL("newToken() token->type", token->type, TT_empty);

// freeToken()
freeToken(&token);
SHOULD_EQUAL("freeToken(*token) token", token, NULL)

freeToken(NULL);

TEST_SUITE_END