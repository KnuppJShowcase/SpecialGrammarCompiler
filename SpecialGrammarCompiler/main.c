#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "core.h"
#include "parser.h"
#include "printer.h"
#include "semantic.h"
#include "executor.h"
#include "tree.h"

int main(int argc, char *argv[]) {  
  
  scanner_open(argv[1]);

  struct nodeProcedure p;

  //printf("PARSING\n");
  
  parseProcedure(&p);
  
  //printf("PRINTING\n");
  
  //semanticProcedure(&p);
  
  // Reinitialize the scanner with data file, execute
  scanner_close();
  scanner_open(argv[2]);
  
  executeProcedure(&p);
  
  //printProcedure(&p);
  
  // Scanning, parsing, printing is done, release memory
  scanner_close();
  return 0;
}