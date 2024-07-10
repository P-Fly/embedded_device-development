#!/usr/bin/python

import argparse
import sys
import fnmatch
import os
import subprocess

CHECK_STYLE = 'tools/uncrustify/scripts/checkstyle.py'

ERR_MSG = """
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Some files fail the indentation rules. To fix them automatically run

cd {}
{} -f {}
cd -

If you think your file indentations is correct, then you can skip this check by
doing 'git commit --no-verify'.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
"""

# get the list of all added and modified files
def get_committed_files():
	files = []
	out = subprocess.check_output('git diff-index --cached HEAD'.split())
	for line in out.split('\n'):
		if line != '':
			cells = line.split()
			if cells[4] in ['A', 'M']:
				files.append(cells[5])
	return files

def check_style(files):
	if not(files):
		return 0
	command = [CHECK_STYLE, '-c'] + files
	try:
		subprocess.check_output(command)
	except subprocess.CalledProcessError, e:
		print ERR_MSG.format(os.getcwd(), CHECK_STYLE, " ".join(files))
		print e.output
		return e.returncode
	return 0

def is_c_file(f):
	if f.endswith('.c') or f.endswith('.h'):
		return True
	return False

def main():
	# Prepare a list of committed files that are c files
	files = [f for f in get_committed_files() if is_c_file(f)]
	return check_style(files)

if __name__ == '__main__':
	sys.exit(main())
