#include <intermediate.h>

#include <stdio.h>
#include <stdlib.h>

#include <file.h>
#include <parser.h>
#include <bnf.h>
#include <ast.h>
#include <symbol.h>
#include <error.h>

#define IR_MAX_FILE_LENGTH 1024

void translateast(ASTNode *ast, char *metadata) {

}

void intermediate(Options *options) {
  char  irfile[IR_MAX_FILE_LENGTH];
  char  ppfile[IR_MAX_FILE_LENGTH];
  char  metafile[IR_MAX_FILE_LENGTH];
  char *woext = filenamewoext(options->output);
  sprintf(irfile,   "%s.isr", woext);
  sprintf(ppfile,   "%s.psr", woext);
  sprintf(metafile, "%s.msr", woext);
  free(woext);
  {
    Parser    *parser       = newParser("parsing/prs/csr.prs");
    FILE      *output       = fopen(irfile,   "w+");
    Stream    *preprocessed = getStreamSS(ssopen(ppfile, parser));
    BNFNode   *tree         = parsebnf("parsing/bnf/program.bnf");
    ASTNode   *ast          = newASTNode(NULL, NULL);
    ASTStatus  status;

    if (preprocessed) {
      status = astparsestream(ast, tree, preprocessed);
      if (preprocessed->symbol->type != SYMBOL_EOF || status != STATUS_CONFIRMED) {
        printirmessage(ERRLVL_ERROR, metafile, preprocessed->symbol, "Unexpected symbol!");
      }
      closeStream(preprocessed);
    }
    if (ast) {
      translateast(ast, metafile);
      deleteAST(&ast);
    }
    if (tree)   deleteBNFTree(&tree);
    if (output) fclose(output);
    if (parser) deleteParser(&parser);
  }
  remove(ppfile);
  remove(metafile);
}