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
using namespace ex_knobs;

//------------------------------------------------------------------------------


  // Delegate Interface
  //
function<void(primCommand_c*)> ex_knobs::ek_command_f;

  // Globals
  //
extern int   ek_yyLineNum;
extern char* ek_yytext;

  // Global primitives
  //
shared_ptr<primCommand_c> prim_command;


//------------------------------------------------------------------------------

void ek_internInit(void)
{
  prim_command = make_shared<primCommand_c>();
}

void ek_parserClenup(void)
{
  prim_command.reset();
}

//------------------------------------------------------------------------------

void ek_commandIdent (const char* dbgStr, const char *cmdId)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandIdent] %s\n",ek_yyLineNum,dbgStr,cmdId);
  prim_command->setIdent(cmdId,ek_yyLineNum);

  // Process command -- for now, just print
  //
  //prim_command->print();
  if (ek_command_f)
    ek_command_f(prim_command.get());

  // After the command has been processed, renew
  //
  prim_command = make_shared<primCommand_c>();
}

void ek_commandArgs (const char* dbgStr, const char *cmdArgs)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandArgs]  %s\n",ek_yyLineNum,dbgStr,cmdArgs);
  prim_command->setArg(cmdArgs,ek_yyLineNum,0);
}

void ek_commandQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandQStr]  %s, vector.size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->begin()->c_str(),quoteStr->size());
  int strLen = 8;
  for (auto it = quoteStr->begin(); it != quoteStr->end(); it++)
    strLen += it->length() + 2;

  string workStr;
  workStr.reserve(strLen);
  for (auto it = quoteStr->begin(); ; ) {
    workStr += *it;
    it++;
    if (it != quoteStr->end())
      workStr += "\n";
    else
      break;
  }

  prim_command->setArg(workStr,ek_yyLineNum,1);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primCommand_c::setLineNum(int lNum)
{
  if      (lineNum < 0)    lineNum = lNum;
  else if (lineNum > lNum) lineNum = lNum;
}

void primCommand_c::setIdent(const string& idStr, int l_lineNum)
{
  setLineNum(l_lineNum);
  ident = idStr;
}

void primCommand_c::setArg (const string& arStr, int l_lineNum, int isQ)
{
  setLineNum(l_lineNum);

  element_c *elem;
  if (isQ) { elem = new elemStr_c (); static_cast<elemStr_c*> (elem)->varStr = arStr; }
  else     { elem = new elemQStr_c(); static_cast<elemQStr_c*>(elem)->varStr = arStr; }

  argLst.push_back(elem);
}

void primCommand_c::print (void)
{
  printf("[primCommand_c::print] line: %d { %s",lineNum,ident.c_str());
  for (auto it = argLst.begin(); it != argLst.end(); it++) {
    element_c::elementType_e elem_type = (*it)->elemType;
    switch (elem_type)
    {
    case element_c::ELEM_STRING:  printf(", %s",    static_cast<elemStr_c*> (*it)->varStr.c_str()); break;
    case element_c::ELEM_QSTRING: printf(", \'%s\'",static_cast<elemQStr_c*>(*it)->varStr.c_str()); break;
    default:
      printf(", ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");
}
//------------------------------------------------------------------------------

