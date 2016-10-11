#include <stdlib.h>
#include <inttypes.h>
#include "str.h"
#include "token.h"
#include "stdio.h"

Token* newToken() {
	Token *token = calloc(1, sizeof(Token));
	if (token == NULL) {
		//TODO: hadle error
	} else {
		token->type = TT_empty;
	}

	return token;
}

void freeToken(Token **pToken) {
	if (pToken == NULL) {
		//TODO: hadle error
	} else {
		if (((*pToken)->type == TT_variable)
        	|| ((*pToken)->type == TT_identifier)
			|| ((*pToken)->type == TT_string)) { //here have to be all TokenTypes which contains String data
			strFree(&(*pToken)->str);
		}
		free(*pToken);
		*pToken = NULL;
	}

}


