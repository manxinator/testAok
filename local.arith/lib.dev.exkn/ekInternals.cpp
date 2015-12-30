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
#include <sstream>
using namespace std;
using namespace ex_knobs;

//------------------------------------------------------------------------------

  // Lex/Yacc related
extern int  ek_yyLineNum;
extern void ek_yyerror(const char *s);

  // Delegate Interface
  //
function<void(shared_ptr<primCommand_c>)> ex_knobs::ek_command_f;
function<void(shared_ptr<primObject_c>)>  ex_knobs::ek_object_f;
function<void(shared_ptr<primKnob_c>)>    ex_knobs::ek_knob_f;
function<void(shared_ptr<string>,int)>    ex_knobs::ek_comment_sl_f;
function<void(shared_ptr<string>,int)>    ex_knobs::ek_comment_ml_f;
function<void(shared_ptr<primXml_c>)>     ex_knobs::ek_xml_f;

extern void ek_lexInit   (void);
extern void ek_lexCleanup(void);

  // Global primitives
  //
shared_ptr<primCommand_c> prim_command;
shared_ptr<primObject_c>  prim_object;
shared_ptr<primKnob_c>    prim_knob;
shared_ptr<primXml_c>     prim_xml;

  // Global Variables
  //
int ekint_commandLineNum = -1;
int ekint_xmlLineNum     = -1;


//------------------------------------------------------------------------------

void ek_parserInit(void)
{
  prim_command = make_shared<primCommand_c>();
  prim_object  = make_shared<primObject_c>();
  prim_knob    = make_shared<primKnob_c>();
  prim_xml     = make_shared<primXml_c>();

  ek_lexInit();
}

void ek_parserCleanup(void)
{
  ek_lexCleanup();

  prim_command.reset();
  prim_object.reset();
  prim_knob.reset();
  prim_xml.reset();
}

//------------------------------------------------------------------------------


void ek_commandIdent (const char* dbgStr, const char *cmdId)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandIdent] %s\n",ek_yyLineNum,dbgStr,cmdId);
  prim_command->setIdent(cmdId,ekint_commandLineNum);

  // Process command -- for now, just print
  //
  if (ek_command_f)
    ek_command_f(prim_command);
  else
    prim_command->print();

  // After the command has been processed, renew
  //
  prim_command = make_shared<primCommand_c>();
}

void ek_commandArgs (const char* dbgStr, const char *cmdArgs)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandArgs]  %s\n",ek_yyLineNum,dbgStr,cmdArgs);
  prim_command->setArg(cmdArgs,0);
}

void ek_commandQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  //E_DEBUG("[%3d]   +   [%s] [ek_commandQStr]  %s, vector.size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->begin()->c_str(),quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_command->setArg(workStr,1);
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

void ek_commandLiNum(int lineNum)
{
  ekint_commandLineNum = lineNum;
}


//==============================================================================


void ek_objectDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectDone] \n",ek_yyLineNum,dbgStr);

  // Process object -- for now, just print
  //
  if (ek_object_f)
    ek_object_f(prim_object);
  else
    prim_object->print();

  // After the command has been processed, renew
  //
  prim_object = make_shared<primObject_c>();
}

void ek_objectStr (const char* dbgStr, const char *objStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectStr] objStr: %s\n",ek_yyLineNum,dbgStr,objStr);
  prim_object->setStr(objStr,ek_yyLineNum,0);
}

void ek_objectQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [ek_objectQStr] quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
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


//==============================================================================


void ek_knobDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [ek_knobDone] \n",ek_yyLineNum,dbgStr);

  // Process knob -- for now, just print
  //
  if (ek_knob_f)
    ek_knob_f(prim_knob);
  else
    prim_knob->print();

  // After the command has been processed, renew
  //
  prim_knob = make_shared<primKnob_c>();
}

void ek_knobStr (const char* dbgStr, int isRhs, const char *knobStr)
{
  E_DEBUG("[%3d] + [%s] [ek_knobStr] isRhs: %d, knobStr: %s\n",ek_yyLineNum,dbgStr,isRhs,knobStr);
  prim_knob->setStr(knobStr,ek_yyLineNum,0,isRhs);
}

void ek_knobQStr (const char* dbgStr, int isRhs, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [ek_knobQStr] isRhs: %d, quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,isRhs,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_knob->setStr(workStr,ek_yyLineNum,1,isRhs);
}

void ek_knobBTick (const char* dbgStr, int isRhs)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ek_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_knob->setBTick(static_cast<int>(btType),btIdentStr,btParenStr,isRhs);

  E_DEBUG("[%3d] + [%s] [ek_knobBTick] ------> isRhs: %d, identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,isRhs,btIdentStr.c_str(),btType);
}


//==============================================================================


void ek_xmlDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [ek_xmlDone] \n",ek_yyLineNum,dbgStr);

  auto xmlBody = ek_collectXmlBody();
  {
    int strLen = 8;
    for (auto it = xmlBody->begin(); it != xmlBody->end(); it++)
      strLen += it->length() + 2;

    string workStr;
    workStr.reserve(strLen);

      // If first line is nothing but spaces, eliminate
    auto it = xmlBody->begin();
    string &firstStr = *it;
    std::size_t charPos = firstStr.find_first_not_of(" \t\n");
    if (charPos == std::string::npos)
      it++;

    for (; ; ) {
      workStr += *it;
      it++;
      if (it != xmlBody->end())
        workStr += "\n";
      else
        break;
    }
    prim_xml->addBody(workStr);
  }

  // Process XML -- for now, just print
  //
  if (ek_xml_f)
    ek_xml_f(prim_xml);
  else
    prim_xml->print();

  // After the command has been processed, renew
  //
  prim_xml = make_shared<primXml_c>();
}

void ek_xmlLiNum(int lineNum)
{
  ekint_xmlLineNum = lineNum;
}

void ek_xmlStart (const char* dbgStr, const char *xmlId)
{
  E_DEBUG("[%3d] + [%s] [ek_xmlStart] xmlId: %s\n",ek_yyLineNum,dbgStr,xmlId);
  prim_xml->setLineNum(ekint_xmlLineNum);
  prim_xml->ident.assign(xmlId);
}

void ek_xmlStr (const char* dbgStr, const char *argStr)
{
  E_DEBUG("[%3d] + [%s] [ek_xmlStr] argStr: %s\n",ek_yyLineNum,dbgStr,argStr);
  prim_xml->setStr(argStr,0);
}

void ek_xmlQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [ek_xmlQStr] quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_xml->setStr(workStr,1);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


void ek_commentSL(const char* commentStr, int lineNum)
{
  shared_ptr<string> strObj = make_shared<string>(commentStr);
  strObj->resize(strObj->length()-1);
  if (ek_comment_sl_f)
    ek_comment_sl_f(strObj,lineNum);
}

void ek_commentML(vector<string> *commLines, int lineNum)
{
  if (ek_comment_ml_f)
  {
    int strLen = 8;
    for (auto it = commLines->begin(); it != commLines->end(); it++)
      strLen += it->length() + 2;

    shared_ptr<string> strObj = make_shared<string>();
    string &workStr = *strObj.get();
    workStr.reserve(strLen);

    for (auto it = commLines->begin(); ; ) {
      workStr += *it;
      it++;
      if (it != commLines->end())
        workStr += "\n";
      else
        break;
    }

    ek_comment_ml_f(strObj,lineNum);
  }
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

void ex_knobs::spQStrToStr (shared_ptr<vector<string> > quoteStr, string &destPtr)
{
  int strLen = 8;
  for (auto it = quoteStr->begin(); it != quoteStr->end(); it++)
    strLen += it->length() + 2;

  destPtr.reserve(strLen);
  for (auto it = quoteStr->begin(); ; ) {
    destPtr += *it;
    it++;
    if (it != quoteStr->end())
      destPtr += "\n";
    else
      break;
  }
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

void primCommand_c::setArg (const string& arStr, int isQ)
{
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primKnob_c::setLineNum(int lNum)
{
  if (lineNum < 0)
    lineNum = lNum;
}

void primKnob_c::setStr (const string& arStr, int l_lineNum, int isQ, int isRhs)
{
  setLineNum(l_lineNum);
  element_c* l_elem = exkn_str2elem(arStr,isQ);
  if (isRhs)
    rhsLst.push_back(l_elem);
  else
    lhsLst.push_back(l_elem);
}

void primKnob_c::setBTick (int btType, const string& idStr, const string& parenStr, int isRhs)
{
  element_c* l_elem = backTickToElem(btType,idStr,parenStr);
  if (isRhs)
    rhsLst.push_back(l_elem);
  else
    lhsLst.push_back(l_elem);
}

void primKnob_c::print (void)
{
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primXml_c::setLineNum(int lNum)
{
  if (lineNum < 0)
    lineNum = lNum;
}

void primXml_c::setStr (const string& arStr, int isQ)
{
  element_c* l_elem = exkn_str2elem(arStr,isQ);
  optLst.push_back(l_elem);
}

void primXml_c::addBody(const string& bodStr)
{
  elemStr_c *l_elemStr = new elemStr_c ();
  l_elemStr->varStr = bodStr;
  lineLst.push_back( static_cast<elemStr_c*>(l_elemStr) );
}

void primXml_c::print (void)
{
}
//------------------------------------------------------------------------------

