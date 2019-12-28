#!/usr/bin/python
# -*- coding: utf-8 -*-

##
# wsolver, whitespace solver
# Copyright (C) 2019 Toni Helminen
#
# wsolver is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# wsolver is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.
# If not, see <http://www.gnu.org/licenses/>.
##

import sys, os, time

NAME    = "wsolver"
VERSION = "1.1"
AUTHOR  = "Toni Helminen"

IGNORE_FILES  = ("makefile", "logo.jpg", "lastemperor", "old")
TAB_TO_SPACES = "  "

def file_list():
  flist = []
  for root, dirs, files in os.walk(".", topdown=False):
    for name in files:
      if not name.startswith(IGNORE_FILES):
        flist.append(os.path.join(root, name))
  return flist

def tabs2spaces(flist):
  for name in flist:
    f = open(name, 'r')
    s = f.read()
    s = s.replace("\t", TAB_TO_SPACES)
    f.close()
    f = open(name, 'w')
    f.write(s)
    f.close()

def cleanup_whitespace(flist):
  for name in flist:
    f, lines = open(name, "r"), []
    for s in f.readlines():
      lines.append(s.rstrip())
    f.close()
    f = open(name, "w")
    f.write("\n".join(lines))
    f.close()

def go():
  flist = file_list()
  cleanup_whitespace(flist)
  tabs2spaces(flist)
  return len(flist)

def main():
  print("{ # Version")
  print("  name        = %s," % (NAME))
  print("  version     = %s," % (VERSION))
  print("  author      = %s," % (AUTHOR))
  print("  description = Removes whitespaces and replaces tabs with spaces")
  print("}\n")
  print "> Working ...\n"
  start = time.time()
  n = go()
  print("{ # Job done!")
  print("  time          = %.3fs," % (time.time() - start))
  print("  files_touched = %d" % (n))
  print("}")

if __name__ == "__main__":
  main()