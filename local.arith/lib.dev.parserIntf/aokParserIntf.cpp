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
class aokParsFile_c {
public:
  aokParserContext_c *ctx;
  string              fileName;

public:
           aokParsFile_c(aokParserContext_c *a_ctx) { ctx = a_ctx; }
  virtual ~aokParsFile_c()                          { ctx = 0;     }

  int doParse(const string &fnStr);

  void ek_command(shared_ptr<ex_knobs::primCommand_c> cmdPrim)  { printf("+++++ [ek_command] line: %3d : %s\n",cmdPrim->getLineNum(),cmdPrim->ident.c_str()); }
  void ek_object (shared_ptr<ex_knobs::primObject_c>  objPrim)  { printf("+++++ [ek_object]  line: %3d\n",     objPrim->getLineNum()); }
  void ek_knob   (shared_ptr<ex_knobs::primKnob_c>    knobPrim) { printf("+++++ [ek_knob]    line: %3d\n",     knobPrim->getLineNum()); }
  void ek_remSL  (shared_ptr<string>          remStr, int lNum) { printf("+++++ [ek_remSL]   line: %3d ~%s~\n",lNum,remStr->c_str()); }
  void ek_remML  (shared_ptr<string>          remStr, int lNum) { printf("+++++ [ek_remML]   line: %3d ~%s~\n",lNum,remStr->c_str()); }
  void ek_xml    (shared_ptr<ex_knobs::primXml_c>     xmlPrim)  { printf("+++++ [ek_xml]     line: %3d Ident: '%s'\n",xmlPrim->getLineNum(),xmlPrim->ident.c_str()); }
};



int aokParsFile_c::doParse(const string &fnStr)
{
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

  return 1;
}


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

