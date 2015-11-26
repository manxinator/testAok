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

  static void test_static_err (const string& eStr)
  {
    fprintf(stderr,"%s\n",eStr.c_str());
  }

  static void test_static_exit(const string& eStr)
  {
    if (eStr.length() > 0)
      fprintf(stderr,"%s\n",eStr.c_str());
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

  // Insert my functions
  printf("               - testInit() installing functions to Arithmetic Parser!!\n");
  g_arithParser->f_errFunc   = bind(&test_static_err, placeholders::_1);
  g_arithParser->f_exitFunc  = bind(&test_static_exit,placeholders::_1);
  g_arithParser->f_getIntVar = bind(&getIntVar,       placeholders::_1);
  g_arithParser->f_setIntVar = bind(&setIntVar,       placeholders::_1,placeholders::_2);
  printf("               - testInit() done!!\n");
}

int testEqnInt1(const string &eqnStr, int expVal)
{
  printf("\n  ************\n");
  printf("  [testEqnInt] parsing '%s'\n",eqnStr.c_str());
  auto eqObj  = g_arithParser->parseEqn(eqnStr);
  int  resVal = eqObj->computeInt();
  printf("  [testEqnInt] Computed Value: %d, Expected: %d {0x%x, 0x%x}\n",resVal,expVal,resVal,expVal);
  return resVal != expVal;
}

void testEqnInt2(const map<string,int> &initVals, const string &eqnStr, int expVal, const map<string,int> &checkVals, int verbose=0)
{
  varMap.clear();
  for (auto m_it = initVals.cbegin(); m_it != initVals.cend(); m_it++)
    setIntVar(m_it->first,m_it->second);

  int errFlag = testEqnInt1(eqnStr,expVal);
  if (verbose) {
    printf("  [testEqnInt] print state of variables:\n");
    for (auto m_it = initVals.cbegin(); m_it != initVals.cend(); m_it++)
      printf("  - %16s = %3d\n",m_it->first.c_str(),getIntVar(m_it->first));
  }
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
  int val;
  val = str2int("0x81234567");  printf("val: 0x%08x\n",val);
  val = str2int("0x871234567"); printf("val: 0x%08x\n",val);

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

  dumpVars();
}

//--------------------------------------------------------------------

int main(int ac, char *av[])
{
  printf("---------------- testArith1 start!!\n\n");

  testInit();
  testCase1();

  //test1();
  //test2();

  //
  // evalExprAndCheck() - Test expression
  // checkVarValue()    - Verify value of any variables modified by unary ops
  //

  printf("\n---------------- testArith1 done!!\n");
  return EXIT_SUCCESS;
}





















//--------------------------------------------------------------------

void test1(void)
{
  shared_ptr<arithParser_c> g_arithParser = make_shared<arithParser_c>();
  if (!g_arithParser) {
    printf("ERROR: arithParser_c::CreateInstance() failed!\n");
    exit(EXIT_FAILURE);
  }
  printf("  [test1] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Insert my functions, and then perform sanity test
  printf("  ****\n");
  g_arithParser->f_errFunc   = std::bind(&test_static_err, std::placeholders::_1);
  g_arithParser->f_exitFunc  = std::bind(&test_static_exit,std::placeholders::_1);
  g_arithParser->f_getIntVar = std::bind(&getIntVar,       std::placeholders::_1);
  g_arithParser->f_setIntVar = std::bind(&setIntVar,       std::placeholders::_1,std::placeholders::_2);
  printf("  [test1] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Parse bare equation and perform another sanity test
  varMap.clear();
  setIntVar("adjacent",3u);
  setIntVar("opposite",4);
  {
    printf("  ----\n");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = midwhere = (adjacent)   + opposite   ");
    printf("  [test1] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [test1] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [test1] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }
  {
    printf("  ----\n");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = midwhere = adjacent*ABC   + -opposite   ");
    printf("  [test1] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [test1] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [test1] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }
  {
    printf("  ----\n");
    //shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  -adjacent * opposite\t+opposite*hypotenuse   ");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  adjacent *\nopposite\t+opposite*hypotenuse + aa*cc   ");
    printf("  [test1] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [test1] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [test1] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }

  //shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = adjacent ++  + opposite   ");
}


void test2(void)
{
  shared_ptr<arithParser_c> g_arithParser = make_shared<arithParser_c>();
  if (!g_arithParser) {
    printf("ERROR: arithParser_c::CreateInstance() failed!\n");
    exit(EXIT_FAILURE);
  }
  printf("  [test2] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Insert my functions, and then perform sanity test
  printf("  ****\n");
  g_arithParser->f_errFunc   = std::bind(&test_static_err, std::placeholders::_1);
  g_arithParser->f_exitFunc  = std::bind(&test_static_exit,std::placeholders::_1);
  g_arithParser->f_getIntVar = std::bind(&getIntVar,       std::placeholders::_1);
  g_arithParser->f_setIntVar = std::bind(&setIntVar,       std::placeholders::_1,std::placeholders::_2);
  printf("  [test2] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Parse bare equation and perform another sanity test
  varMap.clear();
  setIntVar("adjacent",3);
  setIntVar("opposite",4);
  {
    printf("  ----\n");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   55 = 010");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   hypotenuse = 010");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   opposite += ++adjacent");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   opposite += adjacent");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   hypotenuse = adjacent*opposite  ");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj  = g_arithParser->parseEqn("   hypotenuse = adjacent++*--opposite  ");
    printf("  [test2] eqObj->computeInt()  --> %d\n",eqObj->computeInt());
    printf("  [test2] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [test2] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [test2] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));

    int adj = 3;
    int opp = 4;
    printf("  [test2] expected ====> %d",adj++*--opp);
    printf(", adj: %d, opp: %d\n",adj,opp);
  }


  varMap.clear();
  setIntVar("adjacent",3);
  setIntVar("opposite",4);
  setIntVar("zeusnumb",9);
  {
    printf("  ----\n");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   zeusnumb   += ++adjacent*opposite--  ");
//    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   hypotenuse=(zeusnumb+=++adjacent*opposite--)  ");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("   hypotenuse=(zeusnumb+=++adjacent)*opposite--  ");
    printf("  [test2] eqObj->computeInt()  --> %d\n",eqObj->computeInt());
    printf("  [test2] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [test2] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [test2] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
    printf("  [test2] zeusnumb   --> %d\n",eqObj->get_int("zeusnumb"));

    int adj = 3;
    int opp = 4;
    int znu = 9;
    znu += ++adj*opp--;
    printf("  [test2] expected ====> %d",znu);
    printf(", adj: %d, opp: %d\n",adj,opp);
  }
}

