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
* aokTools.h
* Author:  manxinator
* Created: Fri Feb  5 22:15:02 PST 2016
*******************************************************************************/

#ifndef __AOK_TOOLS__
#define __AOK_TOOLS__

#include <string>
#include <vector>

//------------------------------------------------------------------------------

namespace aok_tools
{
  bool str2int   (const std::string &valStr, int &retVal); // false means string

  bool str2strVec(const std::string &valStr, std::vector<std::string> &r_vs, const std::string &opSep="", bool stripEnc=true);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class tokenizer_c {
  private:
    std::string wrkStr, delStr;
  public:
    tokenizer_c(const std::string &targetStr, const std::string &delimStr);

    bool getTok(std::string &tokStr);
  };
}

//------------------------------------------------------------------------------

#endif  //__AOK_TOOLS__

