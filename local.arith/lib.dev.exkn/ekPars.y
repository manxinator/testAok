/*------------------------------------------------------------------------------
  Copyright (c) 2015 manxinator

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
--------------------------------------------------------------------------------
  ekPars.y
  Author:  manxinator
  Created: Wed Dec 23 12:22:08 PST 2015

  Compile: yacc -p ek_yy -o ekPars.cpp -d ekPars.y
  Options: --debug --verbose
------------------------------------------------------------------------------*/
%{
#include "ekPars.hpp"
#include "ekRead.h"
#include "ekInternals.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

extern int   ek_yyLineNum;
extern char* ek_yytext;

        //----------------------------------------------------------------------

extern void ek_yyerror(const char *s);
extern int  ek_yylex  (void);

%}

  // Define the tokens
  //
%token ENDL
%token COMMAND_ID
%token COMMAND_ARGS
%token COMMAND_QSTR

%token XML_TAGID
%token XML_BLOCKTEXT

%token BTICK_SEQ

%token OBJ_START
%token OBJ_ID
%token OBJ_QSTR

%token KNOB_NAME
%token KNOB_QSTR
%token KNOB_EQ

  // - Yacc requires a union for lex return values
  // - Then, associate the return values to defined token types
  //
%union {
  const char *commandIdStr;
  const char *commandArgStr;
  const char *objNameStr;
  const char *knobNameStr;
  const char *xmlTagId;
}
%type <commandIdStr>      COMMAND_ID
%type <commandArgStr>     COMMAND_ARGS
%type <objNameStr>        OBJ_ID
%type <knobNameStr>       KNOB_NAME
%type <xmlTagId>          XML_TAGID

//------------------------------------------------------------------------------
%%

  // The first rule defined is the highest level rule
  //
eklines:
    eklines ekline
  | ekline
  ;

ekline:
    command_stmt              ENDL  { E_DEBUG("[%3d] + [ekline] 1 command_stmt\n\n",ek_yyLineNum); }
  | obj_stmt_aux              ENDL  { E_DEBUG("[%3d] + [ekline] 2 obj_stmt_aux\n\n",ek_yyLineNum); }
  | knob_eqn                  ENDL  { E_DEBUG("[%3d] + [ekline] 3 knob_eqn    \n\n",ek_yyLineNum); }
  | xml_stmt                  ENDL  { E_DEBUG("[%3d] + [ekline] 4 xml_stmt    \n\n",ek_yyLineNum); }
  | ENDL                         { /* E_DEBUG("[%3d] + [ekline] 6 ENDL        \n\n",ek_yyLineNum); */ }
  ;

command_stmt:
    COMMAND_ID command_args       { ek_commandIdent("command_stmt 1",$1); }
  | COMMAND_ID                    { ek_commandIdent("command_stmt 2",$1); }
  ;

command_args:
    command_args COMMAND_ARGS     { ek_commandArgs ("command_args 1",$2); }
  | command_args COMMAND_QSTR     { ek_commandQStr ("command_args 2",ek_collectQStr());  }
  | command_args BTICK_SEQ        { ek_commandBTick("command_args 3"); }
  |              COMMAND_ARGS     { ek_commandArgs ("command_args 4",$1); }
  |              COMMAND_QSTR     { ek_commandQStr ("command_args 5",ek_collectQStr());  }
  |              BTICK_SEQ        { ek_commandBTick("command_args 6"); }
  ;

obj_stmt_aux:
    OBJ_START obj_stmt            { ek_objectDone("obj_stmt_aux 1"); }
  ;

obj_stmt:
    obj_stmt OBJ_ID               { ek_objectStr  ("obj_stmt 1",$2); }
  | obj_stmt OBJ_QSTR             { ek_objectQStr ("obj_stmt 2",ek_collectQStr()); }
  | obj_stmt BTICK_SEQ            { ek_objectBTick("obj_stmt 3"); }
  |          OBJ_ID               { ek_objectStr  ("obj_stmt 4",$1); }
  |          OBJ_QSTR             { ek_objectQStr ("obj_stmt 5",ek_collectQStr()); }
  |          BTICK_SEQ            { ek_objectBTick("obj_stmt 6"); }
  ;


knob_eqn:
    knob_lhs KNOB_EQ knob_rhs     { ek_knobDone("knob_eqn"); }
  ;

knob_lhs:
    knob_lhs KNOB_NAME            { ek_knobStr  ("knob_lhs 1",0,$2); }
  | knob_lhs KNOB_QSTR            { ek_knobQStr ("knob_lhs 2",0,ek_collectQStr()); }
  | knob_lhs BTICK_SEQ            { ek_knobBTick("knob_lhs 3",0); }
  |          KNOB_NAME            { ek_knobStr  ("knob_lhs 4",0,$1); }
  |          KNOB_QSTR            { ek_knobQStr ("knob_lhs 5",0,ek_collectQStr()); }
  |          BTICK_SEQ            { ek_knobBTick("knob_lhs 6",0); }
  ;

knob_rhs:
    knob_rhs KNOB_NAME            { ek_knobStr  ("knob_rhs 1",1,$2); }
  | knob_rhs KNOB_QSTR            { ek_knobQStr ("knob_rhs 2",1,ek_collectQStr()); }
  | knob_rhs BTICK_SEQ            { ek_knobBTick("knob_rhs 3",1); }
  |          KNOB_NAME            { ek_knobStr  ("knob_rhs 4",1,$1); }
  |          KNOB_QSTR            { ek_knobQStr ("knob_rhs 5",1,ek_collectQStr()); }
  |          BTICK_SEQ            { ek_knobBTick("knob_rhs 6",1); }
  ;

xml_stmt:
    XML_TAGID XML_BLOCKTEXT       { E_DEBUG("[%3d]   +   [xml_stmt] Tag: '%s'\n",ek_yyLineNum,$1); }
  ;


%%
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------

// Parser functions and variables
static int ek_yyExitOnErr = 1;

// Lex features needed by yacc
extern int ek_yyparse(void);
extern FILE *ek_yyin;

void ek_yyerror(const char *s)
{
  std::cout << "\n!\n! EEK, parse error! Line: " << ek_yyLineNum << std::endl;
  std::cout << "! Message: " << s << std::endl;
  std::cout << "!\n! (debug yytext='" << ek_yytext << "')\n!\n";
  // might as well halt now:
  if (ek_yyExitOnErr > 0)
    exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------

/*
  NOTE:
  - To enable debugging, set ek_yydebug=1 before calling ek_readfile()
  - ek_readfile() returns 1 for success
*/
int ex_knobs::ek_readfile(const char* inFN, int exitOnErr)
{
//ek_yydebug=1;
  ek_yyExitOnErr = exitOnErr;

  // Open specified filename, and check handle
  FILE *inFH = fopen(inFN, "r");
  if (!inFH) {
    std::cout << "I can't open " << inFN << "!\n";
    return -1;
  }
  // set to parser's file handle... default is STDIN
  // must initialize line numbers
  ek_yyin      = inFH;
  ek_yyLineNum = 1;
  ek_parserInit();

  // parsing loop
  while (!feof(ek_yyin)) {
    printf("--- calling ek_yyparse()!\n");
    ek_yyparse();
  }
  fclose(inFH);
  ek_parserCleanup();
  return ek_yyExitOnErr == 0x00c0ffee ? 0 : 1;
}

//------------------------------------------------------------------------------

