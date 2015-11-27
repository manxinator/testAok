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
* testArith1.cpp
* Author:  manxinator
* Created: Tue Nov 24 00:06:23 PST 2015
*******************************************************************************/

#include "arithmeticAnalyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace std;

map<string,string>        varMap;
shared_ptr<arithParser_c> g_arithParser;

int g_verbose = 1;

//--------------------------------------------------------------------

  int str2int(const string &intStr)
  {
    int retVal;
    try {
      retVal = stoi(intStr,nullptr,0);
    } catch (const out_of_range& oor) {
      // No need to check string length since this exception requires a long string
      // basically, just perform typecasting -- upcast, and then downcast again
      if ((intStr[0] == '0') && ((intStr[1] == 'x') || (intStr[1] == 'X'))) {
        auto ullInt = stoull(intStr,nullptr,0);
        retVal = (int) ullInt;
      } else {
        auto ulInt = stoul(intStr,nullptr,0);
        retVal = (int) ulInt;
      }
    }
    return retVal;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  static void test_static_err (const string& eStr)
  {
    fprintf(stderr,"%s\n",eStr.c_str());
  }

  static void test_static_exit(const string& eStr)
  {
    /* Exit is commented out for clarity, but this function does not exit
       in order to demonstrate Arithmetic Analyzer's abort mechanism.
       Afterwards, the functions below will print more debug information. */
    if (eStr.length() > 0)
      fprintf(stderr,"%s\n",eStr.c_str());
    printf("ERROR!\nERROR!\nERROR!\n");
    printf("ERROR! %s\n",eStr.c_str()); // Want this to be noticable since we're not exiting
    printf("ERROR!\nERROR!\nERROR!\n");
    //exit(EXIT_FAILURE);
  }

  int getIntVar(const string& varName)
  {
    int retVal = 0;
    auto it = varMap.find(varName);
    if (it == varMap.end())
      varMap[varName] = "0";
    else
      retVal = str2int(it->second);
    return retVal;
  }

  void setIntVar(const string& varName, int newVal)
  {
    stringstream ssVal;
    ssVal << newVal;
    varMap[varName] = ssVal.str();
  }

  /*
  void checkIntValue(const string& varName, int expVal)
  {
    auto it = varMap.find(varName);
    if (it == varMap.end()) {
      printf("[checkIntValue] ERROR: variable '%s' not found! Expected value: %d\n",varName.c_str(),expVal);
      exit(-1);
    }
    int readVal = atoi(it->second.c_str());
    if (readVal != expVal) {
      printf("[checkIntValue] ERROR: variable '%s' is %d! Expected value: %d\n",varName.c_str(),readVal,expVal);
      exit(-1);
    }
    printf("[checkIntValue] OK: variable '%s' is %d\n",varName.c_str(),expVal);
  }
  */

  void dumpVars(void)
  {
    printf("  [dumpVars] dumping values!\n");
    for (auto it = varMap.cbegin(); it != varMap.cend(); it++) {
      int varVal = str2int(it->second);
      printf("  * %16s = %d {0x%x}\n",it->first.c_str(),varVal,varVal);
    }
  }

//--------------------------------------------------------------------

void testInit(void)
{
  g_arithParser = make_shared<arithParser_c>();
  if (!g_arithParser) {
    printf("[testInit] ERROR: Instatiating arithParser_c failed!\n");
    exit(EXIT_FAILURE);
  }

  // Attach my functions to the parser
  printf("               - testInit() installing functions to Arithmetic Parser!!\n");
  g_arithParser->f_errFunc   = bind(&test_static_err, placeholders::_1);
  g_arithParser->f_exitFunc  = bind(&test_static_exit,placeholders::_1);
  g_arithParser->f_getIntVar = bind(&getIntVar,       placeholders::_1);
  g_arithParser->f_setIntVar = bind(&setIntVar,       placeholders::_1,placeholders::_2);
  printf("               - testInit() done!!\n");
}

int testEqnInt1(const string &eqnStr, int expVal)
{
  // Just parse the equation and run the computation
  // - Returns 1 on error -- result does not match expected value
  printf("\n  ************\n");
  printf("  [testEqnInt] parsing '%s'\n",eqnStr.c_str());
  auto eqObj  = g_arithParser->parseEqn(eqnStr);
  int  resVal = eqObj->computeInt();
  printf("  [testEqnInt] Computed Value: %d, Expected: %d {0x%x, 0x%x}\n",resVal,expVal,resVal,expVal);
  return resVal != expVal;
}

void testEqnInt2(const map<string,int> &initVals, const string &eqnStr, int expVal, const map<string,int> &checkVals, int verbose=0)
{
  // Clear and initialize the map
  varMap.clear();
  for (auto m_it = initVals.cbegin(); m_it != initVals.cend(); m_it++)
    setIntVar(m_it->first,m_it->second);

  // Call testEqnInt1
  int errFlag = testEqnInt1(eqnStr,expVal);
  if (verbose) {
    printf("  [testEqnInt] print state of variables:\n");
    for (auto m_it = initVals.cbegin(); m_it != initVals.cend(); m_it++)
      printf("  - %16s = %3d\n",m_it->first.c_str(),getIntVar(m_it->first));
  }

  // Verify that values listed in checkVals matches the data stored in the global variable map
  printf("  [testEqnInt] go thru variable check list:\n");
  for (auto m_it = checkVals.cbegin(); m_it != checkVals.cend(); m_it++) {
    string detailStr;
    int resVal = getIntVar(m_it->first);
    if (resVal != m_it->second) {
      errFlag += 1;
      detailStr.assign("<-- ERROR");
    }
    printf("  - %16s = %3d, Expected: %3d %s\n",m_it->first.c_str(),resVal,m_it->second,detailStr.c_str());
  }
  if (errFlag) {
    printf("*\n");
    printf("* ERROR: test failed! Equation: '%s'\n",eqnStr.c_str());
    printf("*\n");
    exit(0);
  }
}

#define TEST_EQN_INT1(EQN_PHRASE)                                   {\
  int resErr = testEqnInt1(                                          \
    #EQN_PHRASE,                                                     \
    EQN_PHRASE                                                       \
  );                                                                 \
  if (resErr) {                                                      \
    printf("*\n");                                                   \
    printf("* ERROR: test failed! Equation: '" #EQN_PHRASE "'\n");   \
    printf("*\n");                                                   \
    dumpVars();                                                      \
    exit(0);                                                         \
  }                                                                  \
}
#define TEST_EQN_INT2(INT_MAP,EQN_PHRASE,CHECK_MAP)                 {\
  testEqnInt2(                                                       \
    INT_MAP,                                                         \
    #EQN_PHRASE,                                                     \
    EQN_PHRASE,                                                      \
    CHECK_MAP                                                        \
  );                                                                 \
}

//--------------------------------------------------------------------

void testCase1(void)
{
  int adjacent = 3, opposite = 4, hypotenuse = 0;
  map<string,int> init_map = { {"adjacent",3}, {"opposite",4}, {"hypotenuse",4} };
  map<string,int> chck_map = { {"adjacent",4}, {"opposite",3}, {"hypotenuse",9} };
  TEST_EQN_INT2(init_map,hypotenuse = adjacent++*--opposite,chck_map);

  TEST_EQN_INT1(034 *  ( 0x9Fa - 3));

  dumpVars();

  TEST_EQN_INT1(!adjacent+(hypotenuse+~1));
  TEST_EQN_INT1(hypotenuse+=-adjacent);
  TEST_EQN_INT1(hypotenuse += adjacent+-opposite);
  TEST_EQN_INT1(1<<2|1<<4);
  TEST_EQN_INT1(0x70ffee23&0x4cc01ade);
  TEST_EQN_INT1(0x7fffffff);
  TEST_EQN_INT1(0x80000000);

  // Corner case -- Working
  int j = 0;
  TEST_EQN_INT1(!~-j);
  TEST_EQN_INT1(!~++j);

  // Corner case -- Working
  int a=0,b=0;
  TEST_EQN_INT1(!!~(a+b));  // ~0 --> F's, !F's --> 0, !0 --> 1

  dumpVars();

  // Corner case -- Working
  {
    int q=4, r=2, s=3;
    map<string,int> init_map2 = { {"q",4}, {"r",2}, {"s",3} };
    map<string,int> chck_map2 = { {"q",4}, {"r",3}, {"s",3} };
    TEST_EQN_INT2(init_map2,q+ -++r*s,chck_map2);
  }

  dumpVars();


  // Corner case ; then, add unary ops
  // A op1 B op2 C op3 D with op1 > op3 >op2
}

//--------------------------------------------------------------------

int main(int ac, char *av[])
{
  printf("---------------- testArith1 start!!\n\n");

  testInit();
  testCase1();

  printf("\n---------------- testArith1 done!!\n");
  return EXIT_SUCCESS;
}

//--------------------------------------------------------------------


