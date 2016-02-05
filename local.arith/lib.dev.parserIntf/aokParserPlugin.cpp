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
#include "arithmeticAnalyzer.h"
#include "aokParserIntf.h"
#include "ekRead.h"
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

  int internalIdx, startIdx;

  int  initLoop(int idx);
  bool doLoop  (int &idx);
public:
           forLoopPlug_c() { idStr = commandStr = "for"; internalIdx = startIdx = 0; }
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
string& aokParserPlugin_c::getCommandStr(void)    { return commandStr; }
string& aokParserPlugin_c::getIdStr     (void)    { return idStr;      }
int     aokParserPlugin_c::digest       (int idx) { return digest_impl(idx); }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define CL_THROW_RUN_ERR(__MSG__) do {                                  \
  auto className = abi::__cxa_demangle(typeid(*this).name(),0,0,0);     \
  stringstream ssErr;                                                   \
  ssErr << "[" << className << "::" << __FUNCTION__ << "] ERROR: ";     \
  ssErr << __MSG__;                                                     \
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

  string defStr;
  string valStr;

  auto it = argLst->begin();
  if (ex_knobs::ELEM_STRING == (*it)->getElemType())
    defStr = static_cast<ex_knobs::elemStr_c*> (*it)->varStr;
  else
    CL_THROW_NOTIMP();

  it++;
  if (it == argLst->end())
    CL_THROW_RUN_ERR(" define " << defStr << "%s has no value!!!");

  if (ex_knobs::ELEM_STRING == (*it)->getElemType())
    valStr = static_cast<ex_knobs::elemStr_c*> (*it)->varStr;
  else
    CL_THROW_NOTIMP();

  int valInt = atoi(valStr.c_str());
  printf("+ Call f_setIntVar(%s,%d)\n",defStr.c_str(),valInt);
  f_setIntVar(defStr,valInt);

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
       ** Create a list of commands that obey the execute bit
  */

  int numEnts = 0;
  int entIdx = initLoop(idx);
  while (doLoop(entIdx))
  {
    numEnts = 0;
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
        // If execute bit turned off, but identStr in execute bit registry
        //   call processEntry, and continue
      }

      printf("* FOR Processing entry: %d\n",iii);
      iii += f_processEntry(procIdx);
    }
  }
  // TODO: pop execute bit
  printf("* FOR done! numEnts: %d\n",numEnts);
  return numEnts+2;
}
int forLoopPlug_c::initLoop(int idx)
{
  internalIdx = 0;
  startIdx    = idx+1;
  return 1;
}
int AAAAAAAAAAA = 0;
bool forLoopPlug_c::doLoop (int &idx)
{
  idx = startIdx;

  if (AAAAAAAAAAA++ < 2)
    return true;
  else
    return false;
}
//------------------------------------------------------------------------------
/* As a use-case example, add CSV support */

