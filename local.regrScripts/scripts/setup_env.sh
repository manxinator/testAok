#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2015 manxinator
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
#-------------------------------------------------------------------------------
# The purpose of this file is to setup environment variables for
# local and remote directories.
#
# From the Project directory (Local workspace), run:
#  . scripts/setup_env.sh
#-------------------------------------------------------------------------------

export TMP_PROJ_NAME=`pwd`
export TMP_BASE_PWD=`basename ${TMP_PROJ_NAME}`
export TMP_BASE_DIR=`pwd | sed "s/\/${TMP_BASE_PWD}//g"`

export LOCAL_WS_DIR="${TMP_BASE_DIR}/${TMP_BASE_PWD}"
export REMOTE_WS_DIR="${TMP_BASE_DIR}/../aok/aokLib"

export PATH="${LOCAL_WS_DIR}/bin:${LOCAL_WS_DIR}/scripts:${TMP_BASE_DIR}/../aok/local.1.genesis/bin:${TMP_BASE_DIR}/../aok/local.1.genesis/scripts:${REMOTE_WS_DIR}/scripts:${PATH}"

unset TMP_PROJ_NAME
unset TMP_BASE_PWD
unset TMP_BASE_DIR




