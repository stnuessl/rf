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
import json
import sys
import re

def prepend_dashes(arg_list):
    if arg_list == None:
        return list()
    
    return list(map(lambda x: '-{}'.format(x), arg_list))

def get_compilation_database(path):
    return json.load(open(path, 'r'))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('file',
                        metavar='file',
                        help=('Path to the compilation database'))
    parser.add_argument('--add',
                        nargs='+',
                        metavar='<flag>',
                        help=('Adds the passed flags to the compilation '
                              'command. Omit leading dashes as this will '
                              'interfere with this program argument parser.'))
    parser.add_argument('--discard',
                        nargs='+',
                        metavar='<flag>',
                        help=('Discards the passed flags from the compilation '
                              'command. Omit leading dashes as this will '
                              'interfere with this program argument parser.'))
    parser.add_argument('--inplace', '-i',
                        default=False,
                        action='store_true',
                        help=('Do not print the updated compilation database '
                              'to stdout but save the changes inplace.'))
    parser.add_argument('--no_pp',
                        default=False,
                        action='store_true',
                        help=('Do not pretty print the changed compilation '
                              'database.'))

    args = parser.parse_args()

    db = get_compilation_database(args.file)
    add_list = prepend_dashes(args.add)
    discard_list = prepend_dashes(args.discard)
    indentation = None if args.no_pp else 4
    dest = open(args.file, 'w') if args.inplace else sys.stdout
    
    for entry in db:
        for x in discard_list:
            entry['command'] = entry['command'].replace(x, '')
        
        if len(add_list) > 0:
            entry['command'] = entry['command'] + ' ' + ' '.join(add_list)
        
        # Remove consecutive spaces
        entry['command'] = re.sub(r' +', ' ', entry['command'])
            
    data = json.dumps(db, indent=indentation)
    
    print('{}'.format(data), file=dest)

if __name__ == '__main__':
    main()
