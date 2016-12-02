#IFJ Project 2016
CC=gcc
CFLAGS=-std=c99 -O2 -Wall -Wextra -g
LIB=-lm

SRC_C_FILES=builtin.c constants.c conversions.c error.c frame.c ial.c instruction.c interpret.c str.c scanner.c token.c expr.c parser.c parser_second.c
SRC_HEADER_FILES=$(SRC_C_FILES:.c=.h)
SRC_OBJ_FILES=$(SRC_C_FILES:.c=.o)
TEST_C_FILES=test/test.c test/test-conversions.c test/test-hashtable.c test/test-instructionList.c test/test-interpret.c test/test-scanner.c test/test_builtin.c test/test_ial.c test/test_str.c test/test_token.c test/test-parser.c
TEST_OBJ_FILES=$(TEST_C_FILES:.c=.o)

all: interpret

interpret: $(SRC_OBJ_FILES) main.o $(SRC_HEADER_FILES)
	$(CC) $(CFLAGS) -o interpret main.o $(SRC_OBJ_FILES) $(LIB)

test: $(SRC_OBJ_FILES) $(TEST_OBJ_FILES) $(SRC_HEADER_FILES) $(TEST_HEADER_FILES)
	$(CC) $(CFLAGS) -o ./test/test $(SRC_OBJ_FILES) $(TEST_OBJ_FILES) $(LIB)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
	
$(SRC_OBJ_FILES): $(SRC_C_FILES) $(SRC_HEADER_FILES)
$(TEST_OBJ_FILES): $(TEST_C_FILES) test/test.h

clean:
	rm -f interpret ./test/test *.o ./test/*.o