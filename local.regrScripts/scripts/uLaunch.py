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
# Author :  manxinator
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

#-------------------------------------------------------------------------------
class launchDelegate:
  def __init__(self):
    self.prog = basename(sys.argv[0])

    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose',   action='count',                                  help='Increase verbosity')
    parser.add_argument('-l', '--logFile',   nargs=1,   dest='logFile',   default='run_log',  help='Log File')
    parser.add_argument('-r', '--runSeed',   nargs=1,   dest='runSeed',                       help='Random seed')
    parser.add_argument('-P', '--preRun',    nargs=1,   dest='preRun',                        help='Comma-separated list of pre-run scripts')
    parser.add_argument('-p', '--postRun',   nargs=1,   dest='postRun',                       help='Comma-separated list of post-run scripts')
    parser.add_argument('-x', '--extraArgs', nargs='+', dest='extraArgs',                     help='Arguments for the executable')
    parser.add_argument('bin',                                                                help='Executable program to launch')

    self.args = parser.parse_args()
    print "\n-----"
    print "- ", self.args
    print "-"

    #= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    global g_progName
    global g_verbose
    g_progName = self.prog
    g_verbose  = 0 if self.args.verbose is None else self.args.verbose
    g_verbose  = 10 # Temporary override


  def launchAndWait(self,execFile,logHand,exArgs=[]):
    cmd = [execFile] + exArgs

    print "[%s] launching: %s, [%s]" %(self.prog, cmd[0], datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
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
    print "[%s] %s done" %(self.prog, cmd[0])


  def launchList(self,cmdStr,logFileHand):
    cmdList = cmdStr.split(",")
    for oneScript in cmdList:
      print "  + %s" % oneScript
      self.launchAndWait(oneScript,logFileHand)


  def run(self):
    try:
      logFH = open(self.args.logFile[0],'a')
    except IOError:
      sys.stderr.write("Failed to open %s for writing" %self.args.logFile[0])
      sys.exit(-1)

    #---------------------------------------------------------------------------
    # 1- run pre-run scripts in order
    # 2- run program
    # 3- run post-run scripts in order
    #---------------------------------------------------------------------------

    # 1- run pre-run scripts
    #
    if self.args.preRun is not None:
      print "------------------------- preRun: %s" % self.args.preRun
      self.launchList(self.args.preRun[0],logFH)

    # 2- run program
    #
    print "------------------------- run program"
    self.launchAndWait(self.args.bin,logFH,self.args.extraArgs)

    # 3- run post-run scripts
    #
    if self.args.postRun is not None:
      print "------------------------- postRun: %s" % self.args.preRun
      self.launchList(self.args.postRun[0],logFH)

    print "[%s] %s; Done!" %(self.prog, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    print "--------------------------------------------------"


  #-----------------------------------------------------------------------------
  def killChild():
    #
    # Kill child processes -- uLaunch will only have one child process at a time
    #
    global g_child_pid
    global g_progName
    global g_verbose

    if g_child_pid is None:
      if g_verbose > 0:
        sys.stderr.write("\n[%s] launcherCleanup() g_child_pid is None" % g_progName)
    else:
      if g_verbose > 0:
        sys.stderr.write("\n[%s] launcherCleanup() killing g_child_pid=%d" % (g_progName,g_child_pid))
      os.kill(g_child_pid, signal.SIGTERM)

  import atexit
  atexit.register(killChild)

#-------------------------------------------------------------------------------

if __name__ == "__main__":
  launcher = launchDelegate()
  launcher.run()

