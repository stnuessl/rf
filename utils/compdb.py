#!/usr/bin/python env

#
# Copyright (C) 2016  Steffen Nüssle
# rf - refactor
#
# This file is part of rf.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import os
import json
import re
import distutils.version

def clang_dir():
    regex = re.compile('[0-9]+\.[0-9]+\.[0-9]+')
    entries = os.listdir('/usr/lib/clang/')
    
    entries = filter(lambda x: regex.match(x), entries)
    entries = sorted(entries, key=distutils.version.StrictVersion)
    
    if len(entries) == 0:
        raise FileNotFoundError('No clang installation directory found')
    
    return '/usr/lib/clang/{}'.format(entries[-1])

def remove_dups(s):
    filter = set()
    uniq = list()
    
    for x in s.split(' '):
        if not x in filter:
            filter.add(x)
            uniq.append(x)

    return ' '.join(uniq)

def sanitize_command(cmd, clang_dir):
    # This list is by all means not complete
    subs = [
        ('-I/usr/include', ''),
        # clang throws are warning for this and suggests -Wno-uninitialized
        ('-Wno-maybe-uninitialized', ''),
    ]
    
    # clang frontend expects to use its own headers
    cmd = '{} -I{}/include'.format(cmd, clang_dir)

    for sub in subs:
        cmd = cmd.replace(*sub)

    cmd = re.sub(' +', ' ', cmd)
    
    return remove_dups(cmd)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--source', nargs='+', required=True, 
                        help='Source files to store in the compilation \
                        database')
    parser.add_argument('-c', '--command', required=True,
                        help='Compilation command for all source files.')
    parser.add_argument('-d', '--dir', required=False, 
                        help='Working directory for the compilation command. \
                        Will use the current one if not specified.')
    parser.add_argument('--clang-dir', help='Specify the clang installation \
                        dir. If none is provided this program will try to find \
                        one.')
    parser.add_argument('-p', '--pretty', action='store_true',
                        help='Pretty print the compilation database')
    
    args = parser.parse_args()

    if args.dir == None:
        args.dir = os.getcwd();
        
    if args.clang_dir == None:
        args.clang_dir = clang_dir()
        
    if args.pretty:
        args.pretty = 4
    else:
        args.pretty = None
    
    args.command = sanitize_command(args.command, args.clang_dir)
    
    func = lambda x: {
        'directory' : args.dir,
        'command' : '{} {}'.format(args.command, x),
        'file' : x
    }

    db = list(map(func, args.source))
          
    print(json.dumps(db, indent=args.pretty))

if __name__ == '__main__':
    main()
