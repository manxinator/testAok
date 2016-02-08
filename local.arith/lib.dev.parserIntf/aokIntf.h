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
* aokIntf.h
* Author:  manxinator
* Created: Fri Feb  5 23:10:50 PST 2016
*******************************************************************************/

#ifndef __AOK_INTERFACE__
#define __AOK_INTERFACE__

#include <string>
#include <functional>
#include <cstdint>

//------------------------------------------------------------------------------

class aokIntf_c {
public:
           aokIntf_c() { }
  virtual ~aokIntf_c() { }

  /*
    get / set uint32, return code, mode=dec,octal,hex
    get / set int
    get / set string
    get type -- use enum within aokIntf_c
    delete knob
    delete object
    iteration
    get / set inheritance
    get / set metadata?
    Search for knob?
    Save object / save all to file
  */

  /*
    uint32_t mode: 0=Decimal, 1=Hex, 2=Octal
    TODO: mode should be an aokIntf_c enum
  */

  uint32_t getU32(const std::string& obj, const std::string& knob, uint32_t def=0xFFFFFFFF, bool*rc   = NULL);
  void     setU32(const std::string& obj, const std::string& knob, uint32_t val,            int  mode = 0);
  int      getI32(const std::string& obj, const std::string& knob, int      def=-1,         bool*rc   = NULL);
  void     setI32(const std::string& obj, const std::string& knob, int      val,            int  mode = 0);

  std::string getStr(const std::string& obj, const std::string& knob, const std::string &def = "__NOT_DEFINED__", bool *rc = NULL);
  void        setStr(const std::string& obj, const std::string& knob, const std::string &val);

protected:
  friend class aokIntfConfig_c;

  std::function<uint32_t(const std::string&, const std::string&, uint32_t, bool*)> f_getU32;
  std::function<void    (const std::string&, const std::string&, uint32_t, int  )> f_setU32;
  std::function<int     (const std::string&, const std::string&, int,      bool*)> f_getI32;
  std::function<void    (const std::string&, const std::string&, int,      int  )> f_setI32;

  std::function<std::string(const std::string&, const std::string&, const std::string, bool*)> f_getStr;
  std::function<void       (const std::string&, const std::string&, const std::string)>        f_setStr;
};

//------------------------------------------------------------------------------

#endif  //__AOK_INTERFACE__

