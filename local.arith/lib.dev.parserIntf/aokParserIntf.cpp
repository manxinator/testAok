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

//------------------------------------------------------------------------------

class aokParsFile_c;

class aokParserContext_c {
public:
  string                                 topFile; // Assume only one file for now
  map<string,shared_ptr<aokParsFile_c> > fileMap;

  // AA (internal)
  shared_ptr<arithParser_c> arithParser;
  map<string,string>        varMap;

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
  arithParser = make_shared<arithParser_c>();
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

  DECL_ENT_CLASS(entCmd_c,primCommand_c,ENT_COMMAND);
  DECL_ENT_CLASS(entObj_c,primObject_c, ENT_OBJECT);
  DECL_ENT_CLASS(entKnb_c,primKnob_c,   ENT_KNOB);
  DECL_ENT_CLASS(entXml_c,primXml_c,    ENT_XML);

  class entFile_c : public fileEnt_c {
  public:
    string                    fileName;
    shared_ptr<aokParsFile_c> parseFile;

             entFile_c() : fileEnt_c(ENT_FILE) { }
    virtual ~entFile_c()                       { }
  };

  vector<fileEnt_c*> entPtrLst;

public:
           aokParsFile_c(aokParserContext_c *a_ctx) { ctx = a_ctx; }
  virtual ~aokParsFile_c() {
    ctx = 0;  // We don't own this
  }

  void ek_command(shared_ptr<ex_knobs::primCommand_c> cmdPrim)  { entPtrLst.push_back( new entCmd_c() ); static_cast<entCmd_c*>(entPtrLst.back())->p_ent = cmdPrim;  }
  void ek_object (shared_ptr<ex_knobs::primObject_c>  objPrim)  { entPtrLst.push_back( new entObj_c() ); static_cast<entObj_c*>(entPtrLst.back())->p_ent = objPrim;  }
  void ek_knob   (shared_ptr<ex_knobs::primKnob_c>    knobPrim) { entPtrLst.push_back( new entKnb_c() ); static_cast<entKnb_c*>(entPtrLst.back())->p_ent = knobPrim; }
  void ek_xml    (shared_ptr<ex_knobs::primXml_c>     xmlPrim)  { entPtrLst.push_back( new entXml_c() ); static_cast<entXml_c*>(entPtrLst.back())->p_ent = xmlPrim;  }
  void ek_remSL  (shared_ptr<string>          remStr, int lNum) { }
  void ek_remML  (shared_ptr<string>          remStr, int lNum) { }

  int doParse(const string &fnStr) {
    fileName = fnStr;

    ex_knobs::ek_command_f    = std::bind(&aokParsFile_c::ek_command,this,std::placeholders::_1);
    ex_knobs::ek_object_f     = std::bind(&aokParsFile_c::ek_object, this,std::placeholders::_1);
    ex_knobs::ek_knob_f       = std::bind(&aokParsFile_c::ek_knob,   this,std::placeholders::_1);
    ex_knobs::ek_comment_sl_f = std::bind(&aokParsFile_c::ek_remSL,  this,std::placeholders::_1, std::placeholders::_2);
    ex_knobs::ek_comment_ml_f = std::bind(&aokParsFile_c::ek_remML,  this,std::placeholders::_1, std::placeholders::_2);
    ex_knobs::ek_xml_f        = std::bind(&aokParsFile_c::ek_xml,    this,std::placeholders::_1);

    int retVal = ex_knobs::ek_readfile(fnStr.c_str(),0);
    if (!retVal) {
      stringstream ss;
      ss << "[aokParsFile_c::doParse] Failed to load file: " << fnStr << "!!!";
      ctx->arithParser->f_errFunc(ss.str());
      return 0;
    }

    printf("[aokParsFile_c::doParse] Listing the loaded entries!\n");
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

    return 1;
  }
};


//------------------------------------------------------------------------------

  /*
    - Top-level class
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

  lock_guard<mutex> load_guard(g_ek_mutex);

  prepareAA();

  // Load first file -- save to map + name of top-level file
  // Go thru file and load includes
  ctx->topFile = fileNameStr;
  auto oneFile = make_shared<aokParsFile_c>(ctx);
  ctx->fileMap[fileNameStr] = oneFile;
  if (!oneFile->doParse(fileNameStr))
    return 0; // ERROR

  return 1;
}

void aokParserIntf_c::prepareAA (void)
{
  // Propagate functions to AA object
  ctx->arithParser->f_errFunc   = f_errFunc;
  ctx->arithParser->f_exitFunc  = f_exitFunc;
  ctx->arithParser->f_getIntVar = bind(&aokParserContext_c::getIntVar,ctx,placeholders::_1);
  ctx->arithParser->f_setIntVar = bind(&aokParserContext_c::setIntVar,ctx,placeholders::_1,placeholders::_2);
}

int aokParserIntf_c::checkAttributes(void)
{
  if (!ctx->arithParser) return 0;
  if (!f_errFunc)        return 0;
  if (!f_exitFunc)       return 0;
  return 1;
}

void aokParserIntf_c::setErrFunc ( function<void(const string&)> a_errFunc )
{
  f_errFunc = a_errFunc;
}

void aokParserIntf_c::setExitFunc( function<void(const string&)> a_exitFunc )
{
  f_exitFunc = a_exitFunc;
}

//------------------------------------------------------------------------------

