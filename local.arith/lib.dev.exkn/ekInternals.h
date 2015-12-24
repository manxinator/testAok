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
* ekInternals.h
* - Private header file
* Author:  manxinator
* Created: Wed Dec 23 03:09:21 PST 2015
*******************************************************************************/

#ifndef __EK_INTERNALS__
#define __EK_INTERNALS__

#include <memory>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

extern void ek_internInit  (void);
extern void ek_parserClenup(void);

  // Commands
extern void ek_commandIdent(const char* dbgStr, const char *cmdId);
extern void ek_commandArgs (const char* dbgStr, const char *cmdArgs);
extern void ek_commandQStr (const char* dbgStr, std::shared_ptr<std::vector<std::string> > quoteStr);

extern std::shared_ptr<std::vector<std::string> > ek_collectQStr(void);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef EKDEBUG
  #define E_DEBUG(...)   printf(__VA_ARGS__)
#else
  #define E_DEBUG(...)
#endif

//------------------------------------------------------------------------------

#endif  //__EK_INTERNALS__

