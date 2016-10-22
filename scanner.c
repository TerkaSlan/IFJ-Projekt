#include "scanner.h"

typedef enum
{
    SEmpty,
    SId,
    SFullId,
    SEqual,
    SNotEqual,
    SNumber,
    SDecimal,
    SDouble,
    SDoubleDecimalPart,
    SDoubleExponentPart,
    SDoubleExponentSign,
    SDoubleExponent,
    SAssignment,
    SGreater,
    SLess,
    SDivide,
    SBlockCommentStart,
    SBlockCommentFinish,
    SComment,
    SExclamation,
    SString,
    SEscape,
    SOctalEscape
} tFSMState;

static FILE* fSourceFile;
#define MIN_ASCII_VALUE 32

int8_t openFile(const char *sFileLocation){

  if (sFileStream == NULL){
    printError(ERR_OTHER, " in identifying the file source in %s at %s", __FILE__, __LINE__);
    return ERR_OTHER;
  }
  if ((fSourceFile = fopen(sFileStream, "r")) == NULL){
    printError(ERR_OTHER, " in opening the file in %s at %s", __FILE__, __LINE__);
    fclose(fSourceFile);
    fSourceFile = NULL;
    return ERR_OTHER;
  }
  return ERR_OK
}

KeywordTokenType getKeywordType(const dtStr *string){
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
        return KTT_string;
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
}

int8_t getToken(Token *token){
  if ((token->str = strNew()) == NULL)
    return ERR_INTERN;

  int32_t iCurrentSymbol;
  uint8_t octalLength = 0;
  tFSMState state = SEmpty;
  while (1){
    iCurrentSymbol = getc(fSourceFile);
    switch(state){

      case SEmpty: {
        if (isalpha(iCurrentSymbol) || (iCurrentSymbol == '_') || (iCurrentSymbol == '$')){
          state = SId;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR)
            return ERR_INTERN;
        }
        else if (isdigit(iCurrentSymbol))
          state = SNumber;
        else if (isspace(iCurrentSymbol)){
          ;
        }
        else if (iCurrentSymbol == EOF){
          token->type = KTT_EOF;
          return ERR_OK;
        }
        else {
          switch(iCurrentSymbol){
            case '=':
              state = SAssignment;
              break;
            case '{':
              token->type = TT_leftCurlyBracket;
              break;
            case '}':
              token->type = TT_rightCurlyBracket;
              break;
            case '(':
              token->type = TT_leftRoundBracket;
              break;
            case ')':
              token->type = TT_rightRoundBracket;
              break;
            case '+':
              token->type = TT_plus;
              break;
            case '-':
              token->type = TT_minus;
              break;
            case '*':
              token->type = TT_multiply;
              break;
            case '/':
              state = SDivide;
              break;
            case '<':
              state = SLess;
              break;
            case '>':
              state = SGreater;
              break;
            case '!':
              state = SExclamation;
              break;
            case ';':
              token->type = TT_semicolon;
              break;
            case '"':
              state = SString;
              break;
            case ',':
              token->type = TT_comma;
              break;
            default:
              printError(ERR_LEX, " in %s at %s, current: %c ", __FILE__, __LINE__, iCurrentSymbol);
              return ERR_LEX;
          }
        }
        break;
      }
      case SGreater: {
        if (iCurrentSymbol == '='){
          token->type = TT_greaterEqual;
          return ERR_OK;
        }
        else{
          token->type = TT_greater;
          return ERR_OK;
        }
        break;
      }
      case SLess: {
        if (iCurrentSymbol == '='){
          token->type = TT_lessEqual;
          return ERR_OK;
        }
        else{
          token->type = TT_less;
          return ERR_OK;
        }
        break;
      }
      case SAssignment: {
        if (iCurrentSymbol == '='){
          state = SEqual;
        }
        else{
          token->type = TT_assignment;
          return ERR_OK;
        }
        break;
      }
      case SNumber: {
        if (isdigit(iCurrentSymbol)){
          state = SNumber;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
              handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == '.'){
          state = SDouble;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol);
          }
        }
        else if ((iCurrentSymbol == 'e') || (iCurrentSymbol == 'E')){
          state = SDoubleExponent;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else{
          int32_t number = stringToInt(token->str);
          if (number == ERR_LEX){
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          strFree(token->str);
          token->iNum = number;
          token->type = TT_number;
        }
        break;
      }
      case SDouble: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleDecimalPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else{
          handleError(token, iCurrentSymbol, ERR_LEX);
        }
        break;
      }
      case SDoubleDecimalPart: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleDecimalPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == 'e' || iCurrentSymbol == 'E'){
          state = SDoubleExponent;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else {
          double db = stringToDouble(token->str);
          if (db == ERR_LEX){
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          strFree(token->str);
          token->dNum = db;
          token->type = TT_Double;
        }
        break;
      }

      case SDoubleExponent: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
          else if ((iCurrentSymbol == '+') || (iCurrentSymbol == '-')){
            state = SDoubleExponentSign;
          }
          else {
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          break;
      }
      case SDoubleExponentPart: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else{
          double db = stringToDouble(token->str);
          if (db == ERR_LEX){
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          token->dNum = db;
          token->type = SDouble;
          return ERR_OK;
        }
        break;
      }
      case SDoubleExponentSign: {
        if (isdigit(iCurrentSymbol)){
          state = SDoubleExponentPart;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else{
          handleError(token, iCurrentSymbol, ERR_LEX);
        }
        break;
      }
      case SId: {
        if (isalpha(iCurrentSymbol) || isdigit(iCurrentSymbol) || iCurrentSymbol == '_' || iCurrentSymbol == '$'){
          state = SId;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
        }
        else if (iCurrentSymbol == '.'){
          state = SFullId; // ?
          return ERR_OK;
        }
        else {
          KeywordTokenType keywordType = getKeywordType(token->str);
          if (keywordType == TT_None){
            token->type = TT_identifier;
            return ERR_OK;
          }
          else if (keywordType == ERR_LEX){
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          else{
            token->type = TT_keyword;
            if (keywordType == TT_True) {
              token->keywordType = KTT_boolean;
              token->iNum = 1;
            }
            if (keywordType == TT_False) {
              token->keywordType = KTT_boolean;
              token->iNum = 0;
            }
            else{
              token->keywordType = keywordType;
            }
          }
        }
        break;
      }
      case SFullId: {
        if (isalpha(iCurrentSymbol) || isdigit(iCurrentSymbol) || iCurrentSymbol == '_' || iCurrentSymbol == '$'){
          state = SFullId;
        }
        else{
          KeywordTokenType keywordType = getKeywordType(&(token->str));
          if (keywordType != TT_None){
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
          token->type = TT_fullIdentifier; // add to token.h
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
          return ERR_OK;
        }
        break;
      }
      case SDivide : {
        if (iCurrentSymbol == '*'){
          state = SBlockCommentStart;
        }
        else if (iCurrentSymbol == '/'){
          state = SComment;
        }
        else {
          token->type = TT_divide;
          return ERR_OK;
        }
        break;
      }
      case SEqual: {
          if (iCurrentSymbol == '='){
            token->type = SEqual;
          }
          else {
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
      }
      case SString: {
        if (iCurrentSymbol == '"'){
          token->type = TT_string;
          return ERR_OK;
        }
        else if (iCurrentSymbol == EOF){
          handleError(token, iCurrentSymbol, ERR_LEX);
        }
        else{
          if (iCurrentSymbol >= MIN_ASCII_VALUE){
            if (iCurrentSymbol == '\\'){
              state = SEscape;
            }
            else{
              state = SString;
              if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
                handleError(token, iCurrentSymbol, ERR_INTERN);
              }
            }
          }
          else{
            handleError(token, iCurrentSymbol, ERR_LEX);
          }
        break;
      }
      case SEscape: {
        switch (iCurrentSymbol) {
          case 'n': {
            state = SString;
            if (strAddChar(token->str, '\n') == STR_ERROR){
              handleError(token, iCurrentSymbol, ERR_INTERN);
            }
            break;
          }
          case 't': {
            state = SString;
            if (strAddChar(token->str, '\t') == STR_ERROR){
              handleError(token, iCurrentSymbol, ERR_INTERN);
            }
            break;
          }
          case '\"': {
            state = SString;
            if (strAddChar(token->str, '\"') == STR_ERROR){
              handleError(token, iCurrentSymbol, ERR_INTERN);
            }
            break;
          }
          case '\\': {
            state = SString;
            if (strAddChar(token->str, '\\') == STR_ERROR){
              handleError(token, iCurrentSymbol, ERR_INTERN);
            }
            break;
          }
          default: {
            if (iCurrentSymbol >= '0' && iCurrentSymbol <= '3'){
              state = SOctalEscape;
              if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
                handleError(token, iCurrentSymbol, ERR_INTERN);
              }
              octalLength++;
              break;
            }
            else if (iCurrentSymbol <= MIN_ASCII_VALUE){
              handleError(token, iCurrentSymbol, ERR_LEX);
            }
            else{
              state = SString;
            }
            break;
          }
        }
      }
      case SOctalEscape: {
        //od 001 do 377
        if ((iCurrentSymbol >= '0' && iCurrentSymbol <= '7') && octalLength < 3){
          state = SOctalEscape;
          if (strAddChar(token->str, iCurrentSymbol) == STR_ERROR){
            handleError(token, iCurrentSymbol, ERR_INTERN);
          }
          octalLength++;
        }
        else{
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
        }
        else {
          state = SComment;
        }
        break;
      }
      case SBlockCommentStart: {
        if (iCurrentSymbol == '*'){
          state = SBlockCommentFinish;
        }
        else if (iCurrentSymbol == EOF){
          handleError(token, iCurrentSymbol, ERR_LEX);
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
          handleError(token, iCurrentSymbol, ERR_LEX);
        }
        else {
          state = SBlockCommentStart;
        }
        break;
      }
      case SExclamation: {
        if (iCurrentSymbol == '='){
          state = SNotEqual;
        }
        else {
          token->type = TT_not;
          return ERR_OK;
        }
        break;
      }
      case SNotEqual: {
        if (iCurrentSymbol == '=') {
          token->type = TT_notEqual;
        }
        else {
          handleError(token, iCurrentSymbol, ERR_LEX);
        }
        break;
      }
      default: {
        handleError(token, iCurrentSymbol, ERR_LEX);
      }
      }
    }
  }
}
}
