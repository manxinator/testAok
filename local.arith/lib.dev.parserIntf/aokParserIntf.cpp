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

class aokParsFile_c;

class aokParserContext_c {
public:
  string                                 topFile; // Assume only one file for now
  map<string,shared_ptr<aokParsFile_c> > fileMap;

  // AA (internal)
  shared_ptr<arithParser_c> arithAnal;
  map<string,string>        varMap;

  // Function DB? -- Plugins

public:
           aokParserContext_c();
  virtual ~aokParserContext_c() { }

  int  getIntVar(const string& varName);
  void setIntVar(const string& varName, int newVal);

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
};

aokParserContext_c::aokParserContext_c()
{
  arithAnal = make_shared<arithParser_c>();
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
    - Class to load one file

    EK
    - command    function
    - object     function
    - knob       function
    - comment_sl function
    - comment_ml function
    - xml        function
  */
#define DECL_ENT_CLASS(_NAME_,_PRIM_,_ET_)    \
  class _NAME_ : public fileEnt_c {           \
  public:                                     \
    shared_ptr<ex_knobs::_PRIM_> p_ent;       \
                                              \
             _NAME_() : fileEnt_c(_ET_) { }   \
    virtual ~_NAME_()                   { }   \
  };
class aokParsFile_c {
public:
  aokParserContext_c *ctx;
  string              fileName;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  typedef enum _entType_e_ {
    ENT_UNDEF   = 0,
    ENT_COMMAND = 1,
    ENT_OBJECT  = 2,
    ENT_KNOB    = 3,
    ENT_XML     = 4,
    ENT_FILE    = 5
  } entType_e;

  class fileEnt_c {
  public:
    entType_e et;

             fileEnt_c(entType_e entT) { et = entT; }
    virtual ~fileEnt_c()               { }
  };

  class entFile_c : public fileEnt_c {
  public:
    string fileName;

             entFile_c() : fileEnt_c(ENT_FILE) { }
    virtual ~entFile_c()                       { }
  };

  DECL_ENT_CLASS(entCmd_c,primCommand_c,ENT_COMMAND);
  DECL_ENT_CLASS(entObj_c,primObject_c, ENT_OBJECT);
  DECL_ENT_CLASS(entKnb_c,primKnob_c,   ENT_KNOB);
  DECL_ENT_CLASS(entXml_c,primXml_c,    ENT_XML);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  vector<fileEnt_c*> entPtrLst; // Entries of the file

public:
           aokParsFile_c(aokParserContext_c *a_ctx) { ctx = a_ctx; }
  virtual ~aokParsFile_c() {
    ctx = 0;  // We don't own this
  }

    // Delegates to wrap up and save the incoming primitives to a vector
    //
  void ek_command(shared_ptr<ex_knobs::primCommand_c> cmdPrim)  { entPtrLst.push_back( new entCmd_c() ); static_cast<entCmd_c*>(entPtrLst.back())->p_ent = cmdPrim;  }
  void ek_object (shared_ptr<ex_knobs::primObject_c>  objPrim)  { entPtrLst.push_back( new entObj_c() ); static_cast<entObj_c*>(entPtrLst.back())->p_ent = objPrim;  }
  void ek_knob   (shared_ptr<ex_knobs::primKnob_c>    knobPrim) { entPtrLst.push_back( new entKnb_c() ); static_cast<entKnb_c*>(entPtrLst.back())->p_ent = knobPrim; }
  void ek_xml    (shared_ptr<ex_knobs::primXml_c>     xmlPrim)  { entPtrLst.push_back( new entXml_c() ); static_cast<entXml_c*>(entPtrLst.back())->p_ent = xmlPrim;  }
  void ek_remSL  (shared_ptr<string>          remStr, int lNum) { }
  void ek_remML  (shared_ptr<string>          remStr, int lNum) { }

    /*
      Connect the delegates and load the file
    */
  int doParse(const string &fnStr)
  {
    fileName = fnStr;

    ex_knobs::ek_command_f    = std::bind(&aokParsFile_c::ek_command,this,std::placeholders::_1);
    ex_knobs::ek_object_f     = std::bind(&aokParsFile_c::ek_object, this,std::placeholders::_1);
    ex_knobs::ek_knob_f       = std::bind(&aokParsFile_c::ek_knob,   this,std::placeholders::_1);
    ex_knobs::ek_comment_sl_f = std::bind(&aokParsFile_c::ek_remSL,  this,std::placeholders::_1, std::placeholders::_2);
    ex_knobs::ek_comment_ml_f = std::bind(&aokParsFile_c::ek_remML,  this,std::placeholders::_1, std::placeholders::_2);
    ex_knobs::ek_xml_f        = std::bind(&aokParsFile_c::ek_xml,    this,std::placeholders::_1);

    int retVal = ex_knobs::ek_readfile(fnStr.c_str(),0);
    if (retVal < 1) {
      stringstream ss;
      ss << "[aokParsFile_c::doParse] Failed to load file: " << fnStr << "!!!";
      ctx->arithAnal->f_errFunc(ss.str());
      return 0;
    }

#ifdef AOKPI_DEBUG
    printf("[aokParsFile_c::doParse] Listing the loaded entries for file='%s'\n",fileName.c_str());
    for (auto it = entPtrLst.begin(); it != entPtrLst.end(); it++) {
      switch ((*it)->et)
      {
      case ENT_COMMAND: { auto eO = static_cast<entCmd_c*>(*it); printf("  [%3d] ENT_COMMAND : %s\n",eO->p_ent->getLineNum(),  eO->p_ent->ident.c_str()); break; }
      case ENT_OBJECT:  { auto eO = static_cast<entObj_c*>(*it); printf("  [%3d] ENT_OBJECT\n",      eO->p_ent->getLineNum()); break;                            }
      case ENT_KNOB:    { auto eO = static_cast<entKnb_c*>(*it); printf("  [%3d] ENT_KNOB  \n",      eO->p_ent->getLineNum()); break;                            }
      case ENT_XML:     { auto eO = static_cast<entXml_c*>(*it); printf("  [%3d] ENT_XML   \n",      eO->p_ent->getLineNum()); break;                            }
      default:
        printf("ERROR: Unexpected condition!!!\n");
      }
    }
    printf("[aokParsFile_c::doParse] Listing done!  ----------------------\n\n");
#endif

    return 1;
  }

  // Processing functions
  //
  int digest (void);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Utility
  string elemVecToStr  (vector<ex_knobs::element_c*> &elemVec);
  string strRemoveEncap(const string& tgtStr);
};

int aokParsFile_c::digest (void)
{
  // Load any include files [Probably can't delegate]
  //
  int vecSize = (int)entPtrLst.size();
  if (vecSize<=0)
    return 1;   // Empty file -- still valid

  for (int iii=0; iii<vecSize; iii++) {
    if (entPtrLst[iii]->et != ENT_COMMAND)
      continue;
    auto primCmd = static_cast<entCmd_c*>(entPtrLst[iii]);
    auto entCmd  = primCmd->p_ent;
    if (entCmd->ident != "include")
      continue;

    // Evaluate into string
    string incFileName = elemVecToStr( entCmd->argLst );

    // Check in ctx to see if file is already loaded, otherwise load the file
    //
    if (ctx->fileMap.end() == ctx->fileMap.find(incFileName)) {
      printf("---> [%3d] include -=%s=- [digest] to be loaded!\n",entCmd->getLineNum(),incFileName.c_str());
      auto oneFile = make_shared<aokParsFile_c>(ctx);
      ctx->fileMap[incFileName] = oneFile;  // Save file into File Map
      if (!oneFile->doParse(incFileName))   // Load the file
        return 0; // ERROR
      if (!oneFile->digest())
        return 0; // ERROR
    } else {
      printf("---> [%3d] include -=%s=- [digest] already loaded!\n",entCmd->getLineNum(),incFileName.c_str());
    }

    // Replace entPtrLst[iii]
    //
    auto newEnt = new entFile_c();
    newEnt->fileName = incFileName;
    entPtrLst[iii] = newEnt;
    delete primCmd;
  }

  // Digest the entire file
  //
  printf("YOYOYOYOYOYO! -- Digesting File %s!!!\n",fileName.c_str());
  for (int iii=0; iii<vecSize; iii++) {
    int entNum = static_cast<int>(entPtrLst[iii]->et);
    printf("  --> entNum: %d\n",entNum);

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
      ctx->arithAnal->f_exitFunc("ERROR\nERROR [aokParsFile_c::elemVecToStr] Not implemented!!!\nERROR\n\n");
      break;
    }
  }
  return retStr;
}
string aokParsFile_c::strRemoveEncap(const string& tgtStr)
{
  string retStr;
  char c = tgtStr[0];
  if ((c == '[') || (c == '\"') || (c == '\'') || (c == '<'))
    retStr = tgtStr.substr(1,tgtStr.size()-2);
  else
    return tgtStr;
  return retStr;
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
  // Propagate functions to AA object
  ctx->arithAnal->f_errFunc   = f_errFunc;
  ctx->arithAnal->f_exitFunc  = f_exitFunc;
  ctx->arithAnal->f_getIntVar = bind(&aokParserContext_c::getIntVar,ctx,placeholders::_1);
  ctx->arithAnal->f_setIntVar = bind(&aokParserContext_c::setIntVar,ctx,placeholders::_1,placeholders::_2);

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

