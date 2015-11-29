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
* arithmeticAnalyzer.h
* Author:  manxinator
* Created: Fri Nov 13 00:48:18 PST 2015
*******************************************************************************/

#ifndef __ARITHMETIC_ANALYZER__
#define __ARITHMETIC_ANALYZER__

#include <string>
#include <memory>
#include <vector>
#include <functional>

//------------------------------------------------------------------------------

class arithElem_c;

class arithParser_c : public std::enable_shared_from_this<arithParser_c> {
public:

  //++++++++++++++++++++++++++++++++++++
  class arithEqn_c {
  private:
    std::weak_ptr<arithParser_c> p_parent;

  public:
    std::shared_ptr<arithElem_c> topNode;

  public:
             arithEqn_c() { }
    virtual ~arithEqn_c() { }

    // Interface to parent
    void setParent(std::shared_ptr<arithParser_c> parenObj);

    // Delegates - Variables
    // Future Optimization:
    //   - eliminate p_parent check
    //   - call these functions within the evaluate function only
    //   !!! Actually, elminate these functions altogether
    int  get_int(const std::string &);
    void set_int(const std::string &, int);

    // Delegates - Error logging
    void doErr  (const std::string &);
    void doExit (const std::string &);

    // Compute calls
    std::shared_ptr<arithElem_c> compute(void);

    int computeInt(void);

    std::string getStr(void); // DEBUG
  };
  //++++++++++++++++++++++++++++++++++++

private:
  std::shared_ptr<arithParser_c::arithEqn_c> CreateEqObj(void);

  int betIdx;
  int betAbrt;
  int betLstId;
  std::shared_ptr<arithElem_c> BuildEqnTree( std::vector<std::shared_ptr<arithElem_c> > &elemListRef );
  void                         BuildSubEqn (
    std::shared_ptr<arithElem_c>                &curNode,
    std::vector<std::shared_ptr<arithElem_c> >  &elemListRef,
    int subLvl, int prevPreced=-1
  );
  std::string genNodeName(void);

public:
           arithParser_c();
  virtual ~arithParser_c() { }

  // Parsing
  //
  std::shared_ptr<arithEqn_c> parseEqn(const std::string &eqStr);

  // Aux features
  //
  std::string getIdent(void) { return __PRETTY_FUNCTION__; }

public:
  // Delegates
  // Future Optimization:
  //   - eliminate checking for valid delegates in the get/set
  //   - move checking into arithParser_c::CreateEqObj
  //
  std::function<int (const std::string&)>     f_getIntVar;
  std::function<void(const std::string&,int)> f_setIntVar;
  std::function<void(const std::string&)>     f_errFunc;
  std::function<void(const std::string&)>     f_exitFunc;

  int  getVarInt(const std::string &);
  void setVarInt(const std::string &, int);
  void errCall  (const std::string &);
  void exitCall (const std::string &);
};

//------------------------------------------------------------------------------

#endif  //__ARITHMETIC_ANALYZER__

