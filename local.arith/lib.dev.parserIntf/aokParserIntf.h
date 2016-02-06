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
* aokParserIntf.h
* Author:  manxinator
* Created: Sat Jan 16 03:10:18 PST 2016
*******************************************************************************/

#ifndef __AOK_PARSER_INTF__
#define __AOK_PARSER_INTF__

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "ekRead.h"
#include "aokTools.h"

//------------------------------------------------------------------------------

class aokParserContext_c;
class aokParserPlugin_c;

  /***********************************************
  RECONSIDER:
    Two sets of arithParser_c
    1- Internal for #defines       [defines and variables only]
    2- External for AOK equations  [object/knob equations]
  ***********************************************/

  /*
    Parser:
    * Top-level class

    WARN: currently does not conform to rule of 5
  */
class aokParserIntf_c : public std::enable_shared_from_this<aokParserIntf_c> {
public:
           aokParserIntf_c();
  virtual ~aokParserIntf_c();

    // Top-level load function
    // - Returns 1 for success, otherwise 0
  int loadFile (const std::string& fileNameStr);
  int loadFiles(const std::vector<std::string>& fileNameVec);

  void setErrFunc ( std::function<void(const std::string&)> );
  void setExitFunc( std::function<void(const std::string&)> );

  // TODO: add AOK Interfaces -- equivalent to getObjProp, etc.
  //

  void registerPlugin( std::shared_ptr<aokParserPlugin_c> plugin, bool overWrite=false );

private:
    // Allows us to abstract implementation details away from user
    // Requires us to implement a proper copy / move constructor -- not for now!
    // Raw pointer for easy debugging -- may be changed after development
  aokParserContext_c *ctx;

  std::function<void(const std::string&)> f_errFunc;
  std::function<void(const std::string&)> f_exitFunc;

  int  checkAttributes(void);
  void prepareContext (void);

  void connectStandardPlugins (void);
};


  /*
    Parser Plugins:
    * Base class for plugins to the parser interface

    NOTE: Derived classes must conform to Non-Virtual Interface Idiom
          https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Non-Virtual_Interface

    WARN: currently does not conform to rule of 5
  */
class aokParserPlugin_c {
protected:
  std::string idStr, commandStr;
    // ID      - Uniquely identify this class
    // Command - File command to invoke plugin

public:
           aokParserPlugin_c() { }
  virtual ~aokParserPlugin_c() { }

  std::string& getCommandStr(void);
  std::string& getIdStr     (void);

    // Digest processes one entry
    // Returns number of entries processed
    // - 0 means re-process; WARNING: infinite loop
    // - Less than 0 is an error
  int digest(int idx);

    // Delegates -- will be filled in by registerPlugin()
    //
  std::function<void(const std::string&)>     f_errFunc;
  std::function<void(const std::string&)>     f_exitFunc;
  std::function<int (const std::string&)>     f_getIntDef;    // Preproc defines
  std::function<void(const std::string&,int)> f_setIntDef;

  std::function<std::string(const std::string&)>             f_getStrDef;    // Preproc defines
  std::function<void(const std::string&,const std::string&)> f_setStrDef;

  std::function<int(int)>                                    f_processEntry;
  std::function<std::shared_ptr<ex_knobs::primitive_c>(int)> f_getEntry;

    // TODO: Interface to random functions

  bool checkAttributes(void);

private:
  virtual int digest_impl(int idx) = 0;

protected:
  // collect string from a vector of elements
  std::string collectString(std::vector<ex_knobs::element_c*>::iterator &itRef,
                            std::vector<ex_knobs::element_c*>::iterator endRef,
                            bool doError=true, int count=-1);
  // collect elements - takes in ::vector<element_c*>::iterator, count=-1
};


//------------------------------------------------------------------------------

#endif  //__AOK_PARSER_INTF__

