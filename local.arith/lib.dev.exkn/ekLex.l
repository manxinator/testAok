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
  ekLex.l
  Author:  manxinator
  Created: Wed Dec 23 12:16:53 PST 2015

  Compile: flex -Pek_yy -o ekLex.cpp ekLex.l
------------------------------------------------------------------------------*/
%{
#include "ekPars.hpp"
#include "ekRead.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>

  /*
    Utility Functions
  */
void stringNewline2LineNum(const char* readStr);

  /*
    Lex Functions
  */
static void ek_newLine   (void);
static void ek_comment_sl(const char* commStr);
static void ek_comment_b0(const char* commStr);    // Start of comment
static void ek_comment_b1(const char* commStr);    // Comment chunks
static void ek_comment_b2(const char* commStr);    // End of comment
static void ek_comment_nl(void);                   // Newline inside comment
static void ek_command_id(const char* commStr);
static void ek_command_ar(const char* commStr);
static void ek_xml_start (const char* xmlStr);
static int  ek_xml_match (const char* xmlStr);

static void ek_quoteStr    (const char* startStr);
static void ek_commandQuote(const char* startStr);

  /*
    Yacc Functions
  */
void ek_yyerror(const char *s);

int ek_yyLineNum = -1; // must be initialized
%}

%option stack

  /*
    States
  */
%s      COMMAND
%s      IN_COMMENT_BL
%s      IN_XML_START
%s      IN_XML_BODY

  /*
    Lex Tokens
  */
wsLeading           ^[\t ]+
whitespace          [\t ]+
quotStr_start       \"
identString         [A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*

newLine             [\t ]*\n

commandId           "#"[\t ]*[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*
commandArgs         [^\t\n \"]+

commentSl           \/\/[^\n]*\n
commentBl_start     \/\*

xml_start           \<[\t \n]*[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*
xml_end             \<[\t \n]*\/[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*[\t \n]*\>

/*----------------------------------------------------------------------------*/
%%
  // Idea: within INITIAL, save last line number before any state transition -- or maybe always
  //       this tells yacc when the last statement was initiated

<INITIAL>{wsLeading}        ;
<INITIAL>{commentSl}        {                               ek_comment_sl(yytext); return ENDL; }
<INITIAL>{commentBl_start}  { yy_push_state(IN_COMMENT_BL); ek_comment_b0(yytext); }
<INITIAL>{commandId}        { BEGIN COMMAND;                ek_command_id(yytext); return COMMAND_ID; }
<INITIAL>{xml_start}        { BEGIN IN_XML_START;           ek_xml_start (yytext); }
<INITIAL>{newLine}          {                               ek_newLine();          return ENDL; }
<INITIAL>.                  ek_yyerror("Unexpected token [inside INITIAL state]");

<COMMAND>{quotStr_start}    { ek_commandQuote(yytext);               return COMMAND_COLLECT; }    //{ ek_quoteStr(yytext); } // TODO: set commandArgCollect=1;  return COMMAND_ARGS;
<COMMAND>{commentSl}        { BEGIN INITIAL; ek_comment_sl(yytext);  return ENDL; }
<COMMAND>{commentBl_start}  { yy_push_state(IN_COMMENT_BL); ek_comment_b0(yytext); }
<COMMAND>{commandArgs}      { ek_command_ar(yytext);                 return COMMAND_ARGS; }
<COMMAND>{whitespace}       ;
<COMMAND>{newLine}          { BEGIN INITIAL; ek_newLine(); return ENDL; }
  /*<COMMAND>.              { printf(" { <COMMAND>. '%s' }\n",yytext); } -- Supposedly unreachable state */

<IN_COMMENT_BL>{
    "*/"        { yy_pop_state(); ek_comment_b2(yytext); }
    [^*\n]+     ek_comment_b1(yytext);
    "*"         ek_comment_b1(yytext);
    \n          ek_comment_nl();
    <<EOF>>     { ek_yyerror("Unexpected EOF encountered while parsing comment block"); yyterminate(); }
} /* { ...; } -- NOTE: Suceeding code block has no effect, but I can't find where this is documented in the flex manual */

<IN_XML_START>{
    /* TODO: if string, then handle string */
    {commentBl_start}   { yy_push_state(IN_COMMENT_BL); ek_comment_b0(yytext); }
    ">"                 { BEGIN(IN_XML_BODY); return XML_TAGID; }
    [\t ]+              // eat whitespaces
    \n                  ek_newLine();
    [^\t \n\>]+         printf("===> [IN_XML_START] eat xml option '%s' at line %d\n",yytext,ek_yyLineNum);
    /* . --> TODO: error! supposedly unreachable state! */
}
<IN_XML_BODY>{
    {xml_end}   { if (ek_xml_match(yytext)) { BEGIN INITIAL; return XML_BLOCKTEXT; } }  // TODO: else, collect
    \n          ek_newLine(); // TODO: pass into XML
    .+          printf("===> [IN_XML_BODY] eat xml body '%s' at line %d\n",yytext,ek_yyLineNum);
}




%%
/*----------------------------------------------------------------------------*/
#define MAX_STR_SIZE    4096

static char ek_static_str[MAX_STR_SIZE]; // CommandID, XmlTag
int debug_aaaa = 0;

void ek_newLine(void)
{
  if (debug_aaaa)  //-- for targeted debugging
    printf("~~~~ current line: %d\n",ek_yyLineNum);
  ek_yyLineNum++;
}

void ek_comment_sl(const char* commStr)
{
  E_DEBUG("* [ek_comment_sl] '%s'\n",commStr);
  ek_newLine();
  // TODO: push to external handler; pass in string and line number
}

        //----------------------------------------------------------------------

static std::vector<std::string> commentMlLines;
static char                     commentMlAccum[MAX_STR_SIZE];

void ek_comment_nl(void)
{
  E_DEBUG("* [ek_comment_nl] '%s' -- at line %d\n",commentMlAccum,ek_yyLineNum);
  std::string newCommLine(commentMlAccum);
  commentMlLines.push_back(newCommLine);
  commentMlAccum[0] = 0;
  ek_newLine();
}

void ek_comment_b0(const char* commStr)
{
  E_DEBUG("* [ek_comment_b0] '%s' -- started at line %d\n",commStr,ek_yyLineNum);

  commentMlAccum[0] = 0;
  strcat(commentMlAccum,commStr);
}

void ek_comment_b1(const char* commStr)
{
  E_DEBUG("* [ek_comment_b1] '%s' -- comment chunk\n",commStr);
  strcat(commentMlAccum,commStr);
}

void ek_comment_b2(const char* commStr)
{
  E_DEBUG("* [ek_comment_b2] '%s' -- ended at line %d\n",commStr,ek_yyLineNum);
  strcat(commentMlAccum,commStr);
  std::string newCommLine(commentMlAccum);
  commentMlLines.push_back(newCommLine);
  commentMlAccum[0] = 0;

  // TODO: push to external handler; pass in vector and line number
#ifdef EKDEBUG
  for (auto it = commentMlLines.begin(); it != commentMlLines.end(); it++)
    printf("* [EK COMMENT]         ----> '%s'\n",it->c_str());
#endif

  commentMlLines.clear();
}
        //----------------------------------------------------------------------
void ek_command_id(const char* commStr)
{
static int ek_command_id_iii = 0;
  sprintf(ek_static_str,"[%d]%s",ek_command_id_iii++,commStr);
  ek_yylval.commandIdStr = ek_static_str;
}

void ek_command_ar(const char* commStr)
{
static char commArgStr[MAX_STR_SIZE];
  strcpy(commArgStr,commStr);
  ek_yylval.commandArgStr = commArgStr;
}

void ek_commandQuote(const char* startStr)
{
  ek_quoteStr(startStr);
  //ek_yylval.commandCollect = 1;
}
        //----------------------------------------------------------------------
void ek_xml_start(const char* xmlStr)
{
  stringNewline2LineNum(xmlStr);

  E_DEBUG("* [ek_xml_start] '%s' line %d\n",xmlStr,ek_yyLineNum);

  // Eat '<' and whitespace, and ignore trailing whitespace
  int dstIdx=0, srcIdx=1;
  while(1) {
    int c = xmlStr[srcIdx];
    if (!isspace(c))
      break;
    if (c == 0)
      ek_yyerror("This state supposedly unreachable! [ek_xml_start()]!");
    srcIdx++;
  }
  while(1) {
    ek_static_str[dstIdx++] = xmlStr[srcIdx++];
    if (xmlStr[srcIdx] == 0)     break;
    if (isspace(xmlStr[srcIdx])) break;
  }
  ek_static_str[dstIdx] = 0;

  ek_yylval.xmlTagId = ek_static_str;
}

void ek_yo(void) //-- for targeted debugging
{
  printf("[ek_yo]!\n");
}

int ek_xml_match(const char* xmlStr)
{
  // Extract TAG in /TAG string
  int srcIdx=1;
  while(1) {
    int c = xmlStr[srcIdx];
    if (!isspace(c))
      break;
    if (c == 0)
      ek_yyerror("This state supposedly unreachable! [ek_xml_match()]!");
    srcIdx++;
  }
  // The next character must be a '/'
  if (xmlStr[srcIdx++] != '/')
    return 0;

  const char* tagStart = &xmlStr[srcIdx];
  E_DEBUG("* [ek_xml_match] '%s' at line %d\n",tagStart,ek_yyLineNum);
//if (strstr(tagStart,"gandalf")) {
//  debug_aaaa = 1;
//  ek_yo();
//}
  return strstr(tagStart,ek_static_str) == tagStart ? 1 : 0;
}

        /*--------------------------------------------------------------------*/

void ek_quoteStr(const char* startStr)
{
  E_DEBUG("BEGIN ek_quoteStr('%s')\n",startStr);

  // TODO: accumulate unlimited-sized strings

  // Quote string handler
  char curLine[4096];
  int  curIdx = 0, c;

  for ( ; ; )
  {
    c = yyinput();
    if (c == '\"') {
      curLine[curIdx] = 0;
      break;
    } else if (c == EOF) {
      ek_yyerror("Unexpected EOF encountered while parsing block quotes");
      //yyterminate();
      return;
    } else if (c == '\n') {
      ek_yyerror("Unexpected newline '\n' found while parsing a string");
      ek_newLine();
    } else if (c == '\\') {
      // C-style escape sequences preceeded by '\'
      // Reference: http://en.cppreference.com/w/cpp/language/escape
      c = yyinput();
      switch (c)
      {
      case '\'':
      case '\"':
      case '\?':
      case '\\':
        break;
      case '\n':
        c = 0x0a;
        ek_newLine();
        break;
      case 'a': c = 0x07; break;
      case 'b': c = 0x08; break;
      case 'f': c = 0x0c; break;
      case 'r': c = 0x0d; break;
      case 't': c = 0x09; break;
      case 'v': c = 0x0b; break;
      case 'n': c = 0x0a; break;
      /*
        The following are not yet supported
        case 'x':         // -- Hex value
        case '0' - '9':   // -- Octal value
        case 'u','U':     // -- Unicode value
      */
      default:
        curLine[curIdx++] = '\\';
      }
    }
    curLine[curIdx++] = c;
  }

  E_DEBUG("END ek_quoteStr --> ~%s~ line num: %d\n",curLine,ek_yyLineNum);
}

        /*--------------------------------------------------------------------*/

void stringNewline2LineNum(const char* readStr)
{
  for (int iii=0; readStr[iii] != 0; iii++)
    if ('\n' == readStr[iii])
      ek_newLine();
}

        /*--------------------------------------------------------------------*/

int yywrap(void)
{
  std::cout << "[yywrap]\n";
  return 1;
}

void ek_keepCompilerHappy(void)
{
  std::cout << yy_top_state() << std::endl;
  yyunput('x',yytext);
}


