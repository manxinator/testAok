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
function<void(primCommand_c*)> ex_knobs::ek_command_f;  // TODO: make this <void(shared_ptr<...>)>
function<void(primObject_c*)>  ex_knobs::ek_object_f;   // TODO: make this <void(shared_ptr<...>)>

  // Globals
  //
extern int   ek_yyLineNum;
extern char* ek_yytext;

extern void ek_lexInit   (void);
extern void ek_lexCleanup(void);

  // Global primitives
  //
shared_ptr<primCommand_c> prim_command;
shared_ptr<primObject_c>  prim_object;


//------------------------------------------------------------------------------

void ek_parserInit(void)
{
  prim_command = make_shared<primCommand_c>();
  prim_object  = make_shared<primObject_c>();

  ek_lexInit();
}

void ek_parserCleanup(void)
{
  ek_lexCleanup();

  prim_command.reset();
  prim_object.reset();
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

void ek_commandBTick(const char* dbgStr)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ek_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_command->setBTick(static_cast<int>(btType),btIdentStr,btParenStr);

  E_DEBUG("[%3d]   +   [%s] [ek_commandBTick] ------> identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,btIdentStr.c_str(),btType);
}


//==============================================================================


void ek_objectDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectDone] \n",ek_yyLineNum,dbgStr);

  // Process object -- for now, just print
  //
  if (ek_object_f)
    ek_object_f(prim_object.get());
  else
    prim_object->print();

  // After the command has been processed, renew
  //
  prim_object = make_shared<primObject_c>();
}

void ek_objectStr (const char* dbgStr, const char *objStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectStr] \n",ek_yyLineNum,dbgStr);

  prim_object->setStr(objStr,ek_yyLineNum,0);
}

void ek_objectQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectQStr] quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->size());
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

  prim_object->setStr(workStr,ek_yyLineNum,1);
}

void ek_objectBTick (const char* dbgStr)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ek_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_object->setBTick(static_cast<int>(btType),btIdentStr,btParenStr);

  E_DEBUG("[%3d] + [%s] [ek_objectBTick] ------> identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,btIdentStr.c_str(),btType);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
element_c* ex_knobs::exkn_str2elem(const string& myStr, int isQ)
{
  element_c *elem;
  if (isQ) { elem = new elemQStr_c(); static_cast<elemQStr_c*>(elem)->varStr = myStr; }
  else     { elem = new elemStr_c (); static_cast<elemStr_c*> (elem)->varStr = myStr; }
  return elem;
}

element_c* ex_knobs::backTickToElem (int btType, const string& idStr, const string& parenStr)
{
  btickType_e btt = static_cast<btickType_e>(btType);
  switch (btt)
  {
  case BTICK_FUNC: {
      elemFunc_c *btCl = new elemFunc_c();
      btCl->identStr = idStr;
      btCl->parenStr = parenStr;
      return static_cast<element_c*>(btCl);
    }
    break;
  case BTICK_EQN: {
      elemEqn_c *btEq = new elemEqn_c();
      btEq->parenStr = parenStr;
      return static_cast<element_c*>(btEq);
    }
    break;
  case BTICK_EXPANSION_A:
  case BTICK_EXPANSION_B:
  case BTICK_EXPANSION_C:
    {
      elemExp_c *btEx = new elemExp_c();
      btEx->identStr = idStr;
      btEx->parenStr = parenStr;
      return static_cast<element_c*>(btEx);
    }
    break;
  case BTICK_UNDEF:
  default:
    break;
  }
  return static_cast<element_c*>(0);
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
  argLst.push_back(exkn_str2elem(arStr,isQ));
}

void primCommand_c::setBTick(int btType, const string& idStr, const string& parenStr)
{
  argLst.push_back(backTickToElem(btType,idStr,parenStr));
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primObject_c::setLineNum(int lNum)
{
  if (lineNum < 0)
    lineNum = lNum;
}

void primObject_c::setStr (const string& arStr, int l_lineNum, int isQ)
{
  setLineNum(l_lineNum);
  argLst.push_back(exkn_str2elem(arStr,isQ));
}

void primObject_c::setBTick (int btType, const string& idStr, const string& parenStr)
{
  argLst.push_back(backTickToElem(btType,idStr,parenStr));
}

void primObject_c::print (void)
{
}
//------------------------------------------------------------------------------

