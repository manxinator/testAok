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
* testExKnLoad1.cpp
* Author:  manxinator
* Created: Wed Dec 23 01:23:10 PST 2015
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "ekRead.h"
#include <stdexcept>

//------------------------------------------------------------------------------

void TestEK_command(std::shared_ptr<ex_knobs::primitive_c> cmdPrim)
{
  bool argRC = false;
  auto argLst = cmdPrim->getArgListRC(argRC);
  if (!argRC)
    throw std::runtime_error("[TestEK_command] ERROR: failed to get argument list!!!\n");

  std::string identStr("ERROR Ident not set!!!");
  if (!cmdPrim->getIdentRC(identStr))
    throw std::runtime_error("[TestEK_command] ERROR: failed to get Ident string!!!\n");

  printf("+++++ [TestEK_command] line: %d { %s",cmdPrim->getLineNum(),identStr.c_str());
  for (auto it = argLst->begin(); it != argLst->end(); it++) {
    ex_knobs::elementType_e elem_type = (*it)->getElemType();
    switch (elem_type)
    {
    case ex_knobs::ELEM_STRING:   printf(", %s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_QSTRING:  printf(", \'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_FUNCTION: {
        ex_knobs::elemFunc_c *eBtFn = dynamic_cast<ex_knobs::elemFunc_c*>(*it);
        printf(", elemFunc_c{%s,%s}",eBtFn->identStr.c_str(),eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EQUATION: {
        ex_knobs::elemEqn_c *eBtFn = dynamic_cast<ex_knobs::elemEqn_c*>(*it);
        printf(", elemEqn_c{%s}",eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EXPANSION: {
        ex_knobs::elemExp_c *eBtEx = dynamic_cast<ex_knobs::elemExp_c*>(*it);
        printf(", elemExp_c{%s,%s}",eBtEx->identStr.c_str(),eBtEx->parenStr.c_str());
        break;
      }
    default:
      printf(", ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");
}

void TestEK_object(std::shared_ptr<ex_knobs::primitive_c> objPrim)
{
  printf("+++++ [TestEK_object] line: %d { ",objPrim->getLineNum());

  bool argRC = false;
  auto argLst = objPrim->getArgListRC(argRC);
  if (!argRC)
    throw std::runtime_error("[TestEK_object] ERROR: failed to get argument list!!!\n");

  for (auto it = argLst->begin(); it != argLst->end(); it++) {
    if (it != argLst->begin())
      printf(", ");
    ex_knobs::elementType_e elem_type = (*it)->getElemType();
    switch (elem_type)
    {
    case ex_knobs::ELEM_STRING:   printf("%s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_QSTRING:  printf("\'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_FUNCTION: {
        ex_knobs::elemFunc_c *eBtFn = dynamic_cast<ex_knobs::elemFunc_c*>(*it);
        printf("elemFunc_c{%s,%s}",eBtFn->identStr.c_str(),eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EQUATION: {
        ex_knobs::elemEqn_c *eBtFn = dynamic_cast<ex_knobs::elemEqn_c*>(*it);
        printf("elemEqn_c{%s}",eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EXPANSION: {
        ex_knobs::elemExp_c *eBtEx = dynamic_cast<ex_knobs::elemExp_c*>(*it);
        printf("elemExp_c{%s,%s}",eBtEx->identStr.c_str(),eBtEx->parenStr.c_str());
        break;
      }
    default:
      printf("ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");
}

void TestEK_knob(std::shared_ptr<ex_knobs::primitive_c> knobPrim)
{
  printf("+++++ [TestEK_knob] LHS line: %d { ",knobPrim->getLineNum());

  bool lhsRC = false, rhsRC = false;
  auto lhsLst = knobPrim->getLhsListRC(lhsRC);
  auto rhsLst = knobPrim->getRhsListRC(rhsRC);
  if (!lhsRC || !rhsRC)
    throw std::runtime_error("[TestEK_knob] ERROR: failed to get argument list!!!\n");

  for (auto it = lhsLst->begin(); it != lhsLst->end(); it++) {
    if (it != lhsLst->begin())
      printf(", ");
    ex_knobs::elementType_e elem_type = (*it)->getElemType();
    switch (elem_type)
    {
    case ex_knobs::ELEM_STRING:   printf("%s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_QSTRING:  printf("\'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_FUNCTION: {
        ex_knobs::elemFunc_c *eBtFn = dynamic_cast<ex_knobs::elemFunc_c*>(*it);
        printf("elemFunc_c{%s,%s}",eBtFn->identStr.c_str(),eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EQUATION: {
        ex_knobs::elemEqn_c *eBtFn = dynamic_cast<ex_knobs::elemEqn_c*>(*it);
        printf("elemEqn_c{%s}",eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EXPANSION: {
        ex_knobs::elemExp_c *eBtEx = dynamic_cast<ex_knobs::elemExp_c*>(*it);
        printf("elemExp_c{%s,%s}",eBtEx->identStr.c_str(),eBtEx->parenStr.c_str());
        break;
      }
    default:
      printf("ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");

  printf("+++++ [TestEK_knob] RHS line: %d { ",knobPrim->getLineNum());
  for (auto it = rhsLst->begin(); it != rhsLst->end(); it++) {
    if (it != rhsLst->begin())
      printf(", ");
    ex_knobs::elementType_e elem_type = (*it)->getElemType();
    switch (elem_type)
    {
    case ex_knobs::ELEM_STRING:   printf("%s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_QSTRING:  printf("\'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
    case ex_knobs::ELEM_FUNCTION: {
        ex_knobs::elemFunc_c *eBtFn = dynamic_cast<ex_knobs::elemFunc_c*>(*it);
        printf("elemFunc_c{%s,%s}",eBtFn->identStr.c_str(),eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EQUATION: {
        ex_knobs::elemEqn_c *eBtFn = dynamic_cast<ex_knobs::elemEqn_c*>(*it);
        printf("elemEqn_c{%s}",eBtFn->parenStr.c_str());
        break;
      }
    case ex_knobs::ELEM_EXPANSION: {
        ex_knobs::elemExp_c *eBtEx = dynamic_cast<ex_knobs::elemExp_c*>(*it);
        printf("elemExp_c{%s,%s}",eBtEx->identStr.c_str(),eBtEx->parenStr.c_str());
        break;
      }
    default:
      printf("ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");
}

void TestEK_RemSL(std::shared_ptr<ex_knobs::primitive_c> commPrim)
{
  std::string commentStr("ERROR Comment not set!!!");
  commPrim->getCommentRC(commentStr);
  printf("+++++ [TestEK_RemSL] line: %d ~%s~\n",commPrim->getLineNum(),commentStr.c_str());
}

void TestEK_RemML(std::shared_ptr<ex_knobs::primitive_c> commPrim)
{
  std::string commentStr("ERROR Comment not set!!!");
  commPrim->getCommentRC(commentStr);
  printf("+++++ [TestEK_RemML] line: %d ~%s~\n",commPrim->getLineNum(),commentStr.c_str());
}

void TestEK_xml(std::shared_ptr<ex_knobs::primitive_c> xmlPrim)
{
  std::string identStr("ERROR Ident not set!!!");
  xmlPrim->getIdentRC(identStr);

  bool optRC = false, txtRC = false;
  auto optLstPtr = xmlPrim->getOptionsRC(optRC);
  auto txtLstPtr = xmlPrim->getTextRC   (txtRC);

  std::string rcPrintStr;
  if (!optRC || !txtRC)
    rcPrintStr = "ERROR: option / text return code error!!!";

  printf("+++++ [TestEK_xml] line: %d Ident: '%s' %s\n",xmlPrim->getLineNum(),identStr.c_str(),rcPrintStr.c_str());
  if (!optRC || !txtRC)
    throw std::runtime_error(rcPrintStr);

  if (optLstPtr->size() > 0) {
    printf("+++++ [TestEK_xml] options: { ");
    for (auto it = optLstPtr->begin(); it != optLstPtr->end(); it++) {
      if (it != optLstPtr->begin())
        printf(", ");
      ex_knobs::elementType_e elem_type = (*it)->getElemType();
      switch (elem_type)
      {
      case ex_knobs::ELEM_STRING:   printf("%s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
      case ex_knobs::ELEM_QSTRING:  printf("\'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
      default:
        printf("ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
      }
    }
    printf(" }\n");
  }
  printf("+++++ [TestEK_xml] body:\n");
  for (auto it = txtLstPtr->begin(); it != txtLstPtr->end(); it++) {
    ex_knobs::elementType_e elem_type = (*it)->getElemType();
    if (elem_type == ex_knobs::ELEM_STRING)
      printf("@@%s@@",static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str());
    else
      printf("ELEM_TYPE:%d",static_cast<int>(elem_type));
    printf("\n");
  }
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("ERROR: %s requires argument <INPUT_FILENAME>\n",argv[0]);
    return EXIT_FAILURE;
  }

  ex_knobs::ek_command_f    = std::bind(&TestEK_command,std::placeholders::_1);
  ex_knobs::ek_object_f     = std::bind(&TestEK_object, std::placeholders::_1);
  ex_knobs::ek_knob_f       = std::bind(&TestEK_knob,   std::placeholders::_1);
  ex_knobs::ek_comment_sl_f = std::bind(&TestEK_RemSL,  std::placeholders::_1);
  ex_knobs::ek_comment_ml_f = std::bind(&TestEK_RemML,  std::placeholders::_1);
  ex_knobs::ek_xml_f        = std::bind(&TestEK_xml,    std::placeholders::_1);

  char* inpFN  = argv[1];
  int   retVal = ex_knobs::ek_readfile(inpFN,0);
  printf("~\n~ [main] argc: %d, argv[0]: %s\n",argc,inpFN);
  printf("~\n~ [main] aok_readfile() returned %d!\n~\n",retVal);
  return 0;
}

//------------------------------------------------------------------------------

