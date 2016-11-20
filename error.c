#include "error.h"
#include <stdio.h>
#include <stdarg.h>


void printError(eError errType, const char *fmt, ...) {
	static const char *errMessages[] = {"Internal", "Lexical", "Syntactic", "Semantic", "Semantic - type", "",
	                                    "Semantic - other", "Runtime - input", "Runtime - uninitialized var",
	                                    "Runtime - division by zero", "Other",};

	//prints type of error and custom formatted string passed as fmt param
	//uses errType as index into the array of error representations; Internal error has a value of 99 which was put to index 0, because 0 represents NO ERROR (ERR_OK)
	fprintf(stderr, "ERROR: %s - ", errMessages[errType == ERR_INTERN ? 0 : (int)errType]);
	va_list args;
	va_start (args, fmt);
	vfprintf(stderr, fmt, args);
	va_end (args);
}
