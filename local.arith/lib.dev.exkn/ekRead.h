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
* ekRead.h
* - Public header file
* Author:  manxinator
* Created: Wed Dec 23 03:06:09 PST 2015
*******************************************************************************/

#ifndef __EK_READ__
#define __EK_READ__

#include <string>
#include <vector>
#include <functional>

//------------------------------------------------------------------------------

namespace ex_knobs
{
  class element_c {
  public:
    typedef enum _elementType_e_ {
      ELEM_UNDEF     = 0,
      ELEM_STRING    = 1,
      ELEM_QSTRING   = 2,
      ELEM_EQUATION  = 3,
      ELEM_FUNCTION  = 4,
      ELEM_EXPANSION = 5
    } elementType_e;
    elementType_e elemType;

  public:
             element_c(elementType_e eType) { elemType = eType; }
    virtual ~element_c() {}
  };

  class elemStr_c : public element_c {
  public:
    std::string varStr;

  public:
             elemStr_c() : element_c(ELEM_STRING) {}
    virtual ~elemStr_c() {}
  };

  class elemQStr_c : public element_c {
  public:
    std::string varStr;
    int         isDoubleQuote;

  public:
             elemQStr_c() : element_c(ELEM_QSTRING) { isDoubleQuote = 1; }
    virtual ~elemQStr_c() {}
  };

  class elemFunc_c : public element_c {
  public:
    std::string identStr;
    std::string parenStr;

  public:
             elemFunc_c() : element_c(ELEM_FUNCTION) { }
    virtual ~elemFunc_c() {}
  };

  class elemEqn_c : public element_c {
  public:
    std::string parenStr;

  public:
             elemEqn_c() : element_c(ELEM_EQUATION) { }
    virtual ~elemEqn_c() {}
  };

  class elemExp_c : public element_c {
  public:
    std::string identStr;
    std::string parenStr;

  public:
             elemExp_c() : element_c(ELEM_EXPANSION) { }
    virtual ~elemExp_c() {}
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primitive_c {
  public:
    typedef enum _primitiveType_e_ {
      PRIM_UNDEF   = 0,
      PRIM_COMMAND = 1
    } primitiveType_e;

    primitiveType_e primType;
    int             lineNum;

  public:
             primitive_c(primitiveType_e pType) { primType = pType; lineNum = -1; }
    virtual ~primitive_c() { }

    virtual void setLineNum(int) = 0;
  };

  class primCommand_c : public primitive_c {
  public:
    std::string ident;

    std::vector<element_c*> argLst;

  public:
             primCommand_c() : primitive_c(PRIM_COMMAND) {}
    virtual ~primCommand_c() {
      for (auto it = argLst.begin(); it != argLst.end(); it++)
        delete *it;
    }

    virtual void setLineNum(int lNum);

    void setIdent(const std::string& idStr, int l_lineNum);
    void setArg  (const std::string& arStr, int l_lineNum, int isQ);

    void setBTick(int btType, const std::string& idStr, const std::string& parenStr);

    void print (void);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  extern int ek_readfile(const char* inFN, int exitOnErr);

  extern std::function<void(primCommand_c*)> ek_command_f;
}

//------------------------------------------------------------------------------

#endif  //__EK_READ__

