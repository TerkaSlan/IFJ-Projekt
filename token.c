/**
  *  \file token.c
  *  \brief Contains implementation of functions regarding the token abstraction
  *  \author Katka
  *
  * Project: IFJ
  * Implementace interpretu imperativního jazyka IFJ16.
  *
  * Description:
  * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2016.pdf
  *
  * Team:
  * Michal Charvát          (xcharv16)
  * Terézia Slanináková     (xslani06)
  * Katarína Grešová        (xgreso00)
  * Marek Schauer           (xschau00)
  * Jakub Handzuš           (xhandz00)
  */

#include <stdlib.h>
#include "token.h"

Token* newToken() {
	Token *token = calloc(1, sizeof(Token));
	if (token != NULL) {
		token->type = TT_empty;
	}

	return token;
}

void freeToken(Token **pToken) {
	if (pToken != NULL) {
		if (*pToken != NULL) {
			if (((*pToken)->type == TT_fullIdentifier)
	        	|| ((*pToken)->type == TT_identifier)
			|| ((*pToken)->type == TT_string)) {
				strFree((*pToken)->str);
			}
			free(*pToken);
			*pToken = NULL;
		}
	}
}


void cleanToken(Token **pToken) {
	if (pToken != NULL) {
		if (*pToken != NULL) {
			if (((*pToken)->type == TT_fullIdentifier)
	        	|| ((*pToken)->type == TT_identifier)
			|| ((*pToken)->type == TT_string)) {
				strFree((*pToken)->str);
			}
			(*pToken)->type = TT_empty;
			(*pToken)->str  = NULL;
		}
	}
}
