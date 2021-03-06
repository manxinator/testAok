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
* aokParserIntf.cpp
* Author:  manxinator
* Created: Sat Jan 16 03:09:41 PST 2016
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "arithmeticAnalyzer.h"
#include "aokParserIntf.h"
#include <mutex>
#include <map>
#include <stack>
#include <sstream>
#include <set>
#include <stdexcept>
#include <cxxabi.h>
using namespace std;

#define AOKPI_DEBUG         1

  // This mutex guards against EK functions. Reasons why it is here:
  // - Flex/Bison are in C and not thread safe
  // - Connecting to EK interface functions must also be protected
  //
mutex g_ek_mutex;

class aokParsFile_c;
class aokParserContext_c;

//------------------------------------------------------------------------------
  // Utility Functions
  //
  int    str2int       (const string &intStr);
  string strRemoveEncap(const string& tgtStr);
//------------------------------------------------------------------------------

class aokParserContext_c {
private:
  // Current object / expander
  string curObj;

  // Defines
  map<string,string> defMap;

  // Plugins / Function DB
  map<string,shared_ptr<aokParserPlugin_c> > plugMap;

  // Exec bit
  stack<bool> ebStack;
  set<string> mbeRegistry;
public:
  map<string,shared_ptr<aokParsFile_c> >
          fileMap;
  string  curParsFile; // index to fileMap
  string  topFile;     // Assume only one file for now

  // AA (internal)
  shared_ptr<arithParser_c> arithAnal;

  // Error Delegates
  std::function<void(const string&)> f_errFunc;
  std::function<void(const string&)> f_exitFunc;

  bool checkAttributes(void);

public:
           aokParserContext_c() { arithAnal = make_shared<arithParser_c>(); }
  virtual ~aokParserContext_c() { }

    // config    - configure this class;
    // preDigest - call before digesting top-level file(s)
  void config ( function<void(const string&)> a_errFunc,
                function<void(const string&)> a_exitFunc );
  void preDigest(const string &curFN);

    // Exec bit
  bool getExecBit(void)      { return ebStack.top();                }
  void popExecBit(void)      { ebStack.pop();                       }
  void pushExecBit(bool set) { ebStack.push(ebStack.top() and set); }

  bool cmdInMBEReg(const string& idStr);

  // Defines -- currently only integer and string (might expand int to long int)
  int  getIntDef(const string& defName);
  void setIntDef(const string& defName, int newVal);

  string getStrDef(const string& defName);
  void   setStrDef(const string& defName, const string& newVal);

  // Get entry at index idx of current file
  shared_ptr<ex_knobs::primitive_c> getEntry(int idx);

  int processEntry(int idx);

  void registerPlugin( shared_ptr<aokParserPlugin_c> plugin, bool overWrite );
};

  /*
    aokParsFile_c
    - Class to abstract a single file

    EK
    - command    function
    - object     function
    - knob       function
    - comment_sl function
    - comment_ml function
    - xml        function
  */
class aokParsFile_c {
private:
  aokParserContext_c *ctx;
  string              fileName;

  vector<shared_ptr<ex_knobs::primitive_c> > fileEntries;

    // Delegates to wrap up and save the incoming primitives to a vector
    //
  void ek_command(shared_ptr<ex_knobs::primitive_c> cmdPrim)  { fileEntries.push_back( cmdPrim  ); }
  void ek_object (shared_ptr<ex_knobs::primitive_c> objPrim)  { fileEntries.push_back( objPrim  ); }
  void ek_knob   (shared_ptr<ex_knobs::primitive_c> knobPrim) { fileEntries.push_back( knobPrim ); }
  void ek_xml    (shared_ptr<ex_knobs::primitive_c> xmlPrim)  { fileEntries.push_back( xmlPrim  ); }
  void ek_remSL  (shared_ptr<ex_knobs::primitive_c> remStr)   { }
  void ek_remML  (shared_ptr<ex_knobs::primitive_c> remStr)   { }

  int loadIncludes(void);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Utility
  string elemVecToStr  (vector<ex_knobs::element_c*> &elemVec);

public:
           aokParsFile_c(aokParserContext_c *a_ctx) { ctx = a_ctx; }
  virtual ~aokParsFile_c() {
    ctx = 0;  // We don't own this
  }

  // Processing functions
  //
  int digest (void);
  int doParse(const string &fnStr);

  string& getFileName(void) { return fileName; }

  shared_ptr<ex_knobs::primitive_c> getEntry(int idx) { return fileEntries[idx]; } // TODO: check if idx > fileEntries.size()
};

//------------------------------------------------------------------------------
/*
  TODO:
  - Standardize when to use throw and when to use error/fail callback
  - Of course, we can always use throw inside the callback
*/
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

void aokParserContext_c::config ( function<void(const string&)> a_errFunc,
                                  function<void(const string&)> a_exitFunc )
{
  f_errFunc  = a_errFunc;
  f_exitFunc = a_exitFunc;
  arithAnal->f_errFunc   = f_errFunc;
  arithAnal->f_exitFunc  = f_exitFunc;
  arithAnal->f_getIntVar = bind(&aokParserContext_c::getIntDef,this,placeholders::_1);
  arithAnal->f_setIntVar = bind(&aokParserContext_c::setIntDef,this,placeholders::_1,placeholders::_2);
}

void aokParserContext_c::preDigest(const string &curFN)
{
  curParsFile = curFN;
  while (!ebStack.empty())
    ebStack.pop();
  ebStack.push(true);
}

bool aokParserContext_c::cmdInMBEReg(const string& idStr)
{
  return mbeRegistry.find(idStr) != mbeRegistry.end();
}

int aokParserContext_c::getIntDef(const string& defName)
{
  int retVal = 0;
  auto it = defMap.find(defName);
  if (it == defMap.end())
    defMap[defName] = "0";
  else
    retVal = str2int(it->second);
  return retVal;
}

void aokParserContext_c::setIntDef(const string& defName, int newVal)
{
  stringstream ssVal;
  ssVal << newVal;
  defMap[defName] = ssVal.str();
}

string aokParserContext_c::getStrDef(const string& defName)
{
  string retVal("__NOT_DEFINED__");
  auto it = defMap.find(defName);
  if (it != defMap.end())
    retVal = defMap[defName];
  return retVal;
}

void aokParserContext_c::setStrDef(const string& defName, const string& newVal)
{
  defMap[defName] = newVal;
}

bool aokParserContext_c::checkAttributes(void)
{
  if (!arithAnal) return 0;
  for (auto &kv : plugMap) { // kv is a single map entry represented as a pair<K,V>
    if (!kv.second->checkAttributes())
      return false;
  }
  return true;
}

shared_ptr<ex_knobs::primitive_c> aokParserContext_c::getEntry(int idx)
{
  return fileMap[curParsFile]->getEntry(idx);  // TODO: check oneEnt not empty
}

void aokParserContext_c::registerPlugin( shared_ptr<aokParserPlugin_c> plugin, bool overWrite )
{
  plugin->arithAnal = arithAnal;

  // Exec bit
  plugin->f_getExecBit  = bind(&aokParserContext_c::getExecBit, this);
  plugin->f_pushExecBit = bind(&aokParserContext_c::pushExecBit,this,placeholders::_1);
  plugin->f_popExecBit  = bind(&aokParserContext_c::popExecBit, this);
  plugin->f_cmdInMBEReg = bind(&aokParserContext_c::cmdInMBEReg,this,placeholders::_1);

  // Remaining Preproc
  plugin->f_getStrDef = bind(&aokParserContext_c::getStrDef,this,placeholders::_1);
  plugin->f_setStrDef = bind(&aokParserContext_c::setStrDef,this,placeholders::_1,placeholders::_2);

  // Add to plugMap
  string &cmdStr = plugin->getCommandStr();
  bool mindExecB = plugin->getMindExecBit();
  if (cmdStr.length() > 0) {
    if ( plugMap.find(cmdStr) != plugMap.end() )
      if (!overWrite)
        return;
    plugMap[cmdStr] = plugin;
    if (mindExecB)
      mbeRegistry.insert(cmdStr);
  } else {
    vector<string> &cmdVec = plugin->getCommandVec();
    for (auto &cmdStr : cmdVec) {
      if ( plugMap.find(cmdStr) != plugMap.end() )
        if (!overWrite)
          continue;
      plugMap[cmdStr] = plugin;
      if (mindExecB)
        mbeRegistry.insert(cmdStr);
    }
  }
}

    void DBG_Command(std::shared_ptr<ex_knobs::primitive_c> cmdPrim)
    {
      bool argRC = false;
      auto argLst = cmdPrim->getArgListRC(argRC);
      if (!argRC)
        throw std::runtime_error("[DBG_Command] ERROR: failed to get argument list!!!\n");

      std::string identStr("ERROR Ident not set!!!");
      if (!cmdPrim->getIdentRC(identStr))
        throw std::runtime_error("[DBG_Command] ERROR: failed to get Ident string!!!\n");

      printf("++++++++++++++++++++> [DBG_Command] line: %d { %s",cmdPrim->getLineNum(),identStr.c_str());
      for (auto elemPtr : *argLst ) {
        ex_knobs::elementType_e elem_type = elemPtr->getElemType();
        switch (elem_type)
        {
        case ex_knobs::ELEM_STRING:   printf(", %s",    static_cast<ex_knobs::elemStr_c*> (elemPtr)->varStr.c_str()); break;
        case ex_knobs::ELEM_QSTRING:  printf(", \'%s\'",static_cast<ex_knobs::elemQStr_c*>(elemPtr)->varStr.c_str()); break;
        case ex_knobs::ELEM_FUNCTION: {
            ex_knobs::elemFunc_c *eBtFn = dynamic_cast<ex_knobs::elemFunc_c*>(elemPtr);
            printf(", elemFunc_c{%s,%s}",eBtFn->identStr.c_str(),eBtFn->parenStr.c_str());
            break;
          }
        case ex_knobs::ELEM_EQUATION: {
            ex_knobs::elemEqn_c *eBtFn = dynamic_cast<ex_knobs::elemEqn_c*>(elemPtr);
            printf(", elemEqn_c{%s}",eBtFn->parenStr.c_str());
            break;
          }
        case ex_knobs::ELEM_EXPANSION: {
            ex_knobs::elemExp_c *eBtEx = dynamic_cast<ex_knobs::elemExp_c*>(elemPtr);
            printf(", elemExp_c{%s,%s}",eBtEx->identStr.c_str(),eBtEx->parenStr.c_str());
            break;
          }
        default:
          printf(", ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
        }
      }
      printf(" }\n");
    }
int aokParserContext_c::processEntry(int idx)
{
  // in processEntry()
  // - if file, then push curParsFile, digest included file
  // - if command, find plugin
  // - if object, process

  auto oneEnt = getEntry(idx);

  int primNo = static_cast<int>(oneEnt->getPrimitiveType());
  printf("  --> [%3d] Primitive Type: %4d [exec: %d]\n",oneEnt->getLineNum(),primNo,ebStack.top());

  int numDigest = 1;
  switch (oneEnt->getPrimitiveType())
  {
  case ex_knobs::PRIM_COMMAND:
    {
      string identStr;
      if (!oneEnt->getIdentRC(identStr))
        CL_THROW_RUN_ERR("failed to get command identifier!!!");
      if (plugMap.find(identStr) == plugMap.end())
        CL_THROW_RUN_ERR("Unknown command: " << identStr);
DBG_Command(oneEnt);
      numDigest = plugMap[identStr]->digest(idx);
    }
    break;
  case ex_knobs::PRIM_OBJECT:
    {
      bool retCode;
      auto lst = oneEnt->getArgListRC(retCode);
      if (!retCode)
        CL_THROW_RUN_ERR("Object didn't return argList!!!");
      //auto itSt  = lst->begin();    // How to handle collect string??
      //auto itEnd = lst->end();
      //curObj = collectString(itSt, itEnd);
      curObj = static_cast<ex_knobs::elemStr_c*>(*lst->begin())->varStr;
      printf("'%s' <-------------- [ex_knobs::PRIM_OBJECT]\n",curObj.c_str());
    }
    break;
  case ex_knobs::PRIM_KNOB:
    {
      bool retCode0, retCode1;
      auto lhsLst = oneEnt->getLhsListRC(retCode0);
      auto rhsLst = oneEnt->getRhsListRC(retCode1);
      if (!retCode0 || !retCode1)
        CL_THROW_RUN_ERR("Knob didn't return LHS List or RHS List!!!");
      //auto itSt  = lst->begin();    // How to handle collect string??
      //auto itEnd = lst->end();
      //curObj = collectString(itSt, itEnd);
      string lhsStr = static_cast<ex_knobs::elemStr_c*>(*lhsLst->begin())->varStr;
      string rhsStr = static_cast<ex_knobs::elemStr_c*>(*rhsLst->begin())->varStr;
      printf("'%s' = '%s' <-------------- [ex_knobs::PRIM_KNOB]\n",lhsStr.c_str(),rhsStr.c_str());
    }
    break;
  case ex_knobs::PRIM_FILE:
    // TODO: Process file here
    break;
  case ex_knobs::PRIM_XML:
    // TODO: Find XML handler
    break;
  case ex_knobs::PRIM_COMMENT_SL:
  case ex_knobs::PRIM_COMMENT_ML:
    // Not handled for now
    break;
  default:
    CL_THROW_RUN_ERR("Unexpected condition!!!");
  }

  return numDigest;
}

//------------------------------------------------------------------------------

int aokParsFile_c::doParse(const string &fnStr)
{
  fileName = fnStr;
    /*
      Connect the delegates and load the file
    */
  ex_knobs::ek_command_f    = std::bind(&aokParsFile_c::ek_command,this,std::placeholders::_1);
  ex_knobs::ek_object_f     = std::bind(&aokParsFile_c::ek_object, this,std::placeholders::_1);
  ex_knobs::ek_knob_f       = std::bind(&aokParsFile_c::ek_knob,   this,std::placeholders::_1);
  ex_knobs::ek_comment_sl_f = std::bind(&aokParsFile_c::ek_remSL,  this,std::placeholders::_1);
  ex_knobs::ek_comment_ml_f = std::bind(&aokParsFile_c::ek_remML,  this,std::placeholders::_1);
  ex_knobs::ek_xml_f        = std::bind(&aokParsFile_c::ek_xml,    this,std::placeholders::_1);

  int retVal = ex_knobs::ek_readfile(fnStr.c_str(),0);
  if (retVal < 1) {
    stringstream ss;
    ss << "[aokParsFile_c::doParse] Failed to load file: " << fnStr << "!!!";
    ctx->f_errFunc(ss.str());
    return 0;
  }

#ifdef AOKPI_DEBUG
  printf("[aokParsFile_c::doParse] Listing the loaded entries for file='%s'\n",fileName.c_str());
  for (auto entPtr : fileEntries) {
    string identStr;
    if (ex_knobs::PRIM_COMMAND == entPtr->getPrimitiveType())
      entPtr->getIdentRC(identStr);
    switch (entPtr->getPrimitiveType())
    {
    case ex_knobs::PRIM_COMMAND: { printf("  [%3d] ENT_COMMAND : %s\n",entPtr->getLineNum(),  identStr.c_str()); break; }
    case ex_knobs::PRIM_OBJECT:  { printf("  [%3d] ENT_OBJECT\n",      entPtr->getLineNum()); break;                    }
    case ex_knobs::PRIM_KNOB:    { printf("  [%3d] ENT_KNOB\n",        entPtr->getLineNum()); break;                    }
    case ex_knobs::PRIM_XML:     { printf("  [%3d] ENT_XML\n",         entPtr->getLineNum()); break;                    }
    default:
      printf("ERROR: Unexpected condition!!!\n");
    }
  }
  printf("[aokParsFile_c::doParse] Listing done!  ----------------------\n\n");
#endif

  return loadIncludes();
}

int aokParsFile_c::loadIncludes (void)
{
  // Load any include files
  //
  int vecSize = (int)fileEntries.size();
  if (vecSize<=0)
    return 1;   // Empty file -- still valid

  for (int iii=0; iii<vecSize; iii++) {
    auto oneEnt = fileEntries[iii];
    if (oneEnt->getPrimitiveType() != ex_knobs::PRIM_COMMAND)
      continue;
    string identStr;
    bool rc = oneEnt->getIdentRC(identStr);
    //
    // TODO: grab #randomSeed here also (global seed / ParserInterface level)
    //
    if (identStr != "include")
      continue;
    if (!rc) {
      stringstream ss;
      ss << "[aokParsFile_c::loadIncludes] this state should be unreachable!";
      ctx->f_errFunc(ss.str());
      return 0;
    }

    // Evaluate arguments into string
    string incFileName;
    {
      auto alRawPtr = oneEnt->getArgListRC(rc);
      if (!rc || !alRawPtr) {
        ctx->f_errFunc("Unexpected #include without a file name!");
        return 0;
      }
      incFileName = elemVecToStr( *alRawPtr );
    }

    // Check in ctx to see if file is already loaded, otherwise load the file
    //
    if (ctx->fileMap.end() == ctx->fileMap.find(incFileName)) {
      printf("---> [%3d] include -=%s=- [loadIncludes] to be loaded!\n",oneEnt->getLineNum(),incFileName.c_str());
      auto oneFile = make_shared<aokParsFile_c>(ctx);
      ctx->fileMap[incFileName] = oneFile;  // Save file into File Map
      if (!oneFile->doParse(incFileName))   // Load the file
        return 0; // f_errFunc() should already have been called
    } else {
      printf("---> [%3d] include -=%s=- [loadIncludes] already loaded!\n",oneEnt->getLineNum(),incFileName.c_str());
    }

    auto filePrim = ex_knobs::primitive_factory(ex_knobs::PRIM_FILE);
    filePrim->setFileName( incFileName );
    filePrim->setLineNum ( oneEnt->getLineNum() );
    fileEntries[iii] = filePrim;
  }

  return 1;
}

int aokParsFile_c::digest (void)
{
  int vecSize = (int)fileEntries.size();
  if (vecSize<=0)
    return 1;   // Empty file -- still valid

  // Digest the entire file
  //
  printf("YOYOYOYOYOYO! -- Digesting File %s!!!\n",fileName.c_str());
  for (int iii=0; iii<vecSize; )
    iii += ctx->processEntry(iii);

  return 1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
string aokParsFile_c::elemVecToStr(vector<ex_knobs::element_c*> &elemVec)
{
  string retStr;
  for (auto it = elemVec.begin(); it != elemVec.end(); it++) {
    ex_knobs::elementType_e elemType = (*it)->getElemType();
    switch (elemType)
    {
    case ex_knobs::ELEM_STRING: {
        auto elem_str = static_cast<ex_knobs::elemStr_c*>(*it);
        retStr += strRemoveEncap(elem_str->varStr);
      }
      break;
    case ex_knobs::ELEM_QSTRING: {
        auto elem_str = static_cast<ex_knobs::elemQStr_c*>(*it);
        retStr += strRemoveEncap(elem_str->varStr);;
      }
      break;
    default:
      ctx->f_exitFunc("ERROR\nERROR [aokParsFile_c::elemVecToStr] Not implemented!!!\nERROR\n\n");
      break;
    }
  }
  return retStr;
}
string strRemoveEncap(const string& tgtStr)
{
  string retStr;
  char c = tgtStr[0];
  if ((c == '[') || (c == '\"') || (c == '\'') || (c == '<'))
    retStr = tgtStr.substr(1,tgtStr.size()-2);
  else
    return tgtStr;
  return retStr;
}
int str2int(const string &intStr)
{
  int retVal;
  try {
    retVal = stoi(intStr,nullptr,0);
  } catch (const out_of_range& oor) {
    // No need to check string length since this exception requires a long string
    // basically, just perform typecasting -- upcast, and then downcast again
    if ((intStr[0] == '0') && ((intStr[1] == 'x') || (intStr[1] == 'X'))) {
      auto ullInt = stoull(intStr,nullptr,0);
      retVal = (int) ullInt;
    } else {
      auto ulInt = stoul(intStr,nullptr,0);
      retVal = (int) ulInt;
    }
  }
  return retVal;
}
//------------------------------------------------------------------------------



  /*
    Parser Interface -- Top-level class
  */

aokParserIntf_c::aokParserIntf_c()  { ctx = new aokParserContext_c(); }
aokParserIntf_c::~aokParserIntf_c() { delete ctx; }

int aokParserIntf_c::loadFile(const string& fileNameStr)
{
  vector<string> fnVec;
  fnVec.push_back( fileNameStr );
  return loadFiles(fnVec);
}

int aokParserIntf_c::loadFiles(const vector<string>& fileNameVec)
{
  /*
    Load files
    1- parse all files
    2- digest all files
  */
  if (!checkAttributes()) {
    if (f_errFunc)
      f_errFunc("  ERROR: [aokParserIntf_c::checkAttributes()] Failed!");
    return 0;
  }
  if (fileNameVec.size() < 1)
    return 0;

  // Set the mutex and prepare to parse
  lock_guard<mutex> load_guard(g_ek_mutex);
  prepareContext();

  ctx->topFile = fileNameVec[0];

  // Load first file -- save to map
  // Go thru file and load includes
  vector<shared_ptr<aokParsFile_c> > parseFileVec;
  for (auto&& oneFNameStr : fileNameVec)
  {
    //string oneFNameStr = *it;
    if (ctx->fileMap.find(oneFNameStr) != ctx->fileMap.end())
      continue;

    auto oneFile = make_shared<aokParsFile_c>(ctx);
    ctx->fileMap[oneFNameStr] = oneFile;
    if (!oneFile->doParse(oneFNameStr))
      return 0; // ERROR
    parseFileVec.push_back(oneFile);
  }

  for (auto filePtr : parseFileVec) {
    ctx->preDigest( filePtr->getFileName() );
    if (!filePtr->digest())
      return 0; // ERROR
  }

  return 1;
}

void aokParserIntf_c::prepareContext (void)
{
  // Propagate functions to Context (which, in turn, propagates to AA object)
  ctx->config( f_errFunc, f_exitFunc );

  connectStandardPlugins();
}

int aokParserIntf_c::checkAttributes(void)
{
  if (!ctx->checkAttributes()) return 0;
  if (!f_errFunc)              return 0;
  if (!f_exitFunc)             return 0;
  // TODO: Interface to Random functions
  // TODO: AOK Interfaces
  return 1;
}

void aokParserIntf_c::registerPlugin( shared_ptr<aokParserPlugin_c> plugin, bool overWrite )
{
  // TODO: move some to aokParserContext_c::registerPlugin (see which ones belong there / here)
  plugin->f_errFunc   = f_errFunc;
  plugin->f_exitFunc  = f_exitFunc;
  plugin->f_getIntDef = ctx->arithAnal->f_getIntVar;
  plugin->f_setIntDef = ctx->arithAnal->f_setIntVar;

  plugin->f_processEntry = bind(&aokParserContext_c::processEntry,ctx,placeholders::_1);
  plugin->f_getEntry     = bind(&aokParserContext_c::getEntry,    ctx,placeholders::_1);

  // TODO: AOK Interfaces
  // TODO: Interface to Random functions

  // Register into context
  ctx->registerPlugin( plugin, overWrite );
}

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void aokParserIntf_c::setErrFunc ( function<void(const string&)> a_errFunc )
{
  f_errFunc = a_errFunc;
}

void aokParserIntf_c::setExitFunc( function<void(const string&)> a_exitFunc )
{
  f_exitFunc = a_exitFunc;
}

//------------------------------------------------------------------------------

