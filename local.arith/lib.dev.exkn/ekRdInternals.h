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
* ekRdInternals.h
* - Private header file
* Author:  manxinator
* Created: Mon Jan 25 03:00:46 PST 2016
*******************************************************************************/

#ifndef __EK_RD_INTERNALS__
#define __EK_RD_INTERNALS__

//------------------------------------------------------------------------------

namespace ex_knobs
{

  class primCommand_c : public primitive_c {
  private:
    std::string             ident;
    std::vector<element_c*> argLst;

  public:
             primCommand_c() : primitive_c(PRIM_COMMAND) {}
    virtual ~primCommand_c() {
      for (auto it = argLst.begin(); it != argLst.end(); it++)
        delete *it;
    }

  private:
    virtual void virt_setIdent  (const std::string &idStr);
    virtual bool virt_getIdentRC(std::string &idStr);

    virtual void virt_setStr  (const std::string& arStr, bool isQ, bool isRhs);
    virtual void virt_setBTick(int btType, const std::string& idStr, const std::string& parenStr, bool isRhs);

    virtual std::vector<element_c*>* virt_getArgListRC(bool &rc);

    virtual void virt_setLineNum(int);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primObject_c : public primitive_c {
  private:
    std::vector<element_c*> argLst;

  public:
             primObject_c() : primitive_c(PRIM_OBJECT) {}
    virtual ~primObject_c() {
      for (auto it = argLst.begin(); it != argLst.end(); it++)
        delete *it;
    }

  private:
    virtual void virt_setStr  (const std::string& arStr, bool isQ, bool isRhs);
    virtual void virt_setBTick(int btType, const std::string& idStr, const std::string& parenStr, bool isRhs);

    virtual std::vector<element_c*>* virt_getArgListRC(bool &rc);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primKnob_c : public primitive_c {
  private:
    std::vector<element_c*> lhsLst;
    std::vector<element_c*> rhsLst;

  public:
             primKnob_c() : primitive_c(PRIM_KNOB) {}
    virtual ~primKnob_c() {
      for (auto it = lhsLst.begin(); it != lhsLst.end(); it++) delete *it;
      for (auto it = rhsLst.begin(); it != rhsLst.end(); it++) delete *it;
    }

  private:
    virtual void virt_setStr  (const std::string& arStr, bool isQ, bool isRhs);
    virtual void virt_setBTick(int btType, const std::string& idStr, const std::string& parenStr, bool isRhs);

    virtual std::vector<element_c*>* virt_getLhsListRC(bool &rc);
    virtual std::vector<element_c*>* virt_getRhsListRC(bool &rc);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primXml_c : public primitive_c {
  private:
    std::string             ident;
    std::vector<element_c*> optLst;
    std::vector<element_c*> lineLst;

  public:
             primXml_c() : primitive_c(PRIM_XML) {}
    virtual ~primXml_c() {
      for (auto it = optLst.begin();  it != optLst.end();  it++) delete *it;
      for (auto it = lineLst.begin(); it != lineLst.end(); it++) delete *it;
    }

  private:
    virtual void virt_setIdent  (const std::string &idStr);
    virtual bool virt_getIdentRC(std::string &idStr);
    virtual void virt_addOpt    (const std::string& optStr, bool isQ);
    virtual void virt_addText   (const std::string& arStr);

    virtual std::vector<element_c*>* virt_getOptionsRC(bool &rc);
    virtual std::vector<element_c*>* virt_getTextRC   (bool &rc);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primCommentSL_c : public primitive_c {
  public:
             primCommentSL_c() : primitive_c(PRIM_COMMENT_SL) {}
    virtual ~primCommentSL_c() { }

  private:
    std::string commentStr;

    virtual void virt_setComment(const std::string &commStr);
    virtual bool virt_getComment(std::string &retStr);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primCommentML_c : public primitive_c {
  public:
             primCommentML_c() : primitive_c(PRIM_COMMENT_ML) {}
    virtual ~primCommentML_c() { }

  private:
    std::string commentStr;

    virtual void virt_setComment(const std::vector<std::string> &commLst);
    virtual bool virt_getComment(std::string &retStr);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  class primFile_c : public primitive_c {
  public:
             primFile_c() : primitive_c(PRIM_FILE) {}
    virtual ~primFile_c() { }

  private:
    std::string fileName;

    virtual void virt_setFileName(const std::string &newFileName);
    virtual bool virt_getFileName(std::string &retStr);
  };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Utility functions
    //

  extern element_c* exkn_str2elem (const std::string& myStr, bool isQ);
  extern element_c* backTickToElem(int btType, const std::string& idStr, const std::string& parenStr);
}

//------------------------------------------------------------------------------

#endif  //__EK_RD_INTERNALS__

