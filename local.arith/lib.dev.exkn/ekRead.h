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

//------------------------------------------------------------------------------

namespace ex_knobs
{
  class primitive_c {
  public:
    typedef enum _primitiveType_e_ {
      PRIM_UNDEF   = 0,
      PRIM_COMMAND = 1
    } primitiveType_e;

    primitiveType_e primType;
    int             lineNum;

  public:
             primitive_c(primitiveType_e pType) { primType = pType;      lineNum = -1; }
             primitive_c()                      { primType = PRIM_UNDEF; lineNum = -1; }
    virtual ~primitive_c() { }

    virtual void setLineNum(int) = 0;
  };

  class primCommand_c : public primitive_c {
  public:
    std::string ident;

    std::vector<std::string> argLst; // -- TODO: convert both of these to element class
    std::vector<int>         argIsQuote;

  public:
             primCommand_c() : primitive_c(PRIM_COMMAND) { }
    virtual ~primCommand_c() {}

    virtual void setLineNum(int lNum);

    void setIdent(const std::string& idStr, int l_lineNum);
    void setArg  (const std::string& idStr, int l_lineNum, int isQ);

    void print (void);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  int ek_readfile(const char* inFN, int exitOnErr);

}

//------------------------------------------------------------------------------

#endif  //__EK_READ__

