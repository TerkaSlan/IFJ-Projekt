cc=gcc
CFLAGS=-g -Wall -Werror -pedantic

TEST_C_FILES=$(wildcard test*.c)
TEST_H_FILES=$(wildcard test*.h)

SOURCE_C_FILES=$(wildcard ../*.c)
SOURCE_H_FILES=$(wildcard ../*.h)

SRC_OBJ_FILES = $(patsubst %.c, %.o, $(SOURCE_C_FILES))
TEST_OBJ_FILES = $(patsubst %.c, %.o, $(TEST_C_FILES))

test: $(SRC_OBJ_FILES) $(TEST_OBJ_FILES)
	$(CC) $(CFLAGS) $(SRC_OBJ_FILES) $(TEST_OBJ_FILES) -o test

$(SRC_OBJ_FILES): $(SOURCE_C_FILES) $(SOURCE_H_FILES)
$(TEST_OBJ_FILES): $(TEST_C_FILES) $(TEST_H_FILES)

clean:
	rm -rf $(TEST_OBJ_FILES) $(SRC_OBJ_FILES) test
