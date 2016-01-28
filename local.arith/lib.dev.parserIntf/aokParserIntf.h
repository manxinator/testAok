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
* Author:  tdeloco
* Created: Sat Jan 16 03:10:18 PST 2016
*******************************************************************************/

#ifndef __AOK_PARSER_INTF__
#define __AOK_PARSER_INTF__

#include <string>
#include <memory>
#include <functional>

//------------------------------------------------------------------------------

class aokParserContext_c;

  /***********************************************
  RECONSIDER:
    Two sets of arithParser_c
    1- Internal for #defines
    2- External for AOK equations
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
  int loadFile(const std::string& fileNameStr);

  void setErrFunc ( std::function<void(const std::string&)> );
  void setExitFunc( std::function<void(const std::string&)> );

private:
    // Allows us to abstract implementation details away from user
    // Requires us to implement a proper copy / move constructor -- not for now!
    // Raw pointer for easy debugging -- may be changed after development
  aokParserContext_c *ctx;

  std::function<void(const std::string&)> f_errFunc;
  std::function<void(const std::string&)> f_exitFunc;

  int  checkAttributes(void);
  void prepareContext (void);
};


//------------------------------------------------------------------------------

#endif  //__AOK_PARSER_INTF__

