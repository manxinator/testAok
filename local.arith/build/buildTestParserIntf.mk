#---------------------------------------------------------------------
# Copyright (c) 2016 manxinator
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#---------------------------------------------------------------------
# App-specific Top Level Makefile
# Author : manxinator
# Created: Sat Jan 16 02:08:10 PST 2016
#---------------------------------------------------------------------

  # AOK Library is in the remote
  #
AOKLIB_WS_DIR:=$(REMOTE_WS_DIR)

# compilation options
MAKE_DEBUG=1
MAKE_STRICT=1


  # Options
  #
PLATFORM_NAME = $(shell uname | tr '[:lower:]' '[:upper:]')
LOCAL_CFLAGS+=-DPLATFORM_${PLATFORM_NAME}=1 -DPLATFORM_NAME='"${PLATFORM_NAME}"'

# Profiling
#LOCAL_CFLAGS+=-pg
#LIBLINKS+=-lc_p    # since LDFLAGS is not used -- also, use only with 'ld' linker

EXE?=runTestParserIntf

ifdef REV_ID
  OBJ_DIR?=${LOCAL_WS_DIR}/bin/${REL_ID}/${UNAME}/obj-${EXE}
  TARGET_DIR?=${LOCAL_WS_DIR}/bin/${REL_ID}/${UNAME}
else
  OBJ_DIR?=${LOCAL_WS_DIR}/bin/obj-${EXE}
  TARGET_DIR?=${LOCAL_WS_DIR}/bin/
endif

#---------------------------------------------------------------------

CXX_FILES+=testParserIntf1.cpp
#CXX_FILES+=dev.testParserIntf1.cpp
INCPATH+=  ${LOCAL_WS_DIR}/app.testParserIntf/
vpath %c   ${LOCAL_WS_DIR}/app.testParserIntf/
vpath %cpp ${LOCAL_WS_DIR}/app.testParserIntf/

include ${LOCAL_WS_DIR}/lib.dev.exkn/make.exKn.mk
include ${LOCAL_WS_DIR}/lib.dev.arithAnal/make.arithAnal.mk
include ${LOCAL_WS_DIR}/lib.dev.parserIntf/make.parserIntf.mk
#include ${LOCAL_WS_DIR}/lib.stage.parserIntf/make.parserIntf.mk

        #-------------------------------------------------------------

include ${AOKLIB_WS_DIR}/build/rules.cpp.make



