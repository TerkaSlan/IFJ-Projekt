#include "scanner.h"
#include <stdio.h> // eof, FILE*

FILE* fSourceFile; // TO ADD 'static' to the final build, can't do it now since it's extern in scanner.h

int8_t openFile(const char *sFileLocation){

  if (sFileLocation == NULL){
    printError(ERR_OTHER, " in identifying the file source in %s at line: %d", __FILE__, __LINE__);
    return ERR_OTHER;
  }
  if ((fSourceFile = fopen(sFileLocation, "r")) == NULL){
    printError(ERR_OTHER, " in opening the file in %s at line: %d", __FILE__, __LINE__);
    fSourceFile = NULL;
    return ERR_OTHER;
  }
  return ERR_OK;
}

KeywordTokenType getKeywordType(dtStr *string){
  switch(string->str[0]){
    case 'b': {
      if (strCmpCStr(string, "boolean") == 0){
        return KTT_boolean;
      }
      else if (strCmpCStr(string, "break") == 0){
        return KTT_break;
      }
      break;
    }
    case 'c': {
      if (strCmpCStr(string, "class") == 0){
        return KTT_class;
      }
      else if (strCmpCStr(string, "continue") == 0){
        return KTT_continue;
      }
      break;
    }
    case 'd': {
      if (strCmpCStr(string, "do") == 0){
        return KTT_do;
      }
      else if (strCmpCStr(string, "double") == 0){
        return KTT_double;
      }
      break;
    }
    case 'e': {
      if (strCmpCStr(string, "else") == 0){
        return KTT_else;
      }
      break;
    }
    case 'f': {
      if (strCmpCStr(string, "false") == 0){
        return KTT_false;
      }
      else if (strCmpCStr(string, "for") == 0){
        return KTT_for;
      }
      break;
    }
    case 'i': {
      if (strCmpCStr(string, "if") == 0){
        return KTT_if;
      }
      else if (strCmpCStr(string, "int") == 0){
        return KTT_int;
      }
      break;
    }
    case 'r': {
      if (strCmpCStr(string, "return") == 0){
        return KTT_return;
      }
      break;
    }
    case 's': {
      if (strCmpCStr(string, "static") == 0){
        return KTT_static;
      }
      break;
    }
    case 'S': {
      if (strCmpCStr(string, "String") == 0){
        return KTT_String;
      }
      break;
    }
    case 't': {
      if (strCmpCStr(string, "true") == 0){
        return KTT_true;
      }
      break;
    }
    case 'v': {
      if (strCmpCStr(string, "void") == 0){
        return KTT_void;
      }
      break;
    }
    case 'w': {
      if (strCmpCStr(string, "while") == 0){
        return KTT_while;
      }
      break;
    }
    default:
      return KTT_none; // add to token.h
  }
  return KTT_none;
}

int8_t getToken(Token *token){

  int32_t iCurrentSymbol;
  uint8_t octalLength = 0;
  dtStr *octalString;

  tFSMState state = SEmpty;
  while (1){
    iCurrentSymbol = getc(fSourceFile);
    switch(state){

      case SEmpty: {
        if (isalpha(iCurrentSymbol) || (iCurrentSymbol == '_') || (iCurrentSymbol == '$')){
          state = SId;
          if ((token->str = strNew()) == NULL)
            handleLexError(token, ERR_INTERN);
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR)
            handleLexError(token, ERR_INTERN);
        }
        else if (isdigit(iCurrentSymbol)){
          state = SNumber;
          if ((token->str = strNew()) == NULL)
            handleLexError(token, ERR_INTERN);
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR)
            handleLexError(token, ERR_INTERN);
        }
        // true for ' ', '\t', '\n', '\v', '\f', '\r'
        else if (iCurrentSymbol == EOF){
          token->type = TT_EOF;
          fclose(fSourceFile);
          return ERR_OK;
        }
        else if (isspace(iCurrentSymbol)){
          ;
        }
        else {
          if ((token->str = strNew()) == NULL)
            handleLexError(token, ERR_INTERN);
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR)
            handleLexError(token, ERR_INTERN);
          switch(iCurrentSymbol){
            case '=':
              state = SAssignment;
              break;
            case '{':
              token->type = TT_leftCurlyBracket;
              strFree(token->str);
              return ERR_OK;
            case '}':
              token->type = TT_rightCurlyBracket;
              strFree(token->str);
              return ERR_OK;
            case '(':
              token->type = TT_leftRoundBracket;
              strFree(token->str);
              return ERR_OK;
            case ')':
              token->type = TT_rightRoundBracket;
              strFree(token->str);
              return ERR_OK;
            case '+':
              token->type = TT_plus;
              strFree(token->str);
              return ERR_OK;
            case '-':
              state = SMinus;
              break;
            case '*':
              token->type = TT_multiply;
              strFree(token->str);
              return ERR_OK;
            case '/':
              state = SDivide;
              // no matter what SDivide results in, i won't be needing the string
              strFree(token->str);
              break;
            case '<':
              state = SLess;
              strFree(token->str);
              break;
            case '>':
              state = SGreater;
              strFree(token->str);
              break;
            case '!':
              state = SExclamation;
              // no matter what SExclamation results in, i won't be needing the string
              strFree(token->str);
              break;
            case ';':
              token->type = TT_semicolon;
              strFree(token->str);
              return ERR_OK;
            case '"':
              state = SString;
              break;
            case ',':
              token->type = TT_comma;
              strFree(token->str);
              return ERR_OK;
            default:
              handleLexError(token, ERR_LEX);
          }
        }
        break;
      }
      case SGreater: {
        if (iCurrentSymbol == '='){
          token->type = TT_greaterEqual;
        }
        else{
          token->type = TT_greater;
          ungetc(iCurrentSymbol, fSourceFile);
        }
        return ERR_OK;
      }
      case SLess: {
        if (iCurrentSymbol == '='){
          token->type = TT_lessEqual;
        }
        else{
          token->type = TT_less;
          ungetc(iCurrentSymbol, fSourceFile);
        }
        return ERR_OK;
      }
      case SAssignment: {
        if (iCurrentSymbol == '='){
          token->type = TT_equal;
        }
        else{
          token->type = TT_assignment;
          ungetc(iCurrentSymbol, fSourceFile);
        }
        strFree(token->str);
        return ERR_OK;
      }
      // Common umbrella for ints and doubles. If double detected, redirected to
      // SDouble state
      case SNumber: {
        if (isdigit(iCurrentSymbol)){
          state = SNumber;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
              handleLexError(token, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == '.'){
          state = SDouble;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, iCurrentSymbol);
          }
        }
        else if ((iCurrentSymbol == 'e') || (iCurrentSymbol == 'E')){
          state = SDoubleExponent;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else{
          ungetc(iCurrentSymbol, fSourceFile);

          int32_t number = stringToInt(token->str);
          if (number == INT_CONVERSION_ERROR){
            handleLexError(token, ERR_LEX);
          }
          strFree(token->str);
          token->iNum = number;
          token->type = TT_number;
          return ERR_OK;
        }
        break;
      }
      case SDouble: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleDecimalPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else{
          handleLexError(token, ERR_LEX);
        }
        break;
      }
      case SDoubleDecimalPart: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleDecimalPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == 'e' || iCurrentSymbol == 'E'){
          state = SDoubleExponent;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else {
          ungetc(iCurrentSymbol, fSourceFile);

          double db = stringToDouble(token->str);
          if (fequal(db, DOUBLE_CONVERSION_ERROR)){
            handleLexError(token, ERR_LEX);
          }
          strFree(token->str);
          token->dNum = db;
          token->type = TT_double;
          return ERR_OK;
        }
        break;
      }
      case SDoubleExponent: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else if ((iCurrentSymbol == '+') || (iCurrentSymbol == '-')){
          state = SDoubleExponentSign;
        }
        else {
          handleLexError(token, ERR_LEX);
        }
        break;
      }
      case SDoubleExponentPart: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else{
          ungetc(iCurrentSymbol, fSourceFile);

          double db = stringToDouble(token->str);
          if (fequal(db, DOUBLE_CONVERSION_ERROR)){
            handleLexError(token, ERR_LEX);
          }
          strFree(token->str);
          token->dNum = db;
          token->type = TT_double;
          return ERR_OK;
        }
        break;
      }
      case SDoubleExponentSign: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else{
          handleLexError(token, ERR_LEX);
        }
        break;
      }
      case SId: {
        if (isalpha(iCurrentSymbol) || isdigit(iCurrentSymbol) || iCurrentSymbol == '_' || iCurrentSymbol == '$'){
          state = SId;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == '.'){
          state = SFullId;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else {
          ungetc(iCurrentSymbol, fSourceFile);
          if (strCmpCStr(token->str, "and") == 0){
            token->type = TT_and;
            strFree(token->str);
            return ERR_OK;
          }
          if (strCmpCStr(token->str, "or") == 0){
            token->type = TT_or;
            strFree(token->str);
            return ERR_OK;
          }
          KeywordTokenType keywordType = getKeywordType(token->str);
          if (keywordType == KTT_none){
            token->type = TT_identifier;
            return ERR_OK;
          }
          else{
            // need to free token->str, sice initialization of token->keywordType would overshadow its value
            strFree(token->str);
            token->type = TT_keyword;
            token->keywordType = keywordType;
            return ERR_OK;
          }
        }
        break;
      }
      case SFullId: {
        if (isalpha(iCurrentSymbol) || isdigit(iCurrentSymbol) || iCurrentSymbol == '_' || iCurrentSymbol == '$'){
          state = SFullId;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
        }
        else{
          // checks if char after '.' is a digit, if it is, fullId won't be valid
          if (isdigit(token->str->str[strCharPos(token->str, '.') + 1]))
              handleLexError(token, ERR_LEX);

          KeywordTokenType keywordType = getKeywordType(token->str);
          if (keywordType != KTT_none){
            handleLexError(token, ERR_LEX);
          }
          else{
            token->type = TT_fullIdentifier;
            ungetc(iCurrentSymbol, fSourceFile);
          }
          return ERR_OK;
        }
        break;
      }
      case SDivide: {
        if (iCurrentSymbol == '*'){
          state = SBlockCommentStart;
        }
        else if (iCurrentSymbol == '/'){
          state = SComment;
        }
        else {
          ungetc(iCurrentSymbol, fSourceFile);
          token->type = TT_divide;
          return ERR_OK;
        }
        break;
      }
      case SString: {
        // Empty string
        if (iCurrentSymbol == '"'){
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
          token->type = TT_string;
          return ERR_OK;
        }
        // You can't just EOF on me with " as a previous symbol
        else if (iCurrentSymbol == EOF){
          handleLexError(token, ERR_LEX);
        }
        else{
          if (iCurrentSymbol >= MIN_ASCII_VALUE){
            if (iCurrentSymbol == '\\'){
              state = SEscape;
            }
            else{
              state = SString;
              if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
                handleLexError(token, ERR_INTERN);
              }
            }
          }
          else{
            handleLexError(token, ERR_LEX);
          }
        break;
      }
      case SEscape: {
        switch (iCurrentSymbol) {
          case 'n': {
            state = SString;
            if (strAddChar(token->str, '\n') == STR_ERROR){
              handleLexError(token, ERR_INTERN);
            }
            break;
          }
          case 't': {
            state = SString;
            if (strAddChar(token->str, '\t') == STR_ERROR){
              handleLexError(token, ERR_INTERN);
            }
            break;
          }
          case '\"': {
            state = SString;
            if (strAddChar(token->str, '\"') == STR_ERROR){
              handleLexError(token, ERR_INTERN);
            }
            break;
          }
          case '\\': {
            state = SString;
            if (strAddChar(token->str, '\\') == STR_ERROR){
              handleLexError(token, ERR_INTERN);
            }
            break;
          }
          default: {
            if (iCurrentSymbol >= '0' && iCurrentSymbol <= '3'){
              state = SOctal;
              octalString = strNew();
              if (strAddChar(octalString, iCurrentSymbol) == STR_ERROR){
                handleLexError(token, ERR_INTERN);
              }
              octalLength++;
            }
            else if (iCurrentSymbol <= MIN_ASCII_VALUE){
              handleLexError(token, ERR_LEX);
            }
            else{
              state = SString;
            }
          }
        }
        break;
      }
      case SOctal: {
        // 001 to 377 are valid sequences (0-3 already in token->str)
        if ((iCurrentSymbol >= '0' && iCurrentSymbol <= '7') && octalLength < 3){
          state = SOctal;
          strAddChar(octalString, iCurrentSymbol);
          octalLength++;
        }
        else{
          ungetc(iCurrentSymbol, fSourceFile);
          int32_t intFromOctal = octalToInt(octalString);
          strFree(octalString);
          if (intFromOctal == INT_CONVERSION_ERROR){
            handleLexError(token, ERR_INTERN);
          }
          if (strAddChar(token->str, (char)intFromOctal) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
          state = SString;
        }
        break;
      }
      case SComment: {
        if (iCurrentSymbol == '\n'){
          state = SEmpty;
        }
        else if (iCurrentSymbol == EOF){
          token->type = TT_EOF;
          return ERR_OK;
        }
        else{
          state = SComment;
        }
        break;
      }
      case SBlockCommentStart: {
        if (iCurrentSymbol == '*'){
          state = SBlockCommentFinish;
        }
        else if (iCurrentSymbol == EOF){
          handleLexError(token, ERR_LEX);
        }
        else {
          state = SBlockCommentStart;
        }
        break;
      }
      case SBlockCommentFinish: {
        if (iCurrentSymbol == '/') {
            state = SEmpty;
        }
        else if (iCurrentSymbol == EOF){
          handleLexError(token, ERR_LEX);
        }
        else {
          state = SBlockCommentStart;
        }
        break;
      }
      case SExclamation: {
        if (iCurrentSymbol == '='){
          token->type = TT_notEqual;
        }
        else {
          token->type = TT_not;
          ungetc(iCurrentSymbol, fSourceFile);
        }
        return ERR_OK;
      }
      case SMinus: {
        if (isdigit(iCurrentSymbol)){
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleLexError(token, ERR_INTERN);
          }
          state = SNumber;
        }
        else{
          ungetc(iCurrentSymbol, fSourceFile);
          token->type = TT_minus;
          strFree(token->str);
          return ERR_OK;
        }
        break;
      }
      default: {
        handleLexError(token, ERR_LEX);
      }
      }
    }
  }
}
