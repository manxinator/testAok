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
* ekRead.h
* - Public header file
* Author:  manxinator
* Created: Wed Dec 23 03:06:09 PST 2015
*******************************************************************************/

#ifndef __EK_READ__
#define __EK_READ__

#include <string>
#include <vector>
#include <memory>
#include <functional>

//------------------------------------------------------------------------------

namespace ex_knobs
{
  typedef enum _elementType_e_ {
    ELEM_UNDEF     = 0,
    ELEM_STRING    = 1,
    ELEM_QSTRING   = 2,
    ELEM_EQUATION  = 3,
    ELEM_FUNCTION  = 4,
    ELEM_EXPANSION = 5
  } elementType_e;

  typedef enum _primitiveType_e_ {
    PRIM_UNDEF      = 0,
    PRIM_COMMAND    = 1,
    PRIM_OBJECT     = 2,
    PRIM_KNOB       = 3,
    PRIM_XML        = 4,
    PRIM_COMMENT_SL = 5,
    PRIM_COMMENT_ML = 6,
    PRIM_FILE       = 1001
  } primitiveType_e;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class element_c {
  public:
             element_c(elementType_e eType) { elemType = eType; }
    virtual ~element_c() {}

    elementType_e getElemType(void) { return elemType; }

  private:
    elementType_e elemType;
  };

  class elemStr_c : public element_c {
  public:
    std::string varStr;

  public:
             elemStr_c() : element_c(ELEM_STRING) {}
    virtual ~elemStr_c() {}
  };

  class elemQStr_c : public element_c {
  public:
    std::string varStr;
    int         isDoubleQuote;

  public:
             elemQStr_c() : element_c(ELEM_QSTRING) { isDoubleQuote = 1; }
    virtual ~elemQStr_c() {}
  };

  class elemFunc_c : public element_c {
  public:
    std::string identStr;
    std::string parenStr;

  public:
             elemFunc_c() : element_c(ELEM_FUNCTION) { }
    virtual ~elemFunc_c() {}
  };

  class elemEqn_c : public element_c {
  public:
    std::string parenStr;

  public:
             elemEqn_c() : element_c(ELEM_EQUATION) { }
    virtual ~elemEqn_c() {}
  };

  class elemExp_c : public element_c {
  public:
    std::string identStr;
    std::string parenStr;

  public:
             elemExp_c() : element_c(ELEM_EXPANSION) { }
    virtual ~elemExp_c() {}
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primitive_c {
  public:
             primitive_c(primitiveType_e pType) { primType = pType; lineNum = -1; }
    virtual ~primitive_c() { }

    void setLineNum(int);
    int  getLineNum(void);

    primitiveType_e getPrimitiveType(void);

    // XML and Command
    void setIdent  (const std::string &idStr);
    bool getIdentRC(std::string &idStr);
      // Return Code:
      //   True  - value placed in idStr
      //   False - idStr untouched

    // Command, Object, Knob
    void setStr  (const std::string& arStr, bool isQ, bool isRhs=false);
    void setBTick(int btType, const std::string& idStr, const std::string& parenStr, bool isRhs=false);

    // Command and Object
    std::vector<element_c*>* getArgListRC(bool &rc);
    // Knob
    std::vector<element_c*>* getLhsListRC(bool &rc);
    std::vector<element_c*>* getRhsListRC(bool &rc);

    // XML
    void addOpt (const std::string& arStr, bool isQ);
    void addText(const std::string& arStr);

    // Comments
    void setComment  (const std::string &commStr);
    void setComment  (const std::vector<std::string> &commLst);
    bool getCommentRC(std::string &retStr);
      // Return Code:
      //   True  - value placed in retStr
      //   False - retStr untouched

    // XML
    std::vector<element_c*>* getOptionsRC(bool &rc);
    std::vector<element_c*>* getTextRC   (bool &rc);

    // File
    void setFileName(const std::string &newFileName);
    bool getFileName(std::string &retStr);
      // Return Code:
      //   True  - value placed in retStr
      //   False - retStr untouched

  protected:
    int lineNum;

  private:
    primitiveType_e primType;

    virtual void virt_setLineNum(int);

    virtual void virt_setIdent  (const std::string &idStr);
    virtual bool virt_getIdentRC(std::string &idStr);
    virtual void virt_addOpt    (const std::string& arStr, bool isQ);
    virtual void virt_addText   (const std::string& arStr);

    virtual void virt_setStr  (const std::string& arStr, bool isQ, bool isRhs);
    virtual void virt_setBTick(int btType, const std::string& idStr, const std::string& parenStr, bool isRhs);

    virtual void virt_setComment(const std::string &commStr);
    virtual void virt_setComment(const std::vector<std::string> &commLst);
    virtual bool virt_getComment(std::string &retStr);

    virtual std::vector<element_c*>* virt_getArgListRC(bool &rc);
    virtual std::vector<element_c*>* virt_getLhsListRC(bool &rc);
    virtual std::vector<element_c*>* virt_getRhsListRC(bool &rc);

    virtual std::vector<element_c*>* virt_getOptionsRC(bool &rc);
    virtual std::vector<element_c*>* virt_getTextRC   (bool &rc);

    virtual void virt_setFileName(const std::string &newFileName);
    virtual bool virt_getFileName(std::string &retStr);
  };

  std::shared_ptr<primitive_c> primitive_factory (primitiveType_e primType);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  /*
    Valid get functions of each primitive type are as follows:
      getLineNum() is valid for all primitive types

      PRIM_COMMAND:
        - getIdentRC()
        - getArgListRC()

      PRIM_OBJECT:
        - getArgListRC()

      PRIM_KNOB:
        - getLhsListRC()
        - getRhsListRC()

      PRIM_XML:
        - getIdentRC()
        - getOptionsRC()
        - getTextRC()

      PRIM_COMMENT_SL:
      PRIM_COMMENT_ML:
        - getCommentRC()

      PRIM_FILE:
        - getFileName()
  */

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Utility functions
    //

  extern void spQStrToStr (std::shared_ptr<std::vector<std::string> > quoteStr, std::string &destPtr);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Interface functions
    //

  extern int ek_readfile(const char* inFN, int exitOnErr);

  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_command_f;
  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_object_f;
  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_knob_f;
  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_comment_sl_f;
  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_comment_ml_f;
  extern std::function<void(std::shared_ptr<primitive_c>)>  ek_xml_f;
}

//------------------------------------------------------------------------------

#endif  //__EK_READ__

