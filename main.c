#include <stdio.h>
#include "parser.h"
#include "scanner.h"
#include "interpret.h"
#include "error.h"
#include "parser_SecondRun.h"

tHashTablePtr globalScopeTable;
tInstructionListPtr instructionList;
tConstContainerPtr constTable;

eError initializeGlobalVariables(){
  if((globalScopeTable = htabInit(HTAB_DEFAULT_SIZE)) == NULL){
    return ERR_INTERN;
  }
  if((constTable = constNew()) == NULL) {
    htabFree(globalScopeTable);
    return ERR_INTERN;
  }
  if ((instructionList = instrListNew()) == NULL){
    htabFree(globalScopeTable);
    constFree(constTable);
    return ERR_INTERN;
  }
  return ERR_OK;
}

void freeGlobalVariables(){
  constFree(constTable);
  htabFree(globalScopeTable);     // TODO Michal: [#24]
  instrListFree(instructionList);
}

int main(int argc, char const *argv[]) {
  openFile("3");
  initializeGlobalVariables();
  printf("Retcode: %d \n", prog());
  parse_2();
  Interpret(globalScopeTable, instructionList);
  closeFile();
  return 0;
}
