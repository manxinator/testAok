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

#-------------------------------------------------------------------------------
class launchDelegate:
  def __init__(self):
    self.prog = basename(sys.argv[0])
    global g_progName
    g_progName = self.prog

    parser = argparse.ArgumentParser()
    parser.add_argument('-v', '--verbose',   action='count',                                help='Increase verbosity')
    parser.add_argument('-l', '--logFile',   nargs=1,   dest='logFile',   default='run_log',  help='Log File')
    parser.add_argument('-r', '--runSeed',   nargs=1,   dest='runSeed',                       help='Random seed')
    parser.add_argument('-P', '--preRun',    nargs=1,   dest='preRun',                        help='Comma-separated list of pre-run scripts')
    parser.add_argument('-p', '--postRun',   nargs=1,   dest='postRun',                       help='Comma-separated list of post-run scripts')
    parser.add_argument('-x', '--extraArgs', nargs='+', dest='extraArgs',                     help='Arguments for the executable')
    parser.add_argument('bin',                                                              help='Executable program to launch')

    self.args = parser.parse_args()
    print "\n-----"
    print "- ", self.args
    print "-"

  def launchAndWait(self,execFile,logHand,exArgs):
    cmd = [self.args.bin] + exArgs

    print "[%s] %s; launching:" %(self.prog, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    print "[%s] %s" %(self.prog,cmd)

    # Run
    global g_child_pid
    childProc = subprocess.Popen(cmd, stdout=logHand, stderr=logHand, shell=False, preexec_fn=os.setsid)
    g_child_pid = childProc.pid

    # Wait via polling, since I couldn't figure out how to kill
    childProc.wait()

    g_child_pid = None
    print "[%s] %s; Done!" %(self.prog, datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'))
    print "--------------------------------------------------"

  def run(self):
    try:
      logFH = open(self.args.logFile[0],'a')
    except IOError:
      sys.stderr.write("Failed to open %s for writing" %self.args.logFile[0])
      sys.exit(-1)

    # TODO:
    # 1- run pre-run scripts in order
    # 2- run program -- log file should be configurable
    # 3- run post-run scripts in order

    # 2- run program
    #
    self.launchAndWait(self.args.bin,logFH,self.args.extraArgs)


  #-----------------------------------------------------------------------------
  def killChild():
    #
    # Kill child processes -- uLaunch will only have one child process at a time
    #
    global g_child_pid
    global g_progName

    if g_child_pid is None:
      sys.stderr.write("\n[%s] launcherCleanup() g_child_pid is None" % g_progName)
    else:
      sys.stderr.write("\n[%s] launcherCleanup() killing g_child_pid=%d" % (g_progName,g_child_pid))
      os.kill(g_child_pid, signal.SIGTERM)

  import atexit
  atexit.register(killChild)

#-------------------------------------------------------------------------------

if __name__ == "__main__":
  launcher = launchDelegate()
  launcher.run()

