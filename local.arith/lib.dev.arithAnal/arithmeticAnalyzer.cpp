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

//#define ARITH_DEBUG_DEF       1
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
#include <stdexcept>

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

static std::string aa_util_entListDbg(const std::vector<std::shared_ptr<arithElem_c> > &elemListRef, int pos);

        //----------------------------------------------------------------------

class arithElem_c {
private:
  int auxTraverseInt(std::shared_ptr<arithParser_c> pp_parent, int &travAbrt);
  int performAsInt  (std::shared_ptr<arithParser_c> pp_parent, const std::string &varId, int opId, int rhsVal);
  int performOpInt  (int opId, int A, int B, int &errFlag);
  int isAssignOp    (int opId);
  int isLhsUnaryOp  (int opId);

  int getIntFromStr (const std::string &strId, int &intVal);

  friend class arithParser_c::arithEqn_c;

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
    AA_DEBUG("    ''' ~arithElem_c ''' destroy %s",strId.c_str());
    if (entList.size() > 0) {
      AA_DEBUG(" -->");
      for (auto it = entList.begin(); it != entList.end(); it++)
        AA_DEBUG(" %s",(*it)->strId.c_str());
    }
    AA_DEBUG("\n");
  }

  void checkOp(void) { isUnary = isLhsUnaryOp(opIdx); }

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

 #ifdef ARITH_DEBUG_DEF
  {
    // DEBUG print
    int jjj = 0;
    AA_DEBUG("  + Print tokens!\n");
    for (auto it = tokList.begin(); it != tokList.end(); it++)
      AA_DEBUG("    + tokLst[%2d] opIdx: %2d, preced: %2d, '%s'\n",jjj++,(*it)->opIdx,(*it)->opPreced,(*it)->strId.c_str());
  }
 #endif

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

std::shared_ptr<arithElem_c> arithParser_c::arithEqn_c::compute(void)
{
  auto o_parent = p_parent.lock();

  auto retElem = std::make_shared<arithElem_c>();
  retElem->dataType = topNode->dataType;

  int travErr = 0;
  switch (retElem->dataType)
  {
  case arithElem_c::ADT_INT:
    retElem->dat_int = topNode->auxTraverseInt(o_parent,travErr);
    if (travErr)
      doErr( "[arithElem_c::compute] topNode->auxTraverseInt() had an error!" );    // not doExit, since that should've been called in auxTraverseInt()
    break;
  case arithElem_c::ADT_UINT32:
  case arithElem_c::ADT_UINT64:
  case arithElem_c::ADT_FLOAT:
    doExit( "[arithElem_c::compute] ADT_UINT32 not implemented!" );
    break;
  }
  // When returning, the only valid members of arithElem_c are:
  // - dataType
  //   - ADT_INT --> dat_int
  //
  return retElem;
}

int arithParser_c::arithEqn_c::computeInt(void)
{
  switch (topNode->dataType)
  {
  case arithElem_c::ADT_INT:  return compute()->dat_int;
  default:
    break;
  }
  doExit("[arithElem_c::computeInt] This state suppoed to be unreachable!!!");
  return -1;
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
    // Variable
    //
    c = eqnStr[++idx];
    for ( ; (c == '_') || isalpha(c) || isdigit(c); idx++)
      c = eqnStr[idx];
    idx -= 1;
    tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
  }
  else if (isdigit(c))
  {
    // Digit -- follow C numeric rules
    //
    if (c == '0')
    {
      c = eqnStr[++idx];
      if ((c == 'x') || (c == 'X'))     // Hex
      {
        ++idx;
        while (isxdigit(eqnStr[idx]))
          idx++;
        tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
      }
      else if (isdigit(c))              // Octal
      {
        while (isdigit(c))
          c = eqnStr[++idx];
        tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
      }
      else                              // Zero
      {
        tokElem->strId.assign("0");
      }
    }
    else                                // Decimal
    {
      while (isdigit(c))
        c = eqnStr[++idx];
      tokElem->strId = eqnStr.substr(tokStart,idx-tokStart);
    }
    // TODO - handle suffix: u, ul
  }
  else
  {
    // OP
    //
    std::string remStr = eqnStr.substr(idx);
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
#define SS_ERR_BET_ABORT(__SS_CMD__) {  \
    std::stringstream ssErr;            \
    __SS_CMD__;                         \
    ssErr << "\n    File: " << __FILE__ \
          << "\n    Line: " << __LINE__;\
    exitCall(ssErr.str());              \
    betAbrt = 1;                        \
}

std::shared_ptr<arithElem_c> arithParser_c::BuildEqnTree( std::vector<std::shared_ptr<arithElem_c> > &elemListRef )
{
  betIdx   = 0;
  betAbrt  = 0;
  betLstId = 0;
  std::shared_ptr<arithElem_c> headNode = std::make_shared<arithElem_c>();
  BuildSubEqn(headNode,0,elemListRef);
#if 0
  if (betAbrt) { // For easy debugging
    AA_DEBUG("  [betAbrt] headNode: %s\n",headNode->strId.c_str());
    auto dummyNode = std::make_shared<arithElem_c>();
    dummyNode->entList.clear();
    dummyNode->opIdx = 0;
    dummyNode->strId = "0";
    return dummyNode;
  }
#endif
  if (betAbrt) {
    headNode->entList.clear();
    headNode->opIdx = 0;
    headNode->strId = "0";
  }
  return headNode;
}

void arithParser_c::BuildSubEqn(
    std::shared_ptr<arithElem_c>                &curNode,     int level,
    std::vector<std::shared_ptr<arithElem_c> >  &elemListRef
  )
{
  curNode->strId    = genNodeName();
  curNode->dataType = elemListRef[betIdx]->dataType;

  int curElOp  = -1;
  int prevElOp = -1;
  while (betIdx < (int)elemListRef.size())
  {
    curElOp = elemListRef[betIdx]->opIdx;

    /*
      Unary Ops: ++, --, -, ~, !
    */

    // What to do with scope operator???
    //

    if ((prevElOp < 0) && (curElOp != OP_LEFTPAR))
    {
      // First op, so just insert into list
      curNode->entList.push_back(elemListRef[betIdx]);
      curNode->entList[curNode->entList.size()-1]->checkOp();
    }
    else
    {
      if (curElOp == 0)
      {
        // Variable or Number
        // Verify that the preceding element OP is not a variable if any
        for (int elIdx = curNode->entList.size()-1; elIdx >= 0; elIdx--) {
          if (curNode->entList[elIdx]->isUnary) continue;
          if (curNode->entList[elIdx]->opPreced == 0) {
            SS_ERR_BET_ABORT(ssErr << "Variable '" << elemListRef[betIdx]->strId << "' preceded with another variable '" << curNode->entList[elIdx]->strId << "'!!");
            return;
          }
          break;
        }
        curNode->entList.push_back(elemListRef[betIdx]);
      }
      else
      {
        int l_leftParDet = 0;
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
            SS_ERR_BET_ABORT(ssErr << "Assignment operator '" << elemListRef[betIdx]->strId << "' not preceded with a variable!");
            return;
          }
        case OP_LEFTPAR:
          if (curElOp == OP_LEFTPAR) {
            l_leftParDet = 1;
            betIdx++;
          } else
            // OP is included in
            curNode->entList.push_back(elemListRef[betIdx++]);

          subNode = std::make_shared<arithElem_c>();
          BuildSubEqn(subNode,0,elemListRef);
          if (betAbrt)
            return;
          curNode->entList.push_back(subNode);
          curElOp = 0;  // Subtree must to evaluate to a single number

          if (l_leftParDet) {
            // Expect matching right parenthesis
            if ((betIdx >= (int)elemListRef.size()) || (elemListRef[betIdx]->opIdx != OP_RIGHTPAR)) {
              SS_ERR_BET_ABORT(ssErr << "Parsing '(' statement, but matching right parenthesis ')' not found!!");
              return;
            }
          } else {
            // Found right parenthesis, but it is not ours -- exit set
            if ((betIdx < (int)elemListRef.size()) && (elemListRef[betIdx]->opIdx == OP_RIGHTPAR))
              return;
          }
          break;
        case OP_RIGHTPAR:
          return;
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
            int lastOpIdx = -1;
            {
              // Traverse curNode, since that's all parsed already
              // We're in an OP, so look for the last variable
              int lstVarIdx = curNode->entList.size()-1;
              for ( ; lstVarIdx >= 0; lstVarIdx--) {
                if (curNode->entList[lstVarIdx]->isUnary)    continue;
                if (curNode->entList[lstVarIdx]->opIdx == 0) break;
                lstVarIdx = 0;
              }
              if (lstVarIdx < 0) {
                SS_ERR_BET_ABORT(ssErr << "Op sequence " << elemListRef[betIdx]->strId << " not understood!");
                return;
              }

              // Get preceding operator index
              for (int elIdx = curNode->entList.size()-1; elIdx >= 0; elIdx--) {
                if (curNode->entList[elIdx]->isUnary) continue;
                if (curNode->entList[elIdx]->opIdx > 0) {
                  lastOpIdx = curNode->entList[elIdx]->betIdx;
                  curElOp   = elemListRef[betIdx]->opIdx;
                  break;
                }
              }
            }
              // Check operator precedence
            if (lastOpIdx >= 0)
            {
              // if cur preced < lastOpIdx ; then current has higher prio
              if (elemListRef[betIdx]->opPreced < elemListRef[lastOpIdx]->opPreced) {
                AA_DEBUG("    &&&&&&&&&&& Here! Current op has higher prio! level=%2d | %s < %s\n",level,elemListRef[lastOpIdx]->strId.c_str(),elemListRef[betIdx]->strId.c_str());

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
                AA_DEBUG("    @@@@@@@@@@@ Here! Current op has lower  prio! level=%2d | %s > %s\n",level,elemListRef[lastOpIdx]->strId.c_str(),elemListRef[betIdx]->strId.c_str());
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
          SS_ERR_BET_ABORT(ssErr << "Unknown operator " << elemListRef[betIdx]->strId);
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
  // TODO: infer finalDataType -- for now, assume INT
  for (auto it = elemListRef.begin(); it != elemListRef.end(); it++)
    (*it)->dataType = finalDataType;
}

#define SS_ERR_ATI_ABORT(__SS_CMD__) {  \
    std::stringstream ssErr;            \
    __SS_CMD__;                         \
    ssErr << "\n    File: " << __FILE__ \
          << "\n    Line: " << __LINE__;\
    pp_parent->exitCall(ssErr.str());   \
    travAbrt = 1;                       \
}

int arithElem_c::auxTraverseInt(std::shared_ptr<arithParser_c> pp_parent, int &travAbrt)
{
  /*
    1- Get first non-op element -- check for left unary OP, check for right unary OP
    2- Loop:
       - Next should be OP
       - Get next non-op -- check for left unary OP, check for right unary OP
       - Perform OP
  */

  int dat_accum =  0;       // Accumulator
  int dat_a_idx = -1;       // Accumulator Index
  int dat_a_var =  0;       // Accumulator is Variable

  int elIdx     = 0;
  int lastOpId  = 0;
  while (elIdx < (int)entList.size())
  {
    int dat_nuevo =  0;
    int dat_n_idx = -1;
    int dat_n_var =  0;

    int lfUnaryIdx = -1;
    int rtUnaryIdx = -1;
    if (entList[elIdx]->isUnary)                                            // left unary op
      lfUnaryIdx = elIdx++;
    if ((elIdx + 1 < (int)entList.size()) && (entList[elIdx+1]->isUnary))   // right unary op
      rtUnaryIdx = elIdx+1;

    if (entList[elIdx]->entList.size() > 0) {
      dat_n_idx = elIdx;
      dat_nuevo = entList[elIdx++]->auxTraverseInt(pp_parent,travAbrt);     // traverse sub-equation
      if (travAbrt)
        return -1;
      if (rtUnaryIdx > 0) {
        SS_ERR_ATI_ABORT(ssErr << "Right-side unary OP '" << entList[elIdx]->strId << "' not expected!" << aa_util_entListDbg(entList,rtUnaryIdx));
        return -1;
      }
    }
    else if ((entList[elIdx]->opPreced > 0) && (lfUnaryIdx < 0)) {          // OP not expected at this time
      SS_ERR_ATI_ABORT(ssErr << "OP '" << entList[elIdx]->strId << "' not expected!" << aa_util_entListDbg(entList,elIdx));
      return -1;
    }
    else                                                                    // variable
    {
      if ((lfUnaryIdx >= 0) && (rtUnaryIdx > 0)) {
        // if both unary -- error if precedence are equal; otherwise right has precedence
        // get value / resolve unary
        if (entList[lfUnaryIdx]->opPreced == entList[rtUnaryIdx]->opPreced) {
          SS_ERR_ATI_ABORT(ssErr << "Unary operators on both side of '" << entList[elIdx]->strId << "' -- unsupported!" << aa_util_entListDbg(entList,rtUnaryIdx));
          return -1;
        }
      }

      dat_n_idx = elIdx;
      dat_n_var = getIntFromStr(entList[elIdx]->strId,dat_nuevo) ? 0 : 1;
      AA_DEBUG("  ~  ~  ~  ~  ~ '%s' {current isVar: %d} {prevIdx=%d} {prevIsVar=%d}\n",entList[elIdx]->strId.c_str(),dat_n_var,dat_a_idx,dat_a_var);
      elIdx++;

      // if right unary or left unary, resolve if dat_n_var==1
      if ((rtUnaryIdx > 0) || (lfUnaryIdx >= 0)) {
        if (dat_n_var > 0) {
          dat_nuevo = pp_parent->getVarInt( entList[dat_n_idx]->strId );
          dat_n_idx = -1;
          dat_n_var =  0;
        }
      }

      // Perform unary ops -- right has precedence
      if (rtUnaryIdx > 0) {
        switch (entList[rtUnaryIdx]->opIdx)
        {
        case OP_PLUSPLUS:   pp_parent->setVarInt( entList[elIdx-1]->strId, dat_nuevo+1 ); break;
        case OP_MINUSMINUS: pp_parent->setVarInt( entList[elIdx-1]->strId, dat_nuevo-1 ); break;
        default:
          SS_ERR_ATI_ABORT(ssErr << "This state supposedly unreachable!" << aa_util_entListDbg(entList,rtUnaryIdx));
          return -1;
        }
      }
      if (lfUnaryIdx >= 0) {
        switch (entList[lfUnaryIdx]->opIdx)
        {
        case OP_PLUSPLUS:   pp_parent->setVarInt( entList[elIdx-1]->strId, dat_nuevo+1 ); dat_nuevo++; break;
        case OP_MINUSMINUS: pp_parent->setVarInt( entList[elIdx-1]->strId, dat_nuevo-1 ); dat_nuevo--; break;
        case OP_BIT_INV:    dat_nuevo = ~dat_nuevo;                                                    break;
        case OP_LOG_NOT:    dat_nuevo = !dat_nuevo ? 1 : 0;                                            break;
        case OP_MINUS:      dat_nuevo = -dat_nuevo;                                                    break;
        default:
          SS_ERR_ATI_ABORT(ssErr << "This state supposedly unreachable!" << aa_util_entListDbg(entList,rtUnaryIdx));
          return -1;
        }
      }

      if (rtUnaryIdx > 0)
        elIdx++;
    }
    // NOTE: elIdx should be pointing to the next element after this

    // Perform OP
    if (lastOpId > 0) {
      int errState = 0;
      int isAssign = isAssignOp(lastOpId);
      if (isAssign) {
        // Perform assignment OP
        if (dat_a_var != 1) {
          SS_ERR_ATI_ABORT(ssErr << "Op ID '" << lastOpId << "' but LHS is not a variable!" << aa_util_entListDbg(entList,dat_a_idx));
          return -1;
        }
        // If nuevo is a variable, resolve
        if (dat_n_var) {
          dat_nuevo = pp_parent->getVarInt( entList[dat_n_idx]->strId );
          dat_n_var = 0;
        }
        // Perform assignment, and write back
        AA_DEBUG("  ### '%s' <-- %d\n",entList[dat_a_idx]->strId.c_str(),dat_nuevo);
        dat_accum = performAsInt(pp_parent,entList[dat_a_idx]->strId,lastOpId,dat_nuevo);
        dat_a_var = 0;
        pp_parent->setVarInt(entList[dat_a_idx]->strId,dat_accum);
      } else {
        // if dat_a_var==1 or dat_n_var==1, resolve
        if (dat_a_var) {
          dat_accum = pp_parent->getVarInt( entList[dat_a_idx]->strId );
          dat_a_var = 0;
        }
        if (dat_n_var) {
          dat_nuevo = pp_parent->getVarInt( entList[dat_n_idx]->strId );
          dat_n_var = 0;
        }
        // Perform OP
       #ifdef ARITH_DEBUG_DEF
        int printAcc = dat_accum;
       #endif
        dat_accum = performOpInt(lastOpId,dat_accum,dat_nuevo,errState);
        dat_a_idx = -1;
        dat_a_var =  0;
        AA_DEBUG("  ### %d <-- %d op=%d %d\n",dat_accum,printAcc,lastOpId,dat_nuevo);
        if (errState) {
          SS_ERR_ATI_ABORT(ssErr << "Unknown Op ID '" << lastOpId << "'!");   // Supposedly unreachable state
          return -1;
        }
      }
    } else {
      dat_accum = dat_nuevo;
      dat_a_idx = dat_n_idx;
      dat_a_var = dat_n_var;
    }

    // Get next OP
    if (elIdx < (int)entList.size()) {
      if (entList[elIdx]->isUnary) {
        SS_ERR_ATI_ABORT(ssErr << "Unexpected unary operator!" << aa_util_entListDbg(entList,elIdx));
        return -1;
      }
      lastOpId = entList[elIdx]->opIdx;
      if (lastOpId <= 0) {
        SS_ERR_ATI_ABORT(ssErr << "This state supposedly unreachable!" << aa_util_entListDbg(entList,elIdx));
        return -1;
      }
    }

    elIdx++;
  }

  if (dat_a_var)
    dat_accum = pp_parent->getVarInt( entList[dat_a_idx]->strId );
  AA_DEBUG("  ~ returning %d ~~~~~~~~~~~~~\n",dat_accum);
  return dat_accum;
}

int arithElem_c::getIntFromStr(const std::string &strId, int &intVal)
{
  char c = strId[0];
  if (isdigit(c)) {
#if 0
    intVal = std::stoi(strId,nullptr,0);
#else
    try {
      intVal = std::stoi(strId,nullptr,0);
    } catch (const std::out_of_range& oor) {
      if ((strId[0] == '0') && ((strId[1] == 'x') || (strId[1] == 'X'))) {
        auto ullInt = std::stoull(strId,nullptr,0);
        intVal = (int) ullInt;
      } else {
        auto ulInt = std::stoul(strId,nullptr,0);
        intVal = (int) ulInt;
      }
    }
#endif
    return 1;
  }
  return 0;
}

int arithElem_c::performAsInt(std::shared_ptr<arithParser_c> pp_parent, const std::string &varId, int opId, int rhsVal)
{
  int lhsVal = 0;
  if (opId != OP_ASGN_EQ)
    lhsVal = pp_parent->getVarInt( varId );
  switch (opId)
  {
  case OP_ASGN_EQ:       lhsVal   = rhsVal; break;
  case OP_ASGN_PLUS:     lhsVal  += rhsVal; break;
  case OP_ASGN_MINUS:    lhsVal  -= rhsVal; break;
  case OP_ASGN_MULTIPLY: lhsVal  *= rhsVal; break;
  case OP_ASGN_DIVIDE:   lhsVal  /= rhsVal; break;
  case OP_ASGN_MODULUS:  lhsVal  %= rhsVal; break;
  case OP_ASGN_LSHIFT:   lhsVal <<= rhsVal; break;
  case OP_ASGN_RSHIFT:   lhsVal >>= rhsVal; break;
  case OP_ASGN_AND:      lhsVal  &= rhsVal; break;
  case OP_ASGN_XOR:      lhsVal  ^= rhsVal; break;
  case OP_ASGN_OR:       lhsVal  |= rhsVal; break;
  default: pp_parent->exitCall("[arithElem_c::performAsInt] this state supposdely unreachable!"); break;
  }
  pp_parent->setVarInt( varId, lhsVal );
  return lhsVal;
}

int arithElem_c::performOpInt(int opId, int A, int B, int &errFlag)
{
  errFlag = 0;
  switch (opId)
  {
  case OP_MINUS:    return A  - B;
  case OP_LOG_AND:  return A && B ? 1 : 0;
  case OP_LOG_OR:   return A || B ? 1 : 0;
  case OP_LSHIFT:   return A << B;
  case OP_RSHIFT:   return A >> B;
  case OP_CMP_LTEQ: return A <= B ? 1 : 0;
  case OP_CMP_GTEQ: return A >= B ? 1 : 0;
  case OP_CMP_EQ:   return A == B ? 1 : 0;
  case OP_CMP_NEQ:  return A != B ? 1 : 0;
  case OP_MULTIPLY: return A  * B;
  case OP_DIVIDE:   return A  / B;
  case OP_MODULUS:  return A  % B;
  case OP_CMP_LT:   return A  < B ? 1 : 0;
  case OP_CMP_GT:   return A  > B ? 1 : 0;
  case OP_BIT_AND:  return A  & B;
  case OP_BIT_XOR:  return A  ^ B;
  case OP_BIT_OR:   return A  | B;
  case OP_PLUS:     return A  + B;
  default:
    errFlag = 1;
  }
  return 0;
}

int arithElem_c::isAssignOp(int opId)
{
  switch (opId)
  {
  case OP_ASGN_EQ      : return 1;
  case OP_ASGN_PLUS    : return 1;
  case OP_ASGN_MINUS   : return 1;
  case OP_ASGN_MULTIPLY: return 1;
  case OP_ASGN_DIVIDE  : return 1;
  case OP_ASGN_MODULUS : return 1;
  case OP_ASGN_LSHIFT  : return 1;
  case OP_ASGN_RSHIFT  : return 1;
  case OP_ASGN_AND     : return 1;
  case OP_ASGN_XOR     : return 1;
  case OP_ASGN_OR      : return 1;
  }
  return 0;
}

int arithElem_c::isLhsUnaryOp(int opId)
{
  switch (opId)
  {
  case OP_PLUSPLUS  : return 1;
  case OP_MINUSMINUS: return 1;
  case OP_BIT_INV   : return 1;
  case OP_LOG_NOT   : return 1;
  case OP_MINUS     : return 1;
  }
  return 0;
}
//------------------------------------------------------------------------------
std::string aa_util_entListDbg(const std::vector<std::shared_ptr<arithElem_c> > &elemListRef, int pos)
{
  std::stringstream ssMsg;
  std::vector<int>  spList;

  ssMsg << "\n    Equation: [";
  for (auto it1 = elemListRef.cbegin(); it1 != elemListRef.cend(); it1++) {
    if ((*it1)->entList.size() > 0) {
      ssMsg << " ()";
      spList.push_back(2);
    } else {
      ssMsg << " " << (*it1)->strId;
      spList.push_back((*it1)->strId.length());
    }
  }

  ssMsg << " ]\n    Position: [";
  for (auto it2 = spList.cbegin(); it2 != spList.cend(); it2++, pos--) {
    if (pos == 0) {
      ssMsg << " ^";
      if (*it2 > 1)
        ssMsg << std::string((*it2) - 1,' ');
    } else
      ssMsg << " " << std::string(*it2,' ');
  }
  ssMsg << " ]";

  return ssMsg.str();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sorted in lexicographic search order
// Precedence: http://en.cppreference.com/w/cpp/language/operator_precedence
const std::vector<opType_t> opList = {
  {"("  , OP_LEFTPAR      , 1},
  {")"  , OP_RIGHTPAR     , 1},
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



