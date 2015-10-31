#!/usr/bin/python
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
# uLaunch.py
# Author:   manxinator
# Created:  Sun Oct 18 00:41:31 PDT 2015
# Language: Python 2.7.6
#-------------------------------------------------------------------------------

import os, sys, argparse, time
import signal, subprocess
from datetime import datetime
from os.path import basename

g_child_pid = None
g_progName  = None
g_verbose   = None
  #
  # TODO:
  # 1- figure out a mechanism for re-running tests
  #   A- inside RUN.* directory
  #   B- from parent directory
  # 2- what to do with random seed?
  #

#-------------------------------------------------------------------------------
class launchDelegate:
  """Launches a set of scripts and programs which constitutes a single test
  """

  def __init__(self):
    self.prog = basename(sys.argv[0])

    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose',    action='count',                                     help='Increase verbosity')
    parser.add_argument('-l', '--logFile',    nargs=1,   dest='logFile',    default=['run_log'],  help='Log File')
    parser.add_argument('-F', '--forceSeed',  nargs=1,   dest='forceSeed',  type=int,             help='Forcibly override random seed')
    parser.add_argument('-r', '--randScript', nargs=1,   dest='randScript',                       help='Specify random seed generator script')
    parser.add_argument('-P', '--preRun',     nargs=1,   dest='preRun',                           help='Comma-separated list of pre-run scripts')
    parser.add_argument('-p', '--postRun',    nargs=1,   dest='postRun',                          help='Comma-separated list of post-run scripts')
    parser.add_argument('-x', '--extraArgs',  nargs='+', dest='extraArgs',                        help='Arguments for the executable')
    parser.add_argument('-t', '--testFile',   nargs=1,   dest='testFile',                         help='Test file')
    parser.add_argument('bin',                                                                    help='Executable program to launch')
    self.args = parser.parse_args()

    #= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    runResultFile = 'result.launchtest.temp'
    if os.path.isfile(runResultFile):
      os.remove(runResultFile)

    #= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    global g_progName
    global g_verbose
    g_progName = self.prog
    g_verbose  = 0 if self.args.verbose is None else self.args.verbose
    g_verbose  = 10 # Temporary override -- TODO: remove this

    #= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    # Introspect argument members that are neither methods nor private
    # This metadata is required
    #
    methodList = [method for method in dir(self.args) if (not callable(getattr(self.args,method))) and (method[0] != '_')]
    strLen     = len(max(methodList,key=len))
    fmtStr     = '{:' + str(strLen) + '}'
    print "********** %s arguments:" % self.prog
    for oneStr in methodList:
      print "* ", fmtStr.format(oneStr), " = ", self.args.__dict__[oneStr]
    print "******************************\n"


  def getExecPath(self,execFile):
    """Returns an array with a single entry -- full path of a given executable
    """
    # TODO: Create a pythonic version of this function
    whichEx = subprocess.Popen(['which',execFile],shell=False,stdout=subprocess.PIPE,preexec_fn=os.setsid)
    exePath = whichEx.stdout.read().strip()
    if len(exePath) == 0:
      sys.stderr.write("ERROR: Executable %s not found!\n" %exePath)
      sys.stderr.flush()
      sys.exit(-2)
    return [exePath]


  def launchAndWait(self,execFile,logHand,exArgs=[]):
    """Launches a single script / executable and waits for its completion
    """
    cmd = self.getExecPath(execFile) + exArgs

    print "[%s] LAUNCH: %s, [%s]" %(self.prog, execFile, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    print "[%s] %s" %(self.prog,cmd)

    # Run
    logHand.write('[%s] * * * * * * * * * * launching %s\n' %(self.prog,cmd[0]) )
    logHand.write('[%s] %s\n' %(self.prog,cmd) )
    logHand.flush()
    childProc = subprocess.Popen(cmd, stdout=logHand, stderr=logHand, shell=False, preexec_fn=os.setsid)

    global g_child_pid
    g_child_pid = childProc.pid

    # Wait via polling, since I couldn't figure out how to kill
    childProc.wait()

    g_child_pid = None
    print "[%s] DONE: %s" %(self.prog, execFile)


  def launchList(self,cmdStr,logFileHand):
    """Splits a comma-separated list of scripts and calls them one at a time
    """
    cmdList = cmdStr.split(",")
    for oneScript in cmdList:
      print "  -- -- -- -- -- -- -- -> %s" % oneScript
      self.launchAndWait(oneScript,logFileHand)


  def run(self):
    """Performs the following operation:
       1- run pre-run scripts in order
       2- run program
       3- run post-run scripts in order
    """

    try:
      logFH = open(self.args.logFile[0],'a')
    except IOError:
      sys.stderr.write("Failed to open %s for writing\n" %self.args.logFile[0])
      sys.stderr.flush()
      sys.exit(-1)

    # 1- run random script and pre-run scripts
    #
    if self.args.randScript is not None:
      print "------------------------- randScript: %s" % self.args.randScript[0]
      forceSeedArgLst = []
      if self.args.forceSeed is not None:
        forceSeedArgLst.append('--forceSeed=%d' % self.args.forceSeed[0])
      self.launchAndWait(self.args.randScript[0],logFH,forceSeedArgLst)

    if self.args.preRun is not None:
      print "------------------------- preRun: %s" % self.args.preRun
      self.launchList(self.args.preRun[0],logFH)

    # 2- run program
    # NOTE:
    # - self.args.testFile is, by default, the first argument
    # - self.args.extraArgs should only be the first line of additional arguments
    # - the rest must be loaded from a file just before calling launchAndWait(...) -- TODO
    #
    print "------------------------- run program"
    l_exArgs = self.args.testFile if self.args.testFile is not None else []
    if self.args.extraArgs is not None:
      l_exArgs = self.args.extraArgs + l_exArgs
    self.launchAndWait(self.args.bin,logFH,l_exArgs)

    # 3- run post-run scripts
    #
    if self.args.postRun is not None:
      print "------------------------- postRun: %s" % self.args.postRun
      logFH.flush()
      self.launchList(self.args.postRun[0],logFH)

    print "++++++++++++++++++++++++++++++++++++++++++++++++++"
    print "+ [%s] %s Done! [%s]" %(self.prog, self.prog, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    print "++++++++++++++++++++++++++++++++++++++++++++++++++"


  #-----------------------------------------------------------------------------
  def killChild():
    """Kill child process
       Note: this script will only have one child process at a time
    """
    global g_child_pid
    global g_progName
    global g_verbose

    sys.stdout.flush()
    if g_child_pid is None:
      if g_verbose > 0:
        sys.stderr.write("\n[%s] killChild() g_child_pid is None\n" % g_progName)
    else:
      if g_verbose > 0:
        sys.stderr.write("\n[%s] killChild() killing g_child_pid=%d\n" % (g_progName,g_child_pid))
      os.kill(g_child_pid, signal.SIGTERM)

  import atexit
  atexit.register(killChild)

#-------------------------------------------------------------------------------

if __name__ == "__main__":
  launcher = launchDelegate()
  launcher.run()

