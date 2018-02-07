#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
execute the shell command
"""

import os
import subprocess

class ShellCmd(object):

    """
    execute the shell command
    """
    @staticmethod
    def execute(cmd):
        val = os.popen(cmd).readlines()
        return val

    @staticmethod
    def executeWithStatus(cmd):
        handle = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        handle.wait()
        return handle.returncode, handle.stdout.read()

