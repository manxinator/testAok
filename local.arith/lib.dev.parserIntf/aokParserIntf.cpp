/*******************************************************************************
* Copyright (c) 2016 tdeloco
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
* Author:  tdeloco
* Created: Sat Jan 16 03:09:41 PST 2016
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <map>
#include <sstream>
#include "arithmeticAnalyzer.h"
#include "aokParserIntf.h"
#include "ekRead.h"
using namespace std;

  // This mutex guards against EK functions. Reasons why it is here:
  // - Flex/Bison are in C and not thread safe
  // - Connecting to EK interface functions must also be protected
  //
mutex g_ek_mutex;

#define AOKPI_DEBUG         1

//------------------------------------------------------------------------------
  // Utility Functions
  //
  int    str2int       (const string &intStr);
  string strRemoveEncap(const string& tgtStr);
//------------------------------------------------------------------------------

class aokParsFile_c;

class aokParserContext_c {
public:
  string                                 topFile; // Assume only one file for now
  map<string,shared_ptr<aokParsFile_c> > fileMap;

  // AA (internal)
  shared_ptr<arithParser_c> arithAnal;
  map<string,string>        varMap;

  // Function DB? -- Plugins

  // Error Delegates
  std::function<void(const std::string&)> f_errFunc;
  std::function<void(const std::string&)> f_exitFunc;

public:
           aokParserContext_c() { arithAnal = make_shared<arithParser_c>(); }
  virtual ~aokParserContext_c() { }

  void config ( function<void(const string&)> a_errFunc,
                function<void(const string&)> a_exitFunc );

  int  getIntVar(const string& varName);
  void setIntVar(const string& varName, int newVal);
};

void aokParserContext_c::config ( function<void(const string&)> a_errFunc,
                                  function<void(const string&)> a_exitFunc )
{
  f_errFunc  = a_errFunc;
  f_exitFunc = a_exitFunc;
  arithAnal->f_errFunc   = f_errFunc;
  arithAnal->f_exitFunc  = f_exitFunc;
  arithAnal->f_getIntVar = bind(&aokParserContext_c::getIntVar,this,placeholders::_1);
  arithAnal->f_setIntVar = bind(&aokParserContext_c::setIntVar,this,placeholders::_1,placeholders::_2);
}

int aokParserContext_c::getIntVar(const string& varName)
{
  int retVal = 0;
  auto it = varMap.find(varName);
  if (it == varMap.end())
    varMap[varName] = "0";
  else
    retVal = str2int(it->second);
  return retVal;
}

void aokParserContext_c::setIntVar(const string& varName, int newVal)
{
  stringstream ssVal;
  ssVal << newVal;
  varMap[varName] = ssVal.str();
}

//------------------------------------------------------------------------------

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
};

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
  for (auto it = fileEntries.begin(); it != fileEntries.end(); it++) {
    auto entPtr = *it;
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
  for (int iii=0; iii<vecSize; iii++) {
    auto oneEnt = fileEntries[iii];
    int  primNo = static_cast<int>(oneEnt->getPrimitiveType());
    printf("  --> [%3d] Primitive Type: %4d\n",oneEnt->getLineNum(),primNo);

    //
    // IDEA: process {cmd, obj, knob} in process class -- so that plugins can use standard functions
    //       xml and file in a {recursive} function within this class
    // Arguments to a Plugins: fileEnt_c, context, proc class, entPtrLst, index
    //

    // Start with #defines -- and find a way to fork this out to a delegate database / plugins
    // Plugins: fileEnt_c, context, entPtrLst, index
    // Create recursive functions for looping
  }

  return 1;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
string aokParsFile_c::elemVecToStr(vector<ex_knobs::element_c*> &elemVec)
{
  string retStr;
  //printf("---> [elemVecToStr] elemVec.size(): %d\n",elemVec.size());
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
  if (!checkAttributes()) {
    if (f_errFunc)
      f_errFunc("  ERROR: [aokParserIntf_c::checkAttributes()] Failed!");
    return 0;
  }

  // Set the mutex and prepare to parse
  lock_guard<mutex> load_guard(g_ek_mutex);
  prepareContext();

  // Load first file -- save to map + name of top-level file
  // Go thru file and load includes
  ctx->topFile = fileNameStr;
  auto oneFile = make_shared<aokParsFile_c>(ctx);
  ctx->fileMap[fileNameStr] = oneFile;  // Save file into File Map
  if (!oneFile->doParse(fileNameStr))
    return 0; // ERROR

  if (!oneFile->digest())
    return 0; // ERROR

  return 1;
}

void aokParserIntf_c::prepareContext (void)
{
  // Propagate functions to Context (which, in turn, propagates to AA object)
  ctx->config( f_errFunc, f_exitFunc );

  // Connect standard plugins
  // - TODO: make plugins modifiable from outside
}

int aokParserIntf_c::checkAttributes(void)
{
  if (!ctx->arithAnal) return 0;
  if (!f_errFunc)      return 0;
  if (!f_exitFunc)     return 0;
  return 1;
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

