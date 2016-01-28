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
#include "ekRdInternals.h"
#include "ekInternals.h"
#include <stdexcept>
#include <sstream>
using namespace std;
using namespace ex_knobs;

//------------------------------------------------------------------------------

  // Lex/Yacc related
extern int  ek_yyLineNum;
extern void ek_yyerror(const char *s);

  // Delegate Interface
  //
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_command_f;
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_object_f;
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_knob_f;
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_comment_sl_f;
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_comment_ml_f;
function<void(shared_ptr<primitive_c>)> ex_knobs::ek_xml_f;

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


void eki_commandIdent (const char* dbgStr, const char *cmdId)
{
  E_DEBUG("[%3d]   +   [%s] [eki_commandIdent] %s\n",ek_yyLineNum,dbgStr,cmdId);
  prim_command->setLineNum(ekint_commandLineNum);
  prim_command->setIdent(cmdId);

  // Process command -- for now, just print
  //
  if (ek_command_f)
    ek_command_f(prim_command);
  else
    ; //prim_command->print();

  // After the command has been processed, renew
  //
  prim_command = make_shared<primCommand_c>();
}

void eki_commandArgs (const char* dbgStr, const char *cmdArgs)
{
  E_DEBUG("[%3d]   +   [%s] [eki_commandArgs]  %s\n",ek_yyLineNum,dbgStr,cmdArgs);
  prim_command->setStr(cmdArgs,false);
}

void eki_commandQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d]   +   [%s] [eki_commandQStr]  %s, vector.size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->begin()->c_str(),quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_command->setStr(workStr,true);
}

void eki_commandBTick(const char* dbgStr)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ekl_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_command->setBTick(static_cast<int>(btType),btIdentStr,btParenStr);

  E_DEBUG("[%3d]   +   [%s] [eki_commandBTick] ------> identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,btIdentStr.c_str(),btType);
}

void eki_commandLiNum(int lineNum)
{
  ekint_commandLineNum = lineNum;
}


//==============================================================================


void eki_objectDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [eki_objectDone] \n",ek_yyLineNum,dbgStr);

  // Process object -- for now, just print
  //
  if (ek_object_f)
    ek_object_f(prim_object);
  else
    ; //prim_object->print();

  // After the command has been processed, renew
  //
  prim_object = make_shared<primObject_c>();
}

void eki_objectStr (const char* dbgStr, const char *objStr)
{
  E_DEBUG("[%3d] + [%s] [eki_objectStr] objStr: %s\n",ek_yyLineNum,dbgStr,objStr);
  prim_object->setLineNum(ek_yyLineNum);
  prim_object->setStr(objStr,false);
}

void eki_objectQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [eki_objectQStr] quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_object->setLineNum(ek_yyLineNum);
  prim_object->setStr(workStr,true);
}

void eki_objectBTick (const char* dbgStr)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ekl_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_object->setBTick(static_cast<int>(btType),btIdentStr,btParenStr);

  E_DEBUG("[%3d] + [%s] [eki_objectBTick] ------> identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,btIdentStr.c_str(),btType);
}


//==============================================================================


void eki_knobDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [eki_knobDone] \n",ek_yyLineNum,dbgStr);

  // Process knob -- for now, just print
  //
  if (ek_knob_f)
    ek_knob_f(prim_knob);
  else
    ; //prim_knob->print();

  // After the command has been processed, renew
  //
  prim_knob = make_shared<primKnob_c>();
}

void eki_knobStr (const char* dbgStr, int isRhs, const char *knobStr)
{
  E_DEBUG("[%3d] + [%s] [eki_knobStr] isRhs: %d, knobStr: %s\n",ek_yyLineNum,dbgStr,isRhs,knobStr);
  prim_knob->setLineNum(ek_yyLineNum);
  prim_knob->setStr(knobStr,false,isRhs ? true : false);
}

void eki_knobQStr (const char* dbgStr, int isRhs, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [eki_knobQStr] isRhs: %d, quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,isRhs,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_knob->setLineNum(ek_yyLineNum);
  prim_knob->setStr(workStr,true,isRhs ? true : false);
}

void eki_knobBTick (const char* dbgStr, int isRhs)
{
  btickType_e btType = BTICK_UNDEF;
  string      btIdentStr;
  string      btParenStr;

  ekl_collectBTInfo(btIdentStr,btParenStr,btType);
  prim_knob->setBTick(static_cast<int>(btType),btIdentStr,btParenStr,isRhs ? true : false);

  E_DEBUG("[%3d] + [%s] [eki_knobBTick] ------> isRhs: %d, identStr: '%s', btType: %d\n",ek_yyLineNum,dbgStr,isRhs,btIdentStr.c_str(),btType);
}


//==============================================================================


void eki_xmlDone (const char* dbgStr)
{
  E_DEBUG("[%3d] + [%s] [eki_xmlDone] \n",ek_yyLineNum,dbgStr);

  auto xmlBody = ekl_collectXmlBody();
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
    prim_xml->addText(workStr);
  }

  // Process XML
  //
  if (ek_xml_f)
    ek_xml_f(prim_xml);
  else
    ; //prim_xml->print();

  // After the command has been processed, renew
  //
  prim_xml = make_shared<primXml_c>();
}

void eki_xmlLiNum(int lineNum)
{
  ekint_xmlLineNum = lineNum;
}

void eki_xmlStart (const char* dbgStr, const char *xmlId)
{
  E_DEBUG("[%3d] + [%s] [eki_xmlStart] xmlId: %s\n",ek_yyLineNum,dbgStr,xmlId);
  prim_xml->setLineNum(ekint_xmlLineNum);
  prim_xml->setIdent(xmlId);
}

void eki_xmlStr (const char* dbgStr, const char *argStr)
{
  E_DEBUG("[%3d] + [%s] [eki_xmlStr] argStr: %s\n",ek_yyLineNum,dbgStr,argStr);
  prim_xml->addOpt(argStr,false);
}

void eki_xmlQStr (const char* dbgStr, shared_ptr<vector<string> > quoteStr)
{
  E_DEBUG("[%3d] + [%s] [eki_xmlQStr] quoteStr->size(): %d\n",ek_yyLineNum,dbgStr,quoteStr->size());
  string workStr;
  spQStrToStr(quoteStr,workStr);
  prim_xml->addOpt(workStr,true);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


void eki_commentSL(const char* commentStr, int lineNum)
{
  if (ek_comment_sl_f) {
    string strObj(commentStr);
    strObj.resize(strObj.length()-1);

    auto dlgPtr = make_shared<primCommentSL_c>();
    dlgPtr->setComment(strObj);
    dlgPtr->setLineNum(lineNum);

    shared_ptr<primitive_c> passPtr = dlgPtr;
    ek_comment_sl_f(passPtr);
  }
}

void eki_commentML(vector<string> *commLines, int lineNum)
{
  if (ek_comment_ml_f)
  {
    auto dlgPtr = make_shared<primCommentML_c>();
    dlgPtr->setComment(*commLines);
    dlgPtr->setLineNum(lineNum);

    shared_ptr<primitive_c> passPtr = dlgPtr;
    ek_comment_ml_f(passPtr);
  }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
element_c* ex_knobs::exkn_str2elem(const string& myStr, bool isQ)
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
shared_ptr<primitive_c> ex_knobs::primitive_factory (primitiveType_e primType)
{
  switch (primType)
  {
    case PRIM_COMMAND:    return make_shared<primCommand_c>();
    case PRIM_OBJECT:     return make_shared<primObject_c>();
    case PRIM_KNOB:       return make_shared<primKnob_c>();
    case PRIM_XML:        return make_shared<primXml_c>();
    case PRIM_COMMENT_SL: return make_shared<primCommentSL_c>();
    case PRIM_COMMENT_ML: return make_shared<primCommentML_c>();
    case PRIM_FILE:       return make_shared<primFile_c>();
    default: break;
  }
  throw std::invalid_argument("Unknown argument to ex_knobs::primitive_factory()");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primitive_c::setLineNum(int lNum)
{
  virt_setLineNum(lNum);
}

int primitive_c::getLineNum(void)
{
  return lineNum;
}

primitiveType_e primitive_c::getPrimitiveType(void)
{
  return primType;
}

void primitive_c::setIdent    (const string &idStr)           {        virt_setIdent  (idStr); }
bool primitive_c::getIdentRC  (string &idStr)                 { return virt_getIdentRC(idStr); }
void primitive_c::addOpt      (const string& optStr, bool qt) {        virt_addOpt    (optStr,qt); }
void primitive_c::addText     (const string& txtStr)          {        virt_addText   (txtStr);  }
void primitive_c::setComment  (const string &commStr)         {        virt_setComment(commStr); }
void primitive_c::setComment  (const vector<string> &commLst) {        virt_setComment(commLst); }
bool primitive_c::getCommentRC(string &retStr)                { return virt_getComment(retStr);  }
vector<element_c*>* primitive_c::getArgListRC(bool &rc)       { return virt_getArgListRC(rc); }
vector<element_c*>* primitive_c::getLhsListRC(bool &rc)       { return virt_getLhsListRC(rc); }
vector<element_c*>* primitive_c::getRhsListRC(bool &rc)       { return virt_getRhsListRC(rc); }
vector<element_c*>* primitive_c::getOptionsRC(bool &rc)       { return virt_getOptionsRC(rc); }
vector<element_c*>* primitive_c::getTextRC   (bool &rc)       { return virt_getTextRC   (rc); }
void primitive_c::setFileName (const string &newFileName)     {        virt_setFileName(newFileName); }
bool primitive_c::getFileName (string &retStr)                { return virt_getFileName(retStr); }
void primitive_c::setStr  (const string& arStr, bool isQ, bool isRhs)                           { virt_setStr(arStr,isQ,isRhs);               }
void primitive_c::setBTick(int btType, const string& idStr, const string& parenStr, bool isRhs) { virt_setBTick(btType,idStr,parenStr,isRhs); }

void primitive_c::virt_setLineNum(int lNum)
{
  if (lineNum < 0)
    lineNum = lNum;
}
void primitive_c::virt_setIdent  (const string &idStr)           { }
bool primitive_c::virt_getIdentRC(string &idStr)                 { return false; }
void primitive_c::virt_addOpt    (const string& optStr, bool qt) { }
void primitive_c::virt_addText   (const string& txtStr)          { }
void primitive_c::virt_setComment(const string &commStr)         { }
void primitive_c::virt_setComment(const vector<string> &commLst) { }
bool primitive_c::virt_getComment(string &retStr)                { return false; }
vector<element_c*>* primitive_c::virt_getArgListRC(bool &rc)     { rc = false; return NULL; }
vector<element_c*>* primitive_c::virt_getLhsListRC(bool &rc)     { rc = false; return NULL; }
vector<element_c*>* primitive_c::virt_getRhsListRC(bool &rc)     { rc = false; return NULL; }
vector<element_c*>* primitive_c::virt_getOptionsRC(bool &rc)     { rc = false; return NULL; }
vector<element_c*>* primitive_c::virt_getTextRC   (bool &rc)     { rc = false; return NULL; }
void primitive_c::virt_setFileName(const string &newFileName)    { }
bool primitive_c::virt_getFileName(string &retStr)               { return false; }
void primitive_c::virt_setStr    (const string& arStr, bool isQ, bool isRhs)                           { }
void primitive_c::virt_setBTick  (int btType, const string& idStr, const string& parenStr, bool isRhs) { }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primCommand_c::virt_setLineNum(int lNum)
{
  if      (lineNum < 0)    lineNum = lNum;
  else if (lineNum > lNum) lineNum = lNum;
}

void primCommand_c::virt_setIdent(const string &idStr)
{
  ident = idStr;
}

bool primCommand_c::virt_getIdentRC(string &idStr)
{
  idStr = ident;
  return true;
}

void primCommand_c::virt_setStr(const string& arStr, bool isQ, bool isRhs)
{
  argLst.push_back(exkn_str2elem(arStr,isQ));
}

void primCommand_c::virt_setBTick(int btType, const string& idStr, const string& parenStr, bool isRhs)
{
  argLst.push_back(backTickToElem(btType,idStr,parenStr));
}

vector<element_c*>* primCommand_c::virt_getArgListRC(bool &rc)
{
  rc = true;
  return &argLst;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primObject_c::virt_setStr(const string& arStr, bool isQ, bool isRhs)
{
  argLst.push_back(exkn_str2elem(arStr,isQ));
}

void primObject_c::virt_setBTick(int btType, const string& idStr, const string& parenStr, bool isRhs)
{
  argLst.push_back(backTickToElem(btType,idStr,parenStr));
}

vector<element_c*>* primObject_c::virt_getArgListRC(bool &rc)
{
  rc = true;
  return &argLst;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primKnob_c::virt_setStr(const string& arStr, bool isQ, bool isRhs)
{
  element_c* l_elem = exkn_str2elem(arStr,isQ);
  if (isRhs)
    rhsLst.push_back(l_elem);
  else
    lhsLst.push_back(l_elem);
}

void primKnob_c::virt_setBTick(int btType, const string& idStr, const string& parenStr, bool isRhs)
{
  element_c* l_elem = backTickToElem(btType,idStr,parenStr);
  if (isRhs)
    rhsLst.push_back(l_elem);
  else
    lhsLst.push_back(l_elem);
}

vector<element_c*>* primKnob_c::virt_getLhsListRC(bool &rc)
{
  rc = true;
  return &lhsLst;
}

vector<element_c*>* primKnob_c::virt_getRhsListRC(bool &rc)
{
  rc = true;
  return &rhsLst;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primXml_c::virt_setIdent(const string &idStr)
{
  ident = idStr;
}

bool primXml_c::virt_getIdentRC(string &idStr)
{
  idStr = ident;
  return true;
}

void primXml_c::virt_addOpt(const string& optStr, bool isQ)
{
  element_c* l_elem = exkn_str2elem(optStr,isQ);
  optLst.push_back(l_elem);
}

void primXml_c::virt_addText(const string& bodStr)
{
  elemStr_c *l_elemStr = new elemStr_c ();
  l_elemStr->varStr = bodStr;
  lineLst.push_back( static_cast<elemStr_c*>(l_elemStr) );
}

vector<element_c*>* primXml_c::virt_getOptionsRC(bool &rc)
{
  rc = true;
  return &optLst;
}

vector<element_c*>* primXml_c::virt_getTextRC(bool &rc)
{
  rc = true;
  return &lineLst;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primCommentSL_c::virt_setComment(const string &commStr)
{
  commentStr = commStr;
}

bool primCommentSL_c::virt_getComment(string &retStr)
{
  retStr = commentStr;
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primCommentML_c::virt_setComment(const vector<string> &commLst)
{
  int strLen = 8;
  for (auto it = commLst.begin(); it != commLst.end(); it++)
    strLen += it->length() + 2;

  commentStr.reserve(strLen);

  for (auto it = commLst.begin(); ; ) {
    commentStr += *it;
    it++;
    if (it != commLst.end())
      commentStr += "\n";
    else
      break;
  }
}

bool primCommentML_c::virt_getComment(string &retStr)
{
  retStr = commentStr;
  return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void primFile_c::virt_setFileName(const string &newFileName)
{
  fileName = newFileName;
}

bool primFile_c::virt_getFileName(string &retStr)
{
  retStr = fileName;
  return true;
}
//------------------------------------------------------------------------------

