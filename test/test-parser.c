/**
 * Testing module borrowed from https://github.com/metthal/IFJ-Projekt
 *
 * This test is created for testing expresion parsing module
 * by Katarina Gresova
 */

// gdb -q --command=test.gdb --args ./test

#include "../expr.h"
#include "../scanner.h"
#include "../parser.h"
#include "../parser_SecondRun.h"
#include "../token.h"
#include "../interpret.h"
#include "test.h"

TEST_SUITE_START(ParserTest);
/*
tPrecedenceStackPtr stack = precedenceStackNew();
SHOULD_NOT_EQUAL("new precedenceStack is not NULL", stack, NULL);
SHOULD_EQUAL("new precedenceStack top = -1", stack->top, -1);
SHOULD_EQUAL("new precedenceStack capacity = 32", stack->capacity, 32);

precedenceStackPush(stack, 2);

SHOULD_EQUAL("after first push top = 0", stack->top, 0);
SHOULD_EQUAL("after first push capacity = 32", stack->capacity, 32);
SHOULD_EQUAL("after first push data on top", stack->data[stack->top], 2);

int64_t top = precedenceStackPop(stack);
SHOULD_EQUAL("top - token data", top, 2);
precedenceStackPush(stack, 2);

precedenceStackPush(stack, 42);

precedenceStackShift(stack);

SHOULD_EQUAL("after string push top = 1", stack->top, 2);
SHOULD_EQUAL("after string push capacity = 32", stack->capacity, 32);
SHOULD_EQUAL("after string push data on top", stack->data[stack->top], 42);

int64_t topTerm = precedenceStackTopTerminal(stack);
SHOULD_EQUAL("top terminal", topTerm, 2);

precedenceStackPop(stack);
SHOULD_EQUAL("after pop top = 0", stack->top, 1);
SHOULD_EQUAL("after pop capacity = 32", stack->capacity, 32);
SHOULD_EQUAL("after pop data on top", stack->data[stack->top], TT_start);

precedenceStackPush(stack, 20);

precedenceStackShift(stack);
SHOULD_EQUAL("after shift top", stack->top, 3);
SHOULD_EQUAL("after shift TT_start on top", stack->data[stack->top], TT_start);

precedenceStackFree(stack);
*/

openFile("3");
printf("Retcode: %d \n", parse());
closeFile();

/*
openFile("expresions");
uint32_t errCode = precedenceParsing(NULL);
printf("%d\n", errCode);
*/

/*
if (openFile("../../skola/java/test9.java") == ERR_OK) {
Token* token = newToken();
tPrecedenceStackPtr stack = precedenceStackNew();
while (token->type != TT_EOF) {
	cleanToken(&token);
	getToken(token);
	precedenceStackPush(stack, token->type);
}
printStack(stack);
freeToken(&token);
precedenceStackFree(stack);
//closeFile();
} else {
	printf("error\n");
}
*/

TEST_SUITE_END
