#include "error.h"
#include <stdio.h>
#include <stdarg.h>


void printError(eError errType, const char *fmt, ...) {
	static const char *errMessages[] = {"Internal", "Lexical", "Syntax", "Semantic", "Semantic - type",
	                                    "Semantic - other", "Runtime - input", "Runtime - uninitialized var",
	                                    "Runtime - division by zero", "Other",};

	//prints type of error and custom formatted string passed as fmt param
	fprintf(stderr, "ERROR: %s - ", errMessages[(int)errType > 90 ? 0 : (int)errType]);
	va_list args;
	va_start (args, fmt);
	vfprintf(stderr, fmt, args);
	va_end (args);
}