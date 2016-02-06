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
* aokTools.cpp
* Author:  manxinator
* Created: Fri Feb  5 22:21:27 PST 2016
*******************************************************************************/

#include "aokTools.h"
using namespace std;
using namespace aok_tools;

//------------------------------------------------------------------------------

bool aok_tools::getIntRC(const string &valStr, int &retVal)
{
  // TODO: support underscores -- but not in the first 2 characters

  /*
    Terribly written! But let's re-write another time
  */

  int state = 0; // 0:decimal, 1:check for hex/oct, 2:hex, 3:oct, -1:string
  int idx = 0;
  for (const char &c : valStr) {
    if ((idx == 0) && (c == '0'))
      state = 1;
    else if ((idx == 1) && (state == 1)) {
      if      ((c == 'x') || (c == 'X')) state = 2;
      else if ((c >= '0') && (c <= '7')) state = 3;
      else                               state = -1;
    }
    else if (state == 2) {
      bool valid = false;
      if      ((c >= '0') && (c <= '9')) valid = true;
      else if ((c >= 'a') && (c <= 'f')) valid = true;
      else if ((c >= 'A') && (c <= 'F')) valid = true;
      if (!valid)
        state = -1;
    }
    else if (state == 3) {
      if ((c < '0') || (c > '7'))
        state = -1; // demote to string
    }
    else
      if ((c < '0') || (c > '9'))
        state = -1; // demote to string
    idx++;
    if (state < 0)
      break;
  }
  if (state < 0)
    return false;
  if (state < 2)
    retVal = atoi(valStr.c_str());
  else if (state == 3)
    retVal = std::stoi( valStr, 0, 8 );
  else if (state == 2)
    sscanf(valStr.c_str()+2,"%x",&retVal);
  return true;
}

//------------------------------------------------------------------------------

