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
* testParserIntf1.cpp
* Author:  manxinator
* Created: Sat Jan 16 02:18:40 PST 2016
*******************************************************************************/

#include "aokParserIntf.h"
#include "aokTools.h"
#include "aokIntf.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
using namespace std;

//--------------------------------------------------------------------

class parseIntfTester {
public:
           parseIntfTester() { configure(); }
  virtual ~parseIntfTester() { }

private:
  shared_ptr<aokParserIntf_c> parserIntf;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  void my_err_func (const string& eStr)
  {
    fprintf(stderr,"%s\n",eStr.c_str());
  }

  void my_exit_func (const string& eStr)
  {
    if (eStr.length() > 0)
      fprintf(stderr,"%s\n",eStr.c_str());
    printf("ERROR!\nERROR!\nERROR!\n");
    printf("ERROR! %s\n",eStr.c_str());
    printf("ERROR!\nERROR!\nERROR!\n");
    exit(EXIT_FAILURE);
  }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
  aokIntf_c aokInterface;

  map<string,uint32_t> mapNum;
  map<string,string>   mapStr;

  uint32_t getU32(const string& obj, const string& knob, uint32_t def=0xFFFFFFFF, bool*rc   = NULL);
  void     setU32(const string& obj, const string& knob, uint32_t val,            int  mode = 0);
  int      getI32(const string& obj, const string& knob, int      def=-1,         bool*rc   = NULL);
  void     setI32(const string& obj, const string& knob, int      val,            int  mode = 0);
  string   getStr(const string& obj, const string& knob, const string &def = "__NOT_DEFINED__", bool *rc = NULL);
  void     setStr(const string& obj, const string& knob, const string &val);

public:
  void configure(void)
  {
    /*
      Parser Intf:
      - Instantiate aokParserIntf_c
      - AOK functions: get, set, err, exit, etc
      * debug command insertion
    */

    parserIntf = make_shared<aokParserIntf_c>();
    parserIntf->setErrFunc ( std::bind(&parseIntfTester::my_err_func,  this, std::placeholders::_1) );
    parserIntf->setExitFunc( std::bind(&parseIntfTester::my_exit_func, this, std::placeholders::_1) );

    /*
      TODO: debug command insertion
    */
  }

  void loadFile(const string& fnStr)
  {
    int loadRslt = parserIntf->loadFile(fnStr);
    printf("\n  ***\n");
    if (loadRslt)
      printf("  *** File %s loaded!\n",fnStr.c_str());
    else
      printf("  *** ERROR: Failed to load '%s'\n",fnStr.c_str());
    printf("  ***\n");
  }

  void cleanup(void)
  {
    parserIntf.reset();
  }
};

//--------------------------------------------------------------------

#define ACC_G(OP,TYPE1,TYPE2,MAP)                                                       \
TYPE1 parseIntfTester::OP(const string& obj, const string& knob, TYPE2 def, bool *rc) { \
  string idxStr = obj + "::" + knob;                                                    \
  if (MAP.end() == MAP.find(idxStr)) {                                                  \
    if (rc) *rc = false;                                                                \
    return def;                                                                         \
  }                                                                                     \
  if (rc) *rc = true;                                                                   \
  return (TYPE1)MAP[idxStr];                                                            \
}

#define ACC_S(OP,TYPE1,TYPE2,MAP)                                                       \
TYPE1 parseIntfTester::OP(const string& obj, const string& knob, TYPE2 val, int mode) { \
  string idxStr = obj + "::" + knob;                                                    \
  MAP[idxStr] = val;                                                                    \
}
#define ACC_Z(OP,TYPE1,TYPE2,MAP)                                                       \
TYPE1 parseIntfTester::OP(const string& obj, const string& knob, TYPE2 val)           { \
  string idxStr = obj + "::" + knob;                                                    \
  MAP[idxStr] = val;                                                                    \
}

ACC_G(getU32,uint32_t,uint32_t,      mapNum)
ACC_S(setU32,void,    uint32_t,      mapNum)
ACC_G(getI32,int,     int,           mapNum)
ACC_S(setI32,void,    int,           mapNum)
ACC_G(getStr,string,  const string&, mapStr)
ACC_Z(setStr,void,    const string&, mapStr)


class aokIntfConfig_c {
public:
  aokIntfConfig_c(parseIntfTester* p_tgt)
  {
    p_tgt->aokInterface.f_getU32 = std::bind(&parseIntfTester::getU32, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    p_tgt->aokInterface.f_setU32 = std::bind(&parseIntfTester::setU32, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    p_tgt->aokInterface.f_getI32 = std::bind(&parseIntfTester::getI32, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    p_tgt->aokInterface.f_setI32 = std::bind(&parseIntfTester::setI32, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    p_tgt->aokInterface.f_getStr = std::bind(&parseIntfTester::getStr, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    p_tgt->aokInterface.f_setStr = std::bind(&parseIntfTester::setStr, p_tgt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  }
};

//--------------------------------------------------------------------

int main(int ac, const char *av[])
{
  if (ac < 2) {
    printf("ERROR: %s requires argument <INPUT_FILENAME>\n",av[0]);
    return EXIT_FAILURE;
  }

  printf("---------------- testParserIntf1 start!!\n\n");

  parseIntfTester piTest;
  aokIntfConfig_c aokConfig(&piTest);
  piTest.loadFile(av[1]);
  piTest.cleanup();

  printf("\n---------------- testParserIntf1 done!!\n");
  return EXIT_SUCCESS;
}

//--------------------------------------------------------------------

void test1(void)
{
  vector<string> vs;
  vs.clear(); aok_tools::str2strVec("[]",vs);
  vs.clear(); aok_tools::str2strVec("[abc]",vs);
  vs.clear(); aok_tools::str2strVec(" (kwanzaa) ",vs);
  vs.clear(); aok_tools::str2strVec(" [",vs);
  vs.clear(); aok_tools::str2strVec("  )",vs);
  vs.clear(); aok_tools::str2strVec("[ ",vs);
  vs.clear(); aok_tools::str2strVec(") ",vs);
  vs.clear(); aok_tools::str2strVec(" { ",vs);
  vs.clear(); aok_tools::str2strVec(" } ",vs);
  vs.clear(); aok_tools::str2strVec(" ( kwanzaa ) ",vs);
  vs.clear(); aok_tools::str2strVec(" ( kwanzaa ; spring ) ",vs);
}



