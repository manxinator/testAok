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
#include <cctype>
using namespace std;
using namespace aok_tools;

//------------------------------------------------------------------------------

bool aok_tools::str2int(const string &valStr, int &retVal)
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
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool aok_tools::str2strVec(const string &valStr, vector<string> &r_vs, const string &opSep, bool stripEnc)
{
  string workStr;
  int    addedEnt = 0;

  if (stripEnc) {
    int  i = 0, j = 0;
    char endDelim = 0;
    for ( ; i < (int)valStr.length(); i++) {
      char c = valStr[i];
      if      (std::isspace(c)) continue;
      else if (c == '(')        endDelim = ')';
      else if (c == '[')        endDelim = ']';
      else if (c == '{')        endDelim = '}';
      break;
    }
    if (endDelim != 0)
    {
      if (i+1 < (int)valStr.length()) {
        for (j=valStr.length()-1; j >= i; j--) {
          char c = valStr[j];
          if      (std::isspace(c)) continue;
          else if (c == endDelim)   workStr = valStr.substr(i+1,j-(i+1));
          else                      workStr = valStr.substr(i,j+1-i);
          j = -2;
          break;
        }
      } else {
        j = -1;
        workStr = valStr.substr(i);
      }
    }
    else
    {
      for (j=valStr.length()-1; j >= i; j--) {
        if (std::isspace(valStr[j])) continue;
        break;
      }
      workStr = valStr.substr(i,j+1-i);
    }
    //printf("workStr: i: %2d, j: %2d, delim: %2xh -- '%s'\n",i,j,(int)endDelim,workStr.c_str());
  } else
    workStr = valStr;

  if (workStr.length() < 1)
    printf("r_vs.size(): %2d\n",r_vs.size());

  if (workStr.length() < 1)
    return false;

  string delStr;
  if (opSep == "") delStr = "[,;.] \t\n";
  else             delStr = opSep;

  // Call tokenizer
  //
  string      tokStr;
  tokenizer_c tok(workStr,delStr);
  while (tok.getTok(tokStr)) {
    r_vs.push_back(tokStr);
    addedEnt++;
  }
/*
  printf("r_vs.size(): %2d",r_vs.size());
  for (auto oneStr : r_vs)
    printf(", '%s'",oneStr.c_str());
  printf("\n");
*/
  return true;
}
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
tokenizer_c::tokenizer_c(const string &targetStr, const string &delimStr)
{
  wrkStr = targetStr;
  delStr = delimStr;
}

bool tokenizer_c::getTok(string &tokStr)
{
  tokStr.clear();
  while (tokStr.length() < 1) {
    auto pos = wrkStr.find_first_of(delStr);
    if (pos == string::npos) {
      if (wrkStr.length() > 0) {
        tokStr = wrkStr;
        wrkStr.clear();
        return true;
      }
      return false;
    }
    tokStr = wrkStr.substr(0,pos);
    wrkStr = wrkStr.substr(pos+1);
  }
  return true;
}

//------------------------------------------------------------------------------

//
// AOK Interface -- Temporarily placed here!
//
#include "aokIntf.h"

uint32_t aokIntf_c::getU32(const string& obj, const string& knob, uint32_t def, bool* rc)   { return f_getU32(obj,knob,def,rc  ); }
void     aokIntf_c::setU32(const string& obj, const string& knob, uint32_t val, int   mode) {        f_setU32(obj,knob,val,mode); }
int      aokIntf_c::getI32(const string& obj, const string& knob, int      def, bool* rc)   { return f_getI32(obj,knob,def,rc  ); }
void     aokIntf_c::setI32(const string& obj, const string& knob, int      val, int   mode) {        f_setI32(obj,knob,val,mode); }

string aokIntf_c::getStr(const string& obj, const string& knob, const string &def, bool* rc) { return f_getStr(obj,knob,def,rc); }
void   aokIntf_c::setStr(const string& obj, const string& knob, const string &val)           {        f_setStr(obj,knob,val   ); }


