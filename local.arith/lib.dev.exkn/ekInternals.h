/*******************************************************************************
* Copyright (c) 2015 manxinator
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
********************************************************************************
* ekInternals.h
* - Private header file
* Author:  manxinator
* Created: Wed Dec 23 03:09:21 PST 2015
*******************************************************************************/

#ifndef __EK_INTERNALS__
#define __EK_INTERNALS__

#include <memory>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

extern void ek_parserInit   (void);
extern void ek_parserCleanup(void);

  // Command
extern void eki_commandLiNum(int lineNum);
extern void eki_commandIdent(const char* dbgStr, const char *cmdId);
extern void eki_commandArgs (const char* dbgStr, const char *cmdArgs);
extern void eki_commandQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr);
extern void eki_commandBTick(const char* dbgStr);

  // Object
extern void eki_objectDone (const char* dbgStr);
extern void eki_objectStr  (const char* dbgStr, const char *objStr);
extern void eki_objectQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr);
extern void eki_objectBTick(const char* dbgStr);

  // Knob
extern void eki_knobDone (const char* dbgStr);
extern void eki_knobStr  (const char* dbgStr, int isRhs, const char *knobStr);
extern void eki_knobQStr (const char* dbgStr, int isRhs, std::shared_ptr<std::vector<std::string> > quoteStr);
extern void eki_knobBTick(const char* dbgStr, int isRhs);

  // Comments
extern void eki_commentSL(const char* commentStr,              int lineNum);
extern void eki_commentML(std::vector<std::string> *commLines, int lineNum);

  // XML
extern void eki_xmlLiNum(int lineNum);
extern void eki_xmlStart(const char* dbgStr, const char *xmlId);
extern void eki_xmlStr  (const char* dbgStr, const char *xmlStr);
extern void eki_xmlQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr);
extern void eki_xmlDone (const char* dbgStr);


typedef enum _btickType_e_ {
  BTICK_UNDEF       = 0,
  BTICK_EQN         = 1,
  BTICK_FUNC        = 2,
  BTICK_EXPANSION_A = 3,    // ^
  BTICK_EXPANSION_B = 4,    // %
  BTICK_EXPANSION_C = 5     // $
} btickType_e;


  // Collect functions
extern std::shared_ptr<std::vector<std::string> > ekl_collectQStr   (void);
extern std::shared_ptr<std::vector<std::string> > ekl_collectXmlBody(void);

extern void ekl_collectBTInfo (std::string &identStr, std::string &parenStr, btickType_e &btType);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef EKDEBUG
  #define E_DEBUG(...)   printf(__VA_ARGS__)
#else
  #define E_DEBUG(...)
#endif

//------------------------------------------------------------------------------

#endif  //__EK_INTERNALS__

