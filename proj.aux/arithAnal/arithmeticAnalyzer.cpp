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
* arithmeticAnalyzer.cpp
* Author:  manxinator
* Created: Fri Nov 13 00:49:40 PST 2015
*******************************************************************************/

#define __CPP_ARITHMETIC_ANALYZER__

#define ARITH_DEBUG_DEF       1
//#define AA_NO_DEFAULT_ERR     1

#ifdef ARITH_DEBUG_DEF
  #include <stdio.h>
  #include <stdlib.h>
  #define AA_DEBUG(...)   printf(__VA_ARGS__)
#else
  #define AA_DEBUG(...)
#endif

#include "arithmeticAnalyzer.h"
#include <sstream>
#include <cstddef>

//------------------------------------------------------------------------------

#ifndef AA_NO_DEFAULT_ERR
  static void arith_static_err (const std::string& eStr)
  {
    fprintf(stderr,"[ARITHMETIC ANALYZER] ERROR: %s\n",eStr.c_str());
  }

  static void arith_static_exit(const std::string& eStr)
  {
    if (eStr.length() > 0)
      fprintf(stderr,"[ARITHMETIC ANALYZER] %s\n",eStr.c_str());
    exit(EXIT_FAILURE);
  }
#endif

        //----------------------------------------------------------------------

typedef struct _opType_t_ {
  std::string opStr;
  int         opId;
  int         preced;
} opType_t;

extern const std::vector<opType_t> opList;

class arithElem_c {
public:
  typedef enum _arithDataType_e_ {
    ADT_INT    = 0,
    ADT_UINT32 = 1,
    ADT_UINT64 = 2,
    ADT_FLOAT  = 3
  } arithDataType_e;

  std::vector<std::shared_ptr<arithElem_c> > entList;   // if size() > 0, then isList == TRUE

  int             opIdx;      // 0 for Variable / Number, >0 for OP
  int             opPreced;
  std::string     strId;
  arithDataType_e dataType;
  int             dat_int;    // dat_uint, dat_float, etc... values evaluated during reduce

  int             isUnary;
  int             betIdx;

public:
           arithElem_c() { opIdx = 0; opPreced = 0; dataType = ADT_INT; isUnary = 0; betIdx = -1; }
  virtual ~arithElem_c() {
    printf("'''~arithElem_c''' destroy %s",strId.c_str());
    if (entList.size() > 0) {
      printf(" -->");
      for (auto it = entList.begin(); it != entList.end(); it++)
        printf(" %s",(*it)->strId.c_str());
    }
    printf("\n");
  }

  static void ConfigDataType( std::vector<std::shared_ptr<arithElem_c> > &elemListRef );
};

class parsTokAux_c {
private:
  int          buffLen;
  int          idx;
  std::string  eqnStr;
public:
           parsTokAux_c(const std::string &targetStr);
  virtual ~parsTokAux_c() {}

  std::shared_ptr<arithElem_c> getTok (void);
};

//------------------------------------------------------------------------------

  /*
    Constructor
  */
arithParser_c::arithParser_c()
{
  AA_DEBUG("  [AA DBG] %s { CONSTRUCTOR }!\n",__PRETTY_FUNCTION__);

#ifndef AA_NO_DEFAULT_ERR
  //
  // Attach default error and logging functions
  //
  f_errFunc  = std::bind(&arith_static_err, std::placeholders::_1);
  f_exitFunc = std::bind(&arith_static_exit,std::placeholders::_1);
#endif
  betIdx   = 0;
  betAbrt  = 0;
  betLstId = 0;
}

//------------------------------------------------------------------------------



  //
  // Parser Implementation
  //
std::shared_ptr<arithParser_c::arithEqn_c> arithParser_c::parseEqn(const std::string &eqStr)
{
  auto eqObj = CreateEqObj();
  AA_DEBUG("  [AA DBG] arithParser_c::parseEqn(%s)!\n",eqStr.c_str());

  //
  // 1- Tokenize string and push into token list
  //  - Determine data type: int, uint32_t, uint64_t, float -- but default to int for now
  //    RULE: assume int, unless some scalar specifies otherwise (such as * 1.0 or 1ull)
  //    * 1.0  - float
  //    * 1u   - uint32_t
  //    * 1ull - uint64_t (or with 0ull)
  // 2- Build tree from list
  //

  parsTokAux_c                               l_tokenizer(eqStr);
  std::vector<std::shared_ptr<arithElem_c> > tokList;

  // 1a - Tokenize
  //
  while (1) {
    std::shared_ptr<arithElem_c> oneTokEl = l_tokenizer.getTok();
    if (oneTokEl->strId == "")
      break;
    tokList.push_back(oneTokEl);
  }
  // 1b - Determine data type
  //
  arithElem_c::ConfigDataType(tokList);

  int iii = 0;
  for (auto it = tokList.begin(); it != tokList.end(); it++)
    (*it)->betIdx = iii++;

  {
    // DEBUG print
    int jjj = 0;
    printf("  + Print tokens!\n");
    for (auto it = tokList.begin(); it != tokList.end(); it++)
      printf("    + tokLst[%2d] opIdx: %2d, preced: %2d, '%s'\n",jjj++,(*it)->opIdx,(*it)->opPreced,(*it)->strId.c_str());
  }

  // 2 - build the tree
  //
  eqObj->topNode = BuildEqnTree(tokList);
  return eqObj;
}



//******************************************************************************



  // ArithParser - Private member function and other functions
  //

std::shared_ptr<arithParser_c::arithEqn_c> arithParser_c::CreateEqObj(void)
{
  auto eqObj = std::make_shared<arithEqn_c>();
  eqObj->setParent(shared_from_this());
  return eqObj;
}

int arithParser_c::getVarInt(const std::string &nameVar)
{
  if (f_getIntVar)
    return f_getIntVar(nameVar);
  else
    exitCall( "[arithParser_c::getVarInt] no function assigned to f_getIntVar!" );
  return 0;
}

void arithParser_c::setVarInt(const std::string &nameVar, int valNew)
{
  if (f_setIntVar)
    return f_setIntVar(nameVar,valNew);
  else
    exitCall( "[arithParser_c::setVarInt] no function assigned to f_setIntVar!" );
}

void arithParser_c::errCall (const std::string &eStr)
{
  if (f_errFunc)
    f_errFunc(eStr);
#ifndef AA_NO_DEFAULT_ERR
  else
    fprintf(stderr,"[ARITHMETIC ANALYZER] %s",eStr.c_str());
#endif
}

void arithParser_c::exitCall(const std::string &eStr)
{
  if (f_exitFunc)
    f_exitFunc(eStr);
#ifndef AA_NO_DEFAULT_ERR
  else {
    fprintf(stderr,"[ARITHMETIC ANALYZER] %s",eStr.c_str());
    exit(EXIT_FAILURE);
  }
#endif
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void arithParser_c::arithEqn_c::setParent(std::shared_ptr<arithParser_c> parenObj)
{
  p_parent = std::weak_ptr<arithParser_c>(parenObj);
}

int arithParser_c::arithEqn_c::get_int(const std::string &varName)
{
  if (auto o_parent = p_parent.lock())
    return o_parent->getVarInt(varName);
  else
    doExit("[arithParser_c::arithEqn_c::get_int()] unable to get lock on parent!");
  return 0;
}

void arithParser_c::arithEqn_c::set_int(const std::string &varName, int newVal)
{
  if (auto o_parent = p_parent.lock())
    return o_parent->setVarInt(varName,newVal);
  else
    doExit("[arithParser_c::arithEqn_c::set_int()] unable to get lock on parent!");
}

void arithParser_c::arithEqn_c::doErr (const std::string &eStr)
{
  if (auto o_parent = p_parent.lock())
    o_parent->errCall(eStr);
#ifndef AA_NO_DEFAULT_ERR
  else
    fprintf(stderr,"[ARITHMETIC ANALYZER] %s",eStr.c_str());
#endif
}

void arithParser_c::arithEqn_c::doExit(const std::string &eStr)
{
  if (auto o_parent = p_parent.lock())
    o_parent->exitCall(eStr);
#ifndef AA_NO_DEFAULT_ERR
  else {
    fprintf(stderr,"[ARITHMETIC ANALYZER] %s",eStr.c_str());
    exit(EXIT_FAILURE);
  }
#endif
}

std::string arithParser_c::arithEqn_c::getStr(void)
{
  std::string l_str = "UNDEFINED";
  if (auto o_parent = p_parent.lock())
    l_str = "supercalifragilistic -- " + o_parent->getIdent();
  else
    ; // ERROR!
  return l_str;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
parsTokAux_c::parsTokAux_c(const std::string &targetStr)
{
  idx     = 0;
  buffLen = targetStr.length();
  eqnStr  = targetStr;
}

std::shared_ptr<arithElem_c> parsTokAux_c::getTok(void)
{
  std::shared_ptr<arithElem_c> tokElem = std::make_shared<arithElem_c>();

  // Skip whitespace
  while (isspace(eqnStr[idx])) idx++;
  if (idx >= buffLen)
    return tokElem;

  // Get the token
  int tokStart = idx;
  char c = eqnStr[idx];
  if ((c == '_') || isalpha(c))
  {
    // Name
//    printf("*** parse word -- w!\n");

    c = eqnStr[++idx];
    for ( ; (c == '_') || isalpha(c) || isdigit(c); idx++)
      c = eqnStr[idx];
    idx -= 1;
    tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
  }
  else if (isdigit(c))
  {
    // Digit -- follow C numeric rules
    if (c == '0')
    {
      c = eqnStr[++idx];
      if ((c == 'x') || (c == 'X'))
      {
        // Hex
//        printf("*** parse digit -- hex\n");
        ++idx;
        while (isxdigit(eqnStr[idx]))
          idx++;
        tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
      }
      else if (isdigit(c))
      {
        // Octal
//        printf("*** parse digit -- octal\n");
        while (isdigit(c))
          c = eqnStr[++idx];
        tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
      }
      else
      {
        // Zero
//        printf("*** parse digit -- zero\n");
        tokElem->strId.assign("0");
      }
    }
    else
    {
      // Decimal
//      printf("*** parse digit -- decimal\n");
      while (isdigit(c))
        c = eqnStr[++idx];
      tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
    }
  }
  else
  {
    // OP
    std::string remStr = eqnStr.substr(idx);
//    printf("*** parse OP '%s'\n",remStr.c_str());
    for (auto it = opList.cbegin(); it != opList.cend(); it++) {
      if (remStr.find(it->opStr) == 0) {
        idx += it->opStr.length();
        tokElem->strId    = it->opStr;
        tokElem->opIdx    = it->opId;
        tokElem->opPreced = it->preced;
        break;
      }
    }
  }
//  printf("*** parse done!\n");
  return tokElem;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Zero means Variable or Number
#define OP_LEFTPAR               1
#define OP_RIGHTPAR              2
#define OP_SCOPE                 3
#define OP_PLUS                  4
#define OP_PLUSPLUS              5
#define OP_MINUS                 6
#define OP_MINUSMINUS            7
#define OP_MULTIPLY              8
#define OP_DIVIDE                9
#define OP_MODULUS              10
#define OP_LSHIFT               11
#define OP_RSHIFT               12
#define OP_BIT_INV              13
#define OP_BIT_AND              14
#define OP_BIT_XOR              15
#define OP_BIT_OR               16
#define OP_LOG_NOT              17
#define OP_LOG_AND              18
#define OP_LOG_OR               19
#define OP_ASGN_EQ              20
#define OP_ASGN_PLUS            21
#define OP_ASGN_MINUS           22
#define OP_ASGN_MULTIPLY        23
#define OP_ASGN_DIVIDE          24
#define OP_ASGN_MODULUS         25
#define OP_ASGN_LSHIFT          26
#define OP_ASGN_RSHIFT          27
#define OP_ASGN_AND             28
#define OP_ASGN_XOR             29
#define OP_ASGN_OR              30
#define OP_CMP_LT               31
#define OP_CMP_GT               32
#define OP_CMP_LTEQ             33
#define OP_CMP_GTEQ             34
#define OP_CMP_EQ               35
#define OP_CMP_NEQ              36
#define OP_COMMA                37

#define PRECED_MAX              15
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::shared_ptr<arithElem_c> arithParser_c::BuildEqnTree( std::vector<std::shared_ptr<arithElem_c> > &elemListRef )
{
#if 0
  return elemListRef[0];
#else
  betIdx   = 0;
  betAbrt  = 0;
  betLstId = 0;
  std::shared_ptr<arithElem_c> headNode = std::make_shared<arithElem_c>();
  BuildSubEqn(headNode,0,elemListRef);
  if (betAbrt) {
    headNode->entList.clear();
    headNode->opIdx = 0;
    headNode->strId = "0";
  }
  return headNode;
#endif
}

void arithParser_c::BuildSubEqn(
    std::shared_ptr<arithElem_c>                &curNode,     int level,
    std::vector<std::shared_ptr<arithElem_c> >  &elemListRef
  )
{
  curNode->strId = genNodeName();

  int startIdx = betIdx;
  int curElOp  = -1;
  int prevElOp = -1;
  while (betIdx < elemListRef.size())
  {
    curElOp = elemListRef[betIdx]->opIdx;

    /* Unary Ops: ++, --, -, ~, ! */

    // What to do with scope operator???
    //

    if (prevElOp < 0)
      // First op, so just insert into list
      curNode->entList.push_back(elemListRef[betIdx]);
    else
    {
      if (curElOp == 0)
      {
        // Variable or Number
        // Verify that the last element OP is not a variable if any
        for (int elIdx = curNode->entList.size()-1; elIdx >= 0; elIdx--) {
          if (curNode->entList[elIdx]->isUnary) continue;
          if (curNode->entList[elIdx]->opPreced == 0) {
            std::stringstream ssErr;
            ssErr << "Variable '" << elemListRef[betIdx]->strId << "' preceded with another variable '" << curNode->entList[elIdx]->strId << "'!!";
            exitCall(ssErr.str());
            betAbrt = 1;
            return;
          }
          break;
        }
        curNode->entList.push_back(elemListRef[betIdx]);
      }
      else
      {
        std::shared_ptr<arithElem_c> subNode;

        // Operator
        switch (curElOp)
        {
        case OP_ASGN_EQ:
        case OP_ASGN_LSHIFT:
        case OP_ASGN_RSHIFT:
        case OP_ASGN_PLUS:
        case OP_ASGN_MINUS:
        case OP_ASGN_MULTIPLY:
        case OP_ASGN_AND:
        case OP_ASGN_XOR:
        case OP_ASGN_OR:
        case OP_ASGN_DIVIDE:
        case OP_ASGN_MODULUS:
          if (prevElOp != 0) {
            std::stringstream ssErr;
            ssErr << "Assignment operator '" << elemListRef[betIdx]->strId << "' not preceded with a variable!";
            exitCall(ssErr.str());
            betAbrt = 1;
            return;
          }
        case OP_LEFTPAR:
          // OP is included in
          curNode->entList.push_back(elemListRef[betIdx++]);

          subNode = std::make_shared<arithElem_c>();
          BuildSubEqn(subNode,0,elemListRef);         //BuildSubEqn(subNode,level+1,elemListRef);
          if (betAbrt)
            return;
          curNode->entList.push_back(subNode);

          curElOp = 0;  // Subtree must to evaluate to a single number
          break;
        //----------------------------------------
        case OP_MINUS:
          // Check if unary: first in the list, or preceeded by another OP
          {
            int l_isUnary = 0;
            int elIdx     = -1;
            if (curNode->entList.size() == 0)
              l_isUnary = 1;
            else {
              for (elIdx = curNode->entList.size()-1; elIdx >= 0; elIdx--) {
                if (curNode->entList[elIdx]->isUnary)      continue;
                if (curNode->entList[elIdx]->opPreced > 0) l_isUnary = 1;
                break;
              }
            }
            if (l_isUnary) {
              elemListRef[betIdx]->isUnary = 1;
              curNode->entList.push_back( elemListRef[betIdx] );

              if (curNode->entList.size() == 0)
                curElOp = -1;
              else {
                if (betIdx < 0) {
                  exitCall("This state suppoed to be unreachable: Unary OP_MINUS 1!!!");
                  betAbrt = 1;
                  return;
                }
                curElOp = elemListRef[elIdx]->opIdx;
              }
              break;
            }
          }
        case OP_LOG_AND:
        case OP_LOG_OR:
        case OP_LSHIFT:
        case OP_RSHIFT:
        case OP_CMP_LTEQ:
        case OP_CMP_GTEQ:
        case OP_CMP_EQ:
        case OP_CMP_NEQ:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_MODULUS:
        case OP_CMP_LT:
        case OP_CMP_GT:
        case OP_BIT_AND:
        case OP_BIT_XOR:
        case OP_BIT_OR:
        case OP_PLUS:
          {
              // Quick check on previous operator type
            if (prevElOp != 0) {
              std::stringstream ssErr;
              ssErr << "Operator " << elemListRef[betIdx]->strId << " after " << elemListRef[betIdx]->strId << " not understood!";
              exitCall(ssErr.str());
              betAbrt = 1;
              return;
            }
              // Get preceding operator index
              // - traverse curNode, since that's all parsed already
            int lastOpIdx = -1;
            {
              for (int elIdx = curNode->entList.size()-1; elIdx >= 0; elIdx--) {
                if (curNode->entList[elIdx]->isUnary)       continue;
                if (curNode->entList[elIdx]->opPreced > 0) {
                  lastOpIdx = curNode->entList[elIdx]->betIdx;
                  curElOp = elemListRef[betIdx]->opIdx;
                  break;
                }
              }
            }
              // Check operator precedence
            if (lastOpIdx >= 0)
            {
              // if cur preced < lastOpIdx ; then current has higher prio
              if (elemListRef[betIdx]->opPreced < elemListRef[lastOpIdx]->opPreced) {
                printf("    &&&&&&&&&&& Here! Current op has higher prio! level=%2d | %s < %s\n",level,elemListRef[lastOpIdx]->strId.c_str(),elemListRef[betIdx]->strId.c_str());

                betIdx--;
                curNode->entList.pop_back();

                subNode = std::make_shared<arithElem_c>();
                BuildSubEqn(subNode,level+1,elemListRef);
                if (betAbrt)
                  return;
                curNode->entList.push_back(subNode);

                curElOp = 0;  // Subtree assumed to evaluate to a single number
                break;
              }
              // if cur preced > lastOpIdx ; then current has lower prio
              if (elemListRef[betIdx]->opPreced > elemListRef[lastOpIdx]->opPreced) {
                printf("    @@@@@@@@@@@ Here! Current op has lower  prio! level=%2d | %s > %s\n",level,elemListRef[lastOpIdx]->strId.c_str(),elemListRef[betIdx]->strId.c_str());
                // if   level==0, replace top level
                // else back to prev level
                if (level == 0) {
                  subNode = curNode;
                  curNode = std::make_shared<arithElem_c>();
                  curNode->strId = genNodeName();
                  curNode->entList.push_back(subNode);
                } else {
                  betIdx--;
                  return;
                }
              }
            }
            curNode->entList.push_back( elemListRef[betIdx] );
          }
          break;
        //----------------------------------------
        // Unary
        case OP_LOG_NOT:
        case OP_BIT_INV:
        case OP_PLUSPLUS:
        case OP_MINUSMINUS:
          elemListRef[betIdx]->isUnary = 1;
          curNode->entList.push_back(elemListRef[betIdx]);
          break;
        //----------------------------------------
        default:
          {
            std::stringstream ssErr;
            ssErr << "Unknown operator " << elemListRef[betIdx]->strId;
            exitCall(ssErr.str());
            betAbrt = 1;
          }
          return;
        }
      }
    }

    prevElOp = curElOp;
    betIdx++;
  }
}

std::string arithParser_c::genNodeName(void)
{
  std::stringstream ssId;
  ssId << "EQN_LIST_" << betLstId++;
  return ssId.str();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void arithElem_c::ConfigDataType( std::vector<std::shared_ptr<arithElem_c> > &elemListRef )
{
  arithDataType_e finalDataType = ADT_INT;
  // TODO: infer finalDataType
  for (auto it = elemListRef.begin(); it != elemListRef.end(); it++)
    (*it)->dataType = finalDataType;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sorted in lexicographic search order
// Precedence: http://en.cppreference.com/w/cpp/language/operator_precedence
const std::vector<opType_t> opList = {
  {"("  , OP_LEFTPAR      , 0},
  {")"  , OP_RIGHTPAR     , 0},
  {"::" , OP_SCOPE        , 1},
  {"++" , OP_PLUSPLUS     , 2},
  {"--" , OP_MINUSMINUS   , 2},
  {"&&" , OP_LOG_AND      , 13},
  {"||" , OP_LOG_OR       , 14},
  {"<<=", OP_ASGN_LSHIFT  , 15},
  {">>=", OP_ASGN_RSHIFT  , 15},
  {"+=" , OP_ASGN_PLUS    , 15},
  {"-=" , OP_ASGN_MINUS   , 15},
  {"*=" , OP_ASGN_MULTIPLY, 15},
  {"&=" , OP_ASGN_AND     , 15},
  {"^=" , OP_ASGN_XOR     , 15},
  {"|=" , OP_ASGN_OR      , 15},
  {"/=" , OP_ASGN_DIVIDE  , 15},
  {"%=" , OP_ASGN_MODULUS , 15},
  {"<<" , OP_LSHIFT       , 7},
  {">>" , OP_RSHIFT       , 7},
  {"<=" , OP_CMP_LTEQ     , 8},
  {">=" , OP_CMP_GTEQ     , 8},
  {"==" , OP_CMP_EQ       , 9},
  {"!=" , OP_CMP_NEQ      , 9},
  {"*"  , OP_MULTIPLY     , 5},
  {"/"  , OP_DIVIDE       , 5},
  {"%"  , OP_MODULUS      , 5},
  {"<"  , OP_CMP_LT       , 8},
  {">"  , OP_CMP_GT       , 8},
  {"~"  , OP_BIT_INV      , 3},
  {"&"  , OP_BIT_AND      , 10},
  {"^"  , OP_BIT_XOR      , 11},
  {"|"  , OP_BIT_OR       , 12},
  {"!"  , OP_LOG_NOT      , 3},
  {"+"  , OP_PLUS         , 6},
  {"-"  , OP_MINUS        , 6},
  {"="  , OP_ASGN_EQ      , 15},
  {","  , OP_COMMA        , 16},
};



