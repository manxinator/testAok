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
#include "ekInternals.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

  /*
    Utility Functions
  */
static void ekl_stringNewline2LineNum(const char* readStr);

  /*
    Lex Functions
  */
static void ekl_newLine   (void);
static void ekl_comment_sl(const char* commStr);
static void ekl_comment_b0(const char* commStr);     // Start of comment
static void ekl_comment_b1(const char* commStr);     // Comment chunks
static int  ekl_comment_b2(const char* commStr);     // End of comment,         Returns 1 if a pop occurred
static int  ekl_comment_nl(void);                    // Newline inside comment, Returns 1 to terminate previous state
static void ekl_command_id(const char* commStr);
static void ekl_command_ar(const char* commStr);

static void ekl_xml_start (const char* xmlStr);
static int  ekl_xml_match (const char* xmlStr);      // Returns 1 to terminate XML parsing
static void ekl_xml_arg   (const char* argStr);
static void ekl_xml_argQ  (const char* startStr);
static void ekl_xml_bodySt(void);
static void ekl_xml_acumm (const char* accStr);
static void ekl_xml_newl  (void);

static void ekl_quoteStr    (const char* startStr);
static void ekl_commandQuote(const char* startStr);
static void ekl_objectQuote (const char* startStr);
static void ekl_objectString(const char* idStr   );

static void ekl_knobQuote (const char* startStr);
static void ekl_knob_lhs  (const char* idStr);
static void ekl_knob_stRhs(const char* eqStr);
static void ekl_knob_rhs  (const char* idStr);

static void ekl_btick_start (const char* startStr);
static void ekl_btick_ident (const char* idStr   );
static void ekl_btick_eqn   (const char* parenStr);
static void ekl_btick_lparen(const char* openStr );
static int  ekl_btick_rparen(const char* tailStr );  // Returns 1 to terminate backtick parsing
static void ekl_btick_accum (const char* dataStr );
static void ekl_btick_exp   (const char* typeStr );
static void ekl_btick_expAcc(const char* dataStr );

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
%s      OBJECT
%s      KNOB_LHS
%s      KNOB_RHS
%s      IN_COMMENT_BL
%s      IN_XML_START
%s      IN_XML_BODY
%s      IN_BACKTICK_START
%s      IN_BACKTICK_PAREN
%s      IN_BACKTICK_EXP1
%s      IN_BACKTICK_EXP2

  /*
    Lex Tokens
    - identString / identLParen
      Note:      Identifiers can start with numbers, as well as alphabet and underscore
      Rationale: EK doesn't handle numbers discretely, so everything is considered a string
  */
wsLeading           ^[\t ]+
whitespace          [\t ]+
quotStr_start       \"
identString         [A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*

newLine             [\t ]*\n

backtick            "`"
percent             "%"
colons              ":"+
leftparen           "("
rightparen          ")"
equalsign           \=
identLParen         [A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*[\t ]*"("

commandId           "#"[\t ]*[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*
commandArgs         [^\t\n \"`]+

commentSl           \/\/[^\n]*\n
commentBl_start     \/\*

xml_start           \<[\t \n]*[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*
xml_end             \<[\t \n]*\/[A-Za-z0-9_][A-Za-z0-9_\-\.\/\\]*[\t \n]*\>

/*----------------------------------------------------------------------------*/
%%
  /* Idea:
     - Within INITIAL, save last line number before any state transition -- or maybe always
     - This tells yacc when the last statement was initiated
  */
  /* <INITIAL>{wsLeading}        ; */
<INITIAL>{whitespace}       ;
<INITIAL>{percent}          { BEGIN OBJECT;                                         return OBJ_START;    }
<INITIAL>{commentSl}        {                               ekl_comment_sl(yytext); return ENDL;         }
<INITIAL>{commentBl_start}  { yy_push_state(IN_COMMENT_BL); ekl_comment_b0(yytext);                      }
<INITIAL>{commandId}        { BEGIN COMMAND;                ekl_command_id(yytext); return COMMAND_ID;   }
<INITIAL>{xml_start}        { BEGIN IN_XML_START;           ekl_xml_start (yytext); return XML_TAGID;    }
<INITIAL>{identString}      { BEGIN KNOB_LHS;               ekl_knob_lhs  (yytext); return KNOB_NAME;    }
<INITIAL>{newLine}          {                               ekl_newLine();          return ENDL;         }
<INITIAL>.                  { ek_yyerror("Unexpected token [inside INITIAL state]"); }

  /* NOTE: CommandID is parsed even before entering the COMMAND state */
<COMMAND>{quotStr_start}    { ekl_commandQuote(yytext);                             return COMMAND_QSTR; }
<COMMAND>{commentSl}        { BEGIN INITIAL; ekl_comment_sl(yytext);                return ENDL;         }
<COMMAND>{commentBl_start}  { yy_push_state(IN_COMMENT_BL);     ekl_comment_b0 (yytext);                 }
<COMMAND>{backtick}         { yy_push_state(IN_BACKTICK_START); ekl_btick_start(yytext);                 }
<COMMAND>{commandArgs}      { ekl_command_ar(yytext);                               return COMMAND_ARGS; }
<COMMAND>{whitespace}       ;
<COMMAND>{newLine}          { BEGIN INITIAL; ekl_newLine(); return ENDL; }
  /* <COMMAND>.   --->    unreachable state according to yacc, which is great! */

<OBJECT>{quotStr_start}     { ekl_objectQuote(yytext);           return OBJ_QSTR; }
<OBJECT>{identString}       { ekl_objectString(yytext);          return OBJ_ID;   }
<OBJECT>{colons}            { ekl_objectString(yytext);          return OBJ_ID;   }
<OBJECT>{commentSl}         { BEGIN INITIAL;                    ekl_comment_sl (yytext); return ENDL; }
<OBJECT>{commentBl_start}   { yy_push_state(IN_COMMENT_BL);     ekl_comment_b0 (yytext);              }
<OBJECT>{backtick}          { yy_push_state(IN_BACKTICK_START); ekl_btick_start(yytext);              }
<OBJECT>{whitespace}        ;
<OBJECT>{newLine}           { BEGIN INITIAL; ekl_newLine(); return ENDL; }
<OBJECT>.                   { ek_yyerror("Unexpected token [inside OBJECT state]"); }

  /* Caveat:
     - LHS is not expected to start with a quote, but succeeding names can be quoted strings
     - Comments and newline not allowed until after KNOB_EQ
  */
<KNOB_LHS>{quotStr_start}   {                 ekl_knobQuote(yytext);  return KNOB_QSTR; }
<KNOB_LHS>{identString}     {                 ekl_knob_lhs(yytext);   return KNOB_NAME; }
<KNOB_LHS>{equalsign}       { BEGIN KNOB_RHS; ekl_knob_stRhs(yytext); return KNOB_EQ;   }
<KNOB_LHS>{commentSl}       { ek_yyerror("===> [KNOB_LHS] expecting '=', got comment '//'"); yyterminate(); }
<KNOB_LHS>{commentBl_start} { ek_yyerror("===> [KNOB_LHS] expecting '=', got comment '/*'"); yyterminate(); }
<KNOB_LHS>{backtick}        { yy_push_state(IN_BACKTICK_START); ekl_btick_start(yytext);              }
<KNOB_LHS>{whitespace}      ;
<KNOB_LHS>{newLine}         { ek_yyerror("===> [KNOB_LHS] expecting '=', got newline '\n'"); yyterminate(); }
<KNOB_LHS>.                 { ek_yyerror("Unexpected token [inside KNOB_LHS state]"); }

  /* NOTE:
     - If nothing was defined in the RHS, checking done by Yacc
  */
<KNOB_RHS>{quotStr_start}   {                ekl_knobQuote(yytext); return KNOB_QSTR; }
<KNOB_RHS>{identString}     {                ekl_knob_rhs(yytext);  return KNOB_NAME; }
<KNOB_RHS>{colons}          {                ekl_knob_rhs(yytext);  return KNOB_NAME; }
<KNOB_RHS>{commentSl}       { BEGIN INITIAL;                    ekl_comment_sl (yytext); return ENDL; }
<KNOB_RHS>{commentBl_start} { yy_push_state(IN_COMMENT_BL);     ekl_comment_b0 (yytext);              }
<KNOB_RHS>{backtick}        { yy_push_state(IN_BACKTICK_START); ekl_btick_start(yytext);              }
<KNOB_RHS>{whitespace}      ;
<KNOB_RHS>{newLine}         { BEGIN INITIAL; ekl_newLine();         return ENDL;      }
<KNOB_RHS>.                 { ek_yyerror("Unexpected token [inside KNOB_RHS state]"); }

<IN_COMMENT_BL>{
      /* NOTE: when coming from certain states, succeeding statements should be considered as being in their own line */
    "*/"        { if (ekl_comment_b2(yytext)) { BEGIN INITIAL; return ENDL; } else yy_pop_state();  }
    [^*\n]+     ekl_comment_b1(yytext);
    "*"         ekl_comment_b1(yytext);
      /* Newlines within comment blocks as delimiters to previous state:
         - if previous {int yy_top_state()} is either command, object or knob, terminate those states
           - pop, begin comment, and return endl when comment is done
           - don't terminate within backtick or other states */
    \n          { if (ekl_comment_nl()) { yy_pop_state(); BEGIN IN_COMMENT_BL; } }
    <<EOF>>     { ek_yyerror("Unexpected EOF encountered while parsing comment block"); yyterminate(); }
} /* { ...; } -- NOTE: Suceeding code block has no effect, but I can't find where this is documented in the flex manual */

<IN_XML_START>{
    {quotStr_start}     { ekl_xml_argQ(yytext); return XML_ARGQS; }
    {whitespace}        ;
    \n                  ekl_newLine();
    {commentBl_start}   { yy_push_state(IN_COMMENT_BL); ekl_comment_b0(yytext); }
    ">"                 { ekl_xml_bodySt();    BEGIN(IN_XML_BODY); return XML_ENDTAG; }
    [^\t \n\>\"]+       { ekl_xml_arg(yytext);                     return XML_ARGID;  }
    <<EOF>>             { ek_yyerror("Unexpected EOF encountered while parsing XML start block"); yyterminate(); }
    /* .   --->    unreachable state according to yacc, which is great! */
}
<IN_XML_BODY>{
    {xml_end}   { if (ekl_xml_match(yytext)) { BEGIN INITIAL; return XML_BLOCKTEXT; } else ekl_xml_acumm(yytext); }
    \n          { ekl_xml_newl(); ekl_newLine(); }
    [^\n\<]+    { ekl_xml_acumm(yytext); }
    "<"         { ekl_xml_acumm(yytext); }
    <<EOF>>     { ek_yyerror("Unexpected EOF encountered while parsing XML block"); yyterminate(); }
}

<IN_BACKTICK_START>{
    {identLParen}   { ekl_btick_ident(yytext); BEGIN IN_BACKTICK_PAREN; }
    {leftparen}     { ekl_btick_eqn  (yytext); BEGIN IN_BACKTICK_PAREN; }
    [\%\$]          { ekl_btick_exp  (yytext); BEGIN IN_BACKTICK_EXP1;  }
    \^              { ekl_btick_exp  (yytext); BEGIN IN_BACKTICK_EXP2;  }
    .               { ek_yyerror("===> [IN_BACKTICK_START] unexpected token!"); yyterminate(); }
}
<IN_BACKTICK_PAREN>{
      /* At this time, quote-strings are parsed just like any other character sequence */
    {commentBl_start} { yy_push_state(IN_COMMENT_BL); ekl_comment_b0 (yytext); } /* allow block comments */
    {rightparen}      { if (ekl_btick_rparen(yytext)) { yy_pop_state(); return BTICK_SEQ; } }
    {leftparen}       { ekl_btick_lparen(yytext); }
 /* [^\(\)\n]+        { ekl_btick_accum(yytext);  }  -- this doesn't work, as I couldn't find an exclude for comment block start */
    {identString}     { ekl_btick_accum(yytext); }   /* Forced to use this with '.' */
    \n                { ekl_btick_accum(yytext); ekl_newLine(); }
    .                 { ekl_btick_accum(yytext); }   /* Forced to use the '.' here */
}
<IN_BACKTICK_EXP1>{
      /* Spaces not allowed at expansions */
    [0-9]+          { yy_pop_state(); ekl_btick_expAcc(yytext); return BTICK_SEQ; }
    \([0-9]+\)      { yy_pop_state(); ekl_btick_expAcc(yytext); return BTICK_SEQ; }
    .               { ek_yyerror("===> [IN_BACKTICK_EXP1] unexpected token!"); yyterminate(); }
}
<IN_BACKTICK_EXP2>{
      /* Spaces not allowed at expansions */
    \((\.\.|[0-9|\,])+\)  { yy_pop_state(); ekl_btick_expAcc(yytext); return BTICK_SEQ; }
    .                     { ek_yyerror("Parsing '^(...)', expected list or range (or combination of both) of numbers"); yyterminate(); }
}




%%
/*----------------------------------------------------------------------------*/
#define MAX_STR_SIZE    4096

static char ekl_static_str[MAX_STR_SIZE]; // CommandID, XmlTag, Object, Knob

void ekl_newLine(void)
{
  ek_yyLineNum++;
}

void ekl_comment_sl(const char* commStr)
{
  E_DEBUG("* [ekl_comment_sl] '%s'\n",commStr);
  int oldLineNum = ek_yyLineNum;
  ekl_newLine();
  // push to external handler; pass in string and line number
  eki_commentSL(commStr,oldLineNum);
}

        //----------------------------------------------------------------------

static int                      commentMlLineSt = -1;
static int                      commentMlPopped = 0;
static std::vector<std::string> commentMlLines;
static char                     commentMlAccum[MAX_STR_SIZE];   // TODO: manage and cleanup memory

int ekl_comment_nl(void) // Returns 1 to terminate previous state
{
  int termPrevState = 0;
  int topState = yy_top_state();
  if ((topState == COMMAND) || (topState == OBJECT) || (topState == KNOB_RHS)) {
    termPrevState   = 1;
    commentMlPopped = 1;
  }

#ifdef EKDEBUG
  std::string topStateStr;
  if      (topState == COMMAND)  topStateStr = "COMMAND";
  else if (topState == OBJECT)   topStateStr = "OBJECT";
  else if (topState == KNOB_RHS) topStateStr = "KNOB_RHS";
  else                           topStateStr = std::to_string(topState);
#endif

  E_DEBUG("* [ekl_comment_nl] '%s' -- at line %d; yy_top_state(): %s\n",commentMlAccum,ek_yyLineNum,topStateStr.c_str());
  std::string newCommLine(commentMlAccum);
  commentMlLines.push_back(newCommLine);
  commentMlAccum[0] = 0;
  ekl_newLine();

  return termPrevState;
}

void ekl_comment_b0(const char* commStr)
{
  E_DEBUG("* [ekl_comment_b0] '%s' -- started at line %d\n",commStr,ek_yyLineNum);

  commentMlLineSt   = ek_yyLineNum;
  commentMlAccum[0] = 0;
  strcat(commentMlAccum,commStr);
}

void ekl_comment_b1(const char* commStr)
{
  E_DEBUG("* [ekl_comment_b1] '%s' -- comment chunk\n",commStr);
  strcat(commentMlAccum,commStr);
}

int ekl_comment_b2(const char* commStr) // Returns 1 if a pop occurred
{
  E_DEBUG("* [ekl_comment_b2] '%s' -- ended at line %d; commentMlPopped: %d\n",commStr,ek_yyLineNum,commentMlPopped);
  strcat(commentMlAccum,commStr);
  std::string newCommLine(commentMlAccum);
  commentMlLines.push_back(newCommLine);
  commentMlAccum[0] = 0;

  // push to external handler; pass in vector and line number
  eki_commentML(&commentMlLines,commentMlLineSt);

  commentMlLines.clear();

  int retPop = commentMlPopped;
  commentMlPopped = 0;
  return retPop;
}
        //----------------------------------------------------------------------
void ekl_command_id(const char* commStr)
{
  E_DEBUG("* [ekl_command_id] '%s' line %d\n",commStr,ek_yyLineNum);
  eki_commandLiNum(ek_yyLineNum);
  sprintf(ekl_static_str,"%s",&commStr[1]);
  ek_yylval.commandIdStr = ekl_static_str;
}

void ekl_command_ar(const char* commStr)
{
  E_DEBUG("* [ekl_command_ar] '%s' line %d\n",commStr,ek_yyLineNum);
static char commArgStr[MAX_STR_SIZE];
  strcpy(commArgStr,commStr);
  ek_yylval.commandArgStr = commArgStr;
}

void ekl_commandQuote(const char* startStr)
{
  E_DEBUG("* [ekl_commandQuote] '%s' line %d\n",startStr,ek_yyLineNum);
  ekl_quoteStr(startStr);
}
        //----------------------------------------------------------------------
void ekl_objectQuote(const char* startStr)
{
  E_DEBUG("* [ekl_objectQuote] '%s' line %d\n",startStr,ek_yyLineNum);
  ekl_quoteStr(startStr);
}

void ekl_objectString(const char* idStr)
{
  E_DEBUG("* [ekl_objectString] '%s' line %d\n",idStr,ek_yyLineNum);
  sprintf(ekl_static_str,"%s",idStr);
  ek_yylval.objNameStr = ekl_static_str;
}
        //----------------------------------------------------------------------
void ekl_knobQuote(const char* startStr)
{
  E_DEBUG("* [ekl_objectQuote] '%s' line %d\n",startStr,ek_yyLineNum);
  ekl_quoteStr(startStr);
}

void ekl_knob_lhs (const char* idStr)
{
  E_DEBUG("* [ekl_knob_lhs] '%s' line %d\n",idStr,ek_yyLineNum);
  sprintf(ekl_static_str,"%s",idStr);
  ek_yylval.knobNameStr = ekl_static_str;
}

void ekl_knob_stRhs (const char* eqStr)
{
  E_DEBUG("* [ekl_knob_stRhs] '%s' line %d\n",eqStr,ek_yyLineNum);
}

void ekl_knob_rhs (const char* idStr)
{
  E_DEBUG("* [ekl_knob_rhs] '%s' line %d\n",idStr,ek_yyLineNum);
  sprintf(ekl_static_str,"%s",idStr);
  ek_yylval.knobNameStr = ekl_static_str;
}
        //----------------------------------------------------------------------
static std::shared_ptr<std::vector<std::string> > xmlLines;
static char  xmlAccum[MAX_STR_SIZE];   // TODO: manage and cleanup memory

void ekl_xml_start(const char* xmlStr)
{
  E_DEBUG("* [ekl_xml_start] '%s' line %d\n",xmlStr,ek_yyLineNum);
  eki_xmlLiNum(ek_yyLineNum);
  ekl_stringNewline2LineNum(xmlStr);

  // Eat '<' and whitespace, and ignore trailing whitespace
  int dstIdx=0, srcIdx=1;
  while(1) {
    int c = xmlStr[srcIdx];
    if (!isspace(c))
      break;
    if (c == 0)
      ek_yyerror("This state supposedly unreachable! [ekl_xml_start()]!");
    srcIdx++;
  }
  while(1) {
    ekl_static_str[dstIdx++] = xmlStr[srcIdx++];
    if (xmlStr[srcIdx] == 0)     break;
    if (isspace(xmlStr[srcIdx])) break;
  }
  ekl_static_str[dstIdx] = 0;

  ek_yylval.xmlTagId = ekl_static_str;
}

void ekl_xml_bodySt(void)
{
  xmlAccum[0] = 0;
  xmlLines = decltype(xmlLines)(new std::vector<std::string>);
}

int ekl_xml_match(const char* xmlStr)  // Returns 1 to terminate XML parsing
{
  // Extract TAG in /TAG string
  int srcIdx=1;
  while(1) {
    int c = xmlStr[srcIdx];
    if (!isspace(c))
      break;
    if (c == 0)
      ek_yyerror("This state supposedly unreachable! [ekl_xml_match()]!");
    srcIdx++;
  }
  // The next character must be a '/'
  if (xmlStr[srcIdx++] != '/')
    return 0;

  std::string tagStr(&xmlStr[srcIdx]);
  std::size_t spPos = tagStr.find_first_of(" \t\n>");
  if (spPos != std::string::npos)
    tagStr.resize(spPos);
  E_DEBUG("* [ekl_xml_match] '%s' resized! (check against '%s')\n",tagStr.c_str(),ekl_static_str);
  return tagStr == ekl_static_str ? 1 : 0;
}

void ekl_xml_arg (const char* argStr)
{
  strcpy(xmlAccum,argStr);
  ek_yylval.xmlArg = xmlAccum;
}

void ekl_xml_argQ (const char* startStr)
{
  E_DEBUG("* [ekl_xml_argQ] '%s' line %d\n",startStr,ek_yyLineNum);
  ekl_quoteStr(startStr);
}

void ekl_xml_acumm (const char* accStr)
{
  E_DEBUG("* [ekl_xml_acumm] '%s' line %d\n",accStr,ek_yyLineNum);
  strcat(xmlAccum,accStr);
}

void ekl_xml_newl (void)
{
  xmlLines->push_back(xmlAccum);
  xmlAccum[0] = 0;
}

std::shared_ptr<std::vector<std::string> > ekl_collectXmlBody(void)
{
  if (0 != xmlAccum[0]) {
    // if there is no newline at the end, save it up
    ekl_xml_newl();
  }
  auto temp = xmlLines;
  xmlLines.reset();
  return temp;
}

        /*--------------------------------------------------------------------*/
std::shared_ptr<std::vector<std::string> > ekl_quoteAccum;

void ekl_quoteStr(const char* startStr)
{
  // Renew ekl_quoteAccum
  ekl_quoteAccum = decltype(ekl_quoteAccum)(new std::vector<std::string>);
  std::string curLineStr;

  // Quote string handler
  char curLine[4096];
  int  curIdx = 0, c = '.', prevC;

  for ( ; ; )
  {
    prevC = c;
    c = yyinput();
    if (c == '\"') {
      curLine[curIdx] = 0;
      break;
    } else if (c == EOF) {
      ek_yyerror("Unexpected EOF encountered while parsing block quotes");
      return;
    } else if (c == '\n') {
      ek_yyerror("Unexpected newline '\n' found while parsing a string");
      ekl_newLine();
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
        ekl_newLine();
        curLine[curIdx] = 0; curIdx = 0; // null-terminate, and restart
        curLineStr.assign(curLine);
        ekl_quoteAccum->push_back(curLineStr);
        continue;
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

  if ((curIdx > 0) || (prevC == '\n')) {
    // Check for prevC, otherwise if the last line is
    //   only a quote("), it gets thrown away
    curLine[curIdx] = 0;
    curLineStr.assign(curLine);
    ekl_quoteAccum->push_back(curLineStr);
  }
}

std::shared_ptr<std::vector<std::string> > ekl_collectQStr(void)
{
  auto temp = ekl_quoteAccum;
  ekl_quoteAccum.reset();
  return temp;
}
        /*--------------------------------------------------------------------*/
std::string ekBTickIdentStr;
std::string ekBTickParenStr;
int         ekBTickParenCount;
btickType_e ekBtType = BTICK_UNDEF;

void ekl_btick_start (const char* startStr)
{
#if 0
  ekBTickIdentStr.clear();
  ekBTickParenStr.clear();
#else
  ekBTickParenCount = 0;
  // Assertions
  if (ekBTickIdentStr.length() > 0) ek_yyerror("Assertion failed: ekBTickIdentStr not empty!!!");
  if (ekBTickParenStr.length() > 0) ek_yyerror("Assertion failed: ekBTickParenStr not empty!!!");
#endif
  E_DEBUG("* [ekl_btick_start]  '%s' at line %d\n",startStr,ek_yyLineNum);
}

void ekl_btick_ident (const char* idStr)
{
  ekBTickIdentStr.assign(idStr);
  // Remove trailing '(' and spaces
  //
  for (int idx = ekBTickIdentStr.length()-1; idx>=0; idx--) {
    int c = ekBTickIdentStr[idx];
    if ((c == '(') || (c == ' ') || (c == '\t'))
      continue;
    ekBTickIdentStr.resize(idx+1);
    break;
  }
  ekBtType = BTICK_FUNC;
  E_DEBUG("* [ekl_btick_ident]  '%s' at line %d\n",idStr,ek_yyLineNum);
}

void ekl_btick_eqn (const char* parenStr)
{
  ekBtType = BTICK_EQN;
  E_DEBUG("* [ekl_btick_eqn]    '%s' at line %d\n",parenStr,ek_yyLineNum);
}

int ekl_btick_rparen (const char* tailStr)  // Returns 1 to terminate backtick parsing
{
  E_DEBUG("* [ekl_btick_rparen] '%s' at line %d {ekBTickParenCount: %d}\n",tailStr,ek_yyLineNum,ekBTickParenCount);
  if (ekBTickParenCount > 0) {
    ekBTickParenStr += tailStr;
    ekBTickParenCount--;
    return 0;
  }
  return 1;
}

void ekl_btick_lparen(const char* openStr)
{
  ekBTickParenCount++;
  ekBTickParenStr += openStr;
}

void ekl_btick_accum (const char* dataStr)
{
  E_DEBUG("* [ekl_btick_accum]  '%s' at line %d\n",dataStr,ek_yyLineNum);
  ekBTickParenStr += dataStr;
}

void ekl_btick_exp (const char* typeStr)
{
  switch (typeStr[0])
  {
  case '^': ekBtType = BTICK_EXPANSION_A; ekBTickIdentStr.assign("^"); break;
  case '%': ekBtType = BTICK_EXPANSION_B; ekBTickIdentStr.assign("%"); break;
  case '$': ekBtType = BTICK_EXPANSION_C; ekBTickIdentStr.assign("$"); break;
  default:
    ek_yyerror("This state supposedly unreachable! [ekl_btick_exp()]!");
    break;
  }
  E_DEBUG("* [ekl_btick_exp]    '%s' at line %d ====> '%s'\n",typeStr,ek_yyLineNum,ekBTickIdentStr.c_str());
}

void ekl_btick_expAcc(const char* dataStr)
{
  E_DEBUG("* [ekl_btick_expAcc] '%s' at line %d\n",dataStr,ek_yyLineNum);
  ekBTickParenStr += dataStr;
  if (ekBTickParenStr[0] == '(') {
    int len = ekBTickParenStr.length();
    if (ekBTickParenStr[len-1] != ')') {
      printf("[ekl_btick_expAcc] ERROR! This state supposedly unreachable!\n");
      exit(EXIT_FAILURE);
    }
    ekBTickParenStr = ekBTickParenStr.substr(1,len-2);
  }
}

void ekl_collectBTInfo (std::string &identStr, std::string &parenStr, btickType_e &btType)
{
  btType   = ekBtType;
  identStr = ekBTickIdentStr;
  parenStr = ekBTickParenStr;
  ekBTickIdentStr.clear();
  ekBTickParenStr.clear();
}
        /*--------------------------------------------------------------------*/
void ek_lexInit(void)
{
  ekBTickIdentStr.reserve(4096);
  ekBTickParenStr.reserve(4096);
}

void ek_lexCleanup(void)
{
  ekl_quoteAccum.reset();
}

        /*--------------------------------------------------------------------*/

void ekl_stringNewline2LineNum(const char* readStr)
{
  for (int iii=0; readStr[iii] != 0; iii++)
    if ('\n' == readStr[iii])
      ekl_newLine();
}

        /*--------------------------------------------------------------------*/

int yywrap(void)
{
#ifdef EKDEBUG
  std::cout << "[yywrap]\n";
#endif
  return 1;
}

void ek_keepCompilerHappy(void)
{
  std::cout << yy_top_state() << std::endl;
  yyunput('x',yytext);
}


