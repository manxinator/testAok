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

//------------------------------------------------------------------------------

void func_command(ex_knobs::primCommand_c* cmdPrim)
{
  printf("+++ [func_command] line: %d { %s",cmdPrim->lineNum,cmdPrim->ident.c_str());
  for (auto it = cmdPrim->argLst.begin(); it != cmdPrim->argLst.end(); it++) {
    ex_knobs::element_c::elementType_e elem_type = (*it)->elemType;
    switch (elem_type)
    {
    case ex_knobs::element_c::ELEM_STRING:  printf(", %s",    static_cast<ex_knobs::elemStr_c*> (*it)->varStr.c_str()); break;
    case ex_knobs::element_c::ELEM_QSTRING: printf(", \'%s\'",static_cast<ex_knobs::elemQStr_c*>(*it)->varStr.c_str()); break;
    default:
      printf(", ELEM_TYPE:%d",static_cast<int>(elem_type)); break;
    }
  }
  printf(" }\n");
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("ERROR: %s requires argument <INPUT_FILENAME>\n",argv[0]);
    return EXIT_FAILURE;
  }

  ex_knobs::ek_command_f = std::bind(&func_command,std::placeholders::_1);

  char* inpFN  = argv[1];
  int   retVal = ex_knobs::ek_readfile(inpFN,0);
  printf("~\n~ [main] argc: %d, argv[0]: %s\n",argc,inpFN);
  printf("~\n~ [main] aok_readfile() returned %d!\n~\n",retVal);
  return 0;
}

//------------------------------------------------------------------------------

