/*******************************************************************************
* Copyright (c) 2016 manxinator
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
* aokParserPlugin.cpp
* Author:  manxinator
* Created: Sat Jan 30 01:02:37 PST 2016
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "aokParserIntf.h"
#include "aokTools.h"
#include "aokIntf.h"
#include <sstream>
#include <cxxabi.h>
using namespace std;


typedef shared_ptr<aokParserPlugin_c> aokParserPluginSPtr;

//------------------------------------------------------------------------------
class definePlug_c : public aokParserPlugin_c {
private:
  virtual int digest_impl(int idx);
public:
           definePlug_c() { idStr = commandStr = "define"; }
  virtual ~definePlug_c() { }
};
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class forLoopPlug_c : public aokParserPlugin_c {
private:
  virtual int digest_impl(int idx);

  int startIdx;

  shared_ptr<arithParser_c::arithEqn_c> checkEqn;
  shared_ptr<arithParser_c::arithEqn_c> iterEqn;

  int  initLoop(int idx);
  bool doLoop  (int &idx);
public:
           forLoopPlug_c() { idStr = commandStr = "for"; startIdx = 0; }
  virtual ~forLoopPlug_c() { }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void aokParserIntf_c::connectStandardPlugins (void)
{
  // overWrite is false, since the user may have registered correspoding plugins
  registerPlugin( static_cast<aokParserPluginSPtr>( make_shared<definePlug_c>()  ), false );
  registerPlugin( static_cast<aokParserPluginSPtr>( make_shared<forLoopPlug_c>() ), false );
}
//------------------------------------------------------------------------------
string& aokParserPlugin_c::getIdStr     (void)    { return idStr;      }
string& aokParserPlugin_c::getCommandStr(void)    { return commandStr; }
int     aokParserPlugin_c::digest       (int idx) { return digest_impl(idx); }

vector<string>& aokParserPlugin_c::getCommandVec     (void) { return getCommandVec_impl(); }
vector<string>& aokParserPlugin_c::getCommandVec_impl(void)
{
static vector<string> aokpp_gcvi_empty;
  return aokpp_gcvi_empty;
}

bool aokParserPlugin_c::checkAttributes(void)
{
  if (!f_errFunc)      return false;
  if (!f_exitFunc)     return false;
  if (!f_getIntDef)    return false;
  if (!f_setIntDef)    return false;
  if (!f_getStrDef)    return false;
  if (!f_setStrDef)    return false;
  if (!f_processEntry) return false;
  if (!f_getEntry)     return false;
  if (!f_getExecBit)   return false;
  if (!f_pushExecBit)  return false;
  if (!f_popExecBit)   return false;
  if (!f_cmdInMBEReg)  return false;
  return true;
}
string aokParserPlugin_c::collectString(vector<ex_knobs::element_c*>::iterator &itRef,
                                        vector<ex_knobs::element_c*>::iterator endRef,
                                        bool doError, int count)
{
  if (itRef == endRef) {
    string noneStr;
    return noneStr;
  }

  /*
    Collect Rules:
    - Combine adjacent strings if both are quoted
    - Non-quoted after non-quoted gets one space
    - if reduced to solitary quoted string, strip quotes, but flag as non-integer
  */


  // Check for homogeneity
  //
  int idx = 0;
  bool homogeneous = true;
  ex_knobs::elementType_e prevElemType = ex_knobs::ELEM_UNDEF;
  for (auto it2 = itRef; it2 != endRef; it2++) {
    auto curElemType = (*it2)->getElemType();
    if ((curElemType != ex_knobs::ELEM_STRING) && (curElemType != ex_knobs::ELEM_QSTRING)) {
      if (doError)
        f_errFunc("[aokParserPlugin_c::collectString] Non-string type found!");
      count = idx;
      break;
    }
    if (idx > 0) {
      if (prevElemType != curElemType)
        homogeneous = false;
    }
    prevElemType = curElemType;
    idx++;
    if (count > 0)
      if (idx >= count)
        break;
  }

  // Collect the string
  //
  idx = 0;
  prevElemType = ex_knobs::ELEM_UNDEF;

  stringstream accumSS;
  for ( ; itRef != endRef; itRef++) {
    auto elemType = (*itRef)->getElemType();
    string curStr;
    if      (ex_knobs::ELEM_STRING  == elemType) curStr = static_cast<ex_knobs::elemStr_c*>  (*itRef)->varStr;
    else if (ex_knobs::ELEM_QSTRING == elemType) curStr = static_cast<ex_knobs::elemQStr_c*> (*itRef)->varStr;

    if (homogeneous)
      accumSS << curStr;
    else {
      if (idx < 1) {
        if (ex_knobs::ELEM_STRING == elemType)
          accumSS << curStr;
        else
          accumSS << '\"' << curStr;
      }
      else if (prevElemType == elemType)
        accumSS << ' ' << curStr;
      else {
        if (ex_knobs::ELEM_STRING == elemType)
          accumSS << "\" " << curStr;
        else
          accumSS << " \"" << curStr;
      }
    }

    prevElemType = elemType;
    idx++;
    if (count > 0)
      if (idx >= count)
        break;
  }
  if ((!homogeneous) && (prevElemType == ex_knobs::ELEM_QSTRING))
    accumSS << '\"';
  return accumSS.str();
}
string aokParserPlugin_c::collectParsExec(vector<ex_knobs::element_c*>::iterator &itRef,
                                          vector<ex_knobs::element_c*>::iterator endRef,
                                          bool doError, int count)
{
  /*
    NOTE: Not implemented at this time
          To simplify the current revision, support STRING and QSTRING only
    Implementation idea:
      - return a vector of parser execution units (vector passed by reference)
        - execute to resolve equations and functions, then may be passed into AA
        - execution unit might contain a single string (or numeric string)
        - execution unit is within aokParserIntf domain -- NOT AOK!
      - delimiter, strip encap char
  */
  return collectString(itRef, endRef, doError, count);
}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define CL_THROW_RUN_ERR(__MSG__) do {                                  \
  auto className = abi::__cxa_demangle(typeid(*this).name(),0,0,0);     \
  stringstream ssErr;                                                   \
  ssErr << "[" << className << "::" << __FUNCTION__ << "] ERROR: ";     \
  ssErr << __MSG__;                                                     \
  ssErr << std::endl;                                                   \
  throw std::runtime_error(ssErr.str());                                \
} while(0)
#define CL_THROW_NOTIMP() CL_THROW_RUN_ERR("not implemented!!!")

//------------------------------------------------------------------------------
int definePlug_c::digest_impl(int idx)
{
  printf("[definePlug_c::digest_impl] idx: %d\n",idx);
  auto cmd_prim = f_getEntry(idx);

  bool argRC = false;
  auto argLst = cmd_prim->getArgListRC(argRC);
  if (!argRC)
    CL_THROW_RUN_ERR("failed to get argument list!!!");

  /*
    Defines are very simple
    - First element must be a string or quoted string
    - Succeeding elements must be collected and reduced to a single string or number
    - Support empty defines? ie: value = ""
  */

  string defStr;
  string valStr;

  auto it = argLst->begin();
  if      (ex_knobs::ELEM_STRING  == (*it)->getElemType()) defStr = static_cast<ex_knobs::elemStr_c*>  (*it)->varStr;
  else if (ex_knobs::ELEM_QSTRING == (*it)->getElemType()) defStr = static_cast<ex_knobs::elemQStr_c*> (*it)->varStr;
  else
    CL_THROW_NOTIMP();

  it++;

  valStr = collectString(it,argLst->end());
  if (valStr.size() < 1)
    CL_THROW_RUN_ERR("#define " << defStr << " not followed by definition! " <<
                     "Empty defines are not supported at this time!");

  // Call set define
  int valInt = 0;
  if (aok_tools::str2int(valStr,valInt)) {
    printf("+ Call f_setIntDef(%s,%d)\n",defStr.c_str(),valInt);
    f_setIntDef(defStr,valInt);
  } else {
    printf("+ Call f_setStrDef(%s,%s)\n",defStr.c_str(),valStr.c_str());
    f_setStrDef(defStr,valStr);
  }

  return 1;
}
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int forLoopPlug_c::digest_impl(int idx)
{
  printf("[forLoopPlug_c::digest_impl] idx: %d\n",idx);

  /*
    IDEA: Feature to prevent processing (not entering loop, continue, or break)
          Implement execute bit
          Go thru normal steps, but do not execute
          Push execute bit in initLoop, pop when exiting
       ** Create a registry of commands that obey the execute bit
  */

  bool execBit = f_getExecBit();
  bool bLoop;

  int numEnts;
  int laPrima = 1;
  int entIdx  = initLoop(idx);
  while ( (bLoop = doLoop(entIdx)) || laPrima )
  {
    numEnts = 0;
    laPrima = 0;
    if (!bLoop || !execBit) {
      execBit = false;
      f_pushExecBit(false);
    }

    if (execBit)
    {
      // Execute for loop contents
      //
      for (int iii=0; ; ) {
        int procIdx = entIdx + iii;

        // check if entry is #endfor
        auto line_prim = f_getEntry(procIdx);
        if (ex_knobs::PRIM_COMMAND == line_prim->getPrimitiveType()) {
          string identStr;
          if (!line_prim->getIdentRC(identStr))
            CL_THROW_RUN_ERR(" failed to get command identifier!!!");
          if ("endfor" == identStr) {
            numEnts = iii;
            break;
          }
        }

        printf("* FOR Processing entry: %d\n",iii);
        iii += f_processEntry(procIdx);
      }
    }
    else
    {
      // Go thru loop but do not execute
      //
      for (int iii=0; ; ) {
        int procIdx = entIdx + iii;

        // check if entry is in Mind Exec Bit registry
        auto line_prim = f_getEntry(procIdx);
        if (ex_knobs::PRIM_COMMAND == line_prim->getPrimitiveType()) {
          string identStr;
          if (!line_prim->getIdentRC(identStr))
            CL_THROW_RUN_ERR(" failed to get command identifier!!!");
          if ("endfor" == identStr) {
            numEnts = iii;
            break;
          }

          // Check registry
          if ( f_cmdInMBEReg(identStr) ) {
            printf("* FOR Mock Processing entry: %d\n",iii);
            iii += f_processEntry(procIdx);
            continue;
          }
        }

        printf("* FOR Skipping entry: %d\n",iii);
        iii++;
      }

      f_popExecBit();
    }
    printf("      @@@ iter: %d\n",iterEqn->computeInt());
  }

  printf("* FOR done! numEnts: %d\n",numEnts);
  return numEnts+2;
}
int forLoopPlug_c::initLoop(int idx)
{
  startIdx = idx+1;

  bool argRC    = false;
  auto cmd_prim = f_getEntry(idx);
  auto argLst   = cmd_prim->getArgListRC(argRC);
  if (!argRC)
    CL_THROW_RUN_ERR("failed to get argument list!!!");

  auto   it     = argLst->begin();
  string valStr = collectParsExec(it,argLst->end());
  vector<string> eqnVec;

  if (!aok_tools::str2strVec(valStr,eqnVec,"(;) \t\n") || (eqnVec.size() != 3))
    CL_THROW_RUN_ERR("for loop failed to parse parentheses statement!");

  printf("[forLoopPlug_c::initLoop] *************** valStr: '%s' -->",valStr.c_str());
  for (auto oneStr : eqnVec) printf(" '%s'",oneStr.c_str());
  printf("\n");

  auto startEqn = arithAnal->parseEqn(eqnVec[0]);
  checkEqn      = arithAnal->parseEqn(eqnVec[1]);
  iterEqn       = arithAnal->parseEqn(eqnVec[2]);

  printf("      @@@ start: %d\n",startEqn->computeInt());

  return 1;
}
bool forLoopPlug_c::doLoop (int &idx)
{
  idx = startIdx;

  int check =
        checkEqn->computeInt();
  printf("      @@@ check: %d\n",check);
  return checkEqn->computeInt() != 0;
}
//------------------------------------------------------------------------------
/*
  Plugins to support:
  - #solve (same as #eval)
  - #if, ifdef, ifndef, elif, elsif, else -- try to support in one class
*/
/* As a use-case example, add CSV support */

