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
* ekInternals.cpp
* Author:  manxinator
* Created: Wed Dec 23 01:07:57 PST 2015
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "ekRead.h"
#include "ekInternals.h"
using namespace std;

//------------------------------------------------------------------------------

class primitive_c {
public:
  typedef enum _primitiveType_e_ {
    PRIM_UNDEF   = 0,
    PRIM_COMMAND = 1
  } primitiveType_e;

  primitiveType_e primType;

public:
           primitive_c(primitiveType_e pType) { primType = pType; }
           primitive_c()                      { primType = PRIM_UNDEF; }
  virtual ~primitive_c() { }
};

  // Globals
  //
shared_ptr<primitive_c> prim_command;

extern int   ek_yyLineNum;
extern char* ek_yytext;

//------------------------------------------------------------------------------

void ek_internInit(void)
{
  prim_command = make_shared<primitive_c>(primitive_c::PRIM_COMMAND);
}

//------------------------------------------------------------------------------

void ek_commandIdent (const char* dbgStr, const char *cmdId)
{
  E_DEBUG("[%3d]   +   [%s] [ek_commandIdent] %s\n",ek_yyLineNum,dbgStr,cmdId);

  // After the command has been processed, renew
  //
  prim_command = make_shared<primitive_c>(primitive_c::PRIM_COMMAND);
}

void ek_commandArgs (const char* dbgStr, const char *cmdArgs)
{
  E_DEBUG("[%3d]   +   [%s] [ek_commandArgs]  %s\n",ek_yyLineNum,dbgStr,cmdArgs);
}

void ek_commandQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr)
{
  printf("Here! vector.size(): %d\n",quoteStr->size());

  E_DEBUG("[%3d]   +   [%s] [ek_commandQStr]  %s\n",ek_yyLineNum,dbgStr,quoteStr->begin()->c_str());
}

//------------------------------------------------------------------------------

