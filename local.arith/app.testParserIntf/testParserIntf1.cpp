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

#include <stdio.h>
#include <stdlib.h>
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

int main(int ac, const char *av[])
{
  if (ac < 2) {
    printf("ERROR: %s requires argument <INPUT_FILENAME>\n",av[0]);
    return EXIT_FAILURE;
  }

  printf("---------------- testParserIntf1 start!!\n\n");

  parseIntfTester piTest;
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



