
#include "arithmeticAnalyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <sstream>

using namespace std;

map<string,string> varMap;

//--------------------------------------------------------------------

  static void test_static_err (const std::string& eStr)
  {
    fprintf(stderr,"%s\n",eStr.c_str());
  }

  static void test_static_exit(const std::string& eStr)
  {
    if (eStr.length() > 0)
      fprintf(stderr,"%s\n",eStr.c_str());
    //exit(EXIT_FAILURE);
  }

  int getIntVar(const std::string& varName)
  {
    int retVal = 0;
    auto it = varMap.find(varName);
    if (it == varMap.end())
      varMap[varName] = "0";
    else
      retVal = atoi(it->second.c_str());
    return retVal;
  }

  void setIntVar(const std::string& varName, int newVal)
  {
    stringstream ssVal;
    ssVal << newVal;
    varMap[varName] = ssVal.str();
  }

  void checkIntValue(const std::string& varName, int expVal)
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

//--------------------------------------------------------------------

void test1(void)
{
  shared_ptr<arithParser_c> g_arithParser = make_shared<arithParser_c>();
  if (!g_arithParser) {
    printf("ERROR: arithParser_c::CreateInstance() failed!\n");
    exit(EXIT_FAILURE);
  }
  printf("  [initAA] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Insert my functions, and then perform sanity test
  printf("  ****\n");
  g_arithParser->f_errFunc   = std::bind(&test_static_err, std::placeholders::_1);
  g_arithParser->f_exitFunc  = std::bind(&test_static_exit,std::placeholders::_1);
  g_arithParser->f_getIntVar = std::bind(&getIntVar,       std::placeholders::_1);
  g_arithParser->f_setIntVar = std::bind(&setIntVar,       std::placeholders::_1,std::placeholders::_2);
  printf("  [initAA] ID: '%s'\n",g_arithParser->getIdent().c_str());

  // Parse bare equation and perform another sanity test
  varMap.clear();
  setIntVar("adjacent",3u);
  setIntVar("opposite",4);
  {
    printf("  ----\n");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = midwhere = adjacent   + opposite   ");
    printf("  [initAA] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [initAA] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [initAA] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }
  {
    printf("  ----\n");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = midwhere = adjacent*ABC   + -opposite   ");
    printf("  [initAA] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [initAA] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [initAA] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }
  {
    printf("  ----\n");
    //shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  -adjacent * opposite\t+opposite*hypotenuse   ");
    shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  adjacent *\nopposite\t+opposite*hypotenuse + aa*cc   ");
    printf("  [initAA] adjacent   --> %d\n",eqObj->get_int("adjacent"));
    printf("  [initAA] opposite   --> %d\n",eqObj->get_int("opposite"));
    printf("  [initAA] hypotenuse --> %d\n",eqObj->get_int("hypotenuse"));
  }

  //shared_ptr<arithParser_c::arithEqn_c> eqObj = g_arithParser->parseEqn("  hypotenuse = adjacent ++  + opposite   ");
}

//--------------------------------------------------------------------

//dumpVars
//evalExprAndCheck



//--------------------------------------------------------------------

int main(int ac, char *av[])
{
  printf("---------------- testAA start!!\n\n");
  test1();

  // evalExprAndCheck() - Test expression
  // checkVarValue()    - Verify value of any variables modified by unary ops
  //

  printf("\n---------------- testAA done!!\n");
  return EXIT_SUCCESS;
}

