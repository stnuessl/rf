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

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', required=True, 
                        help='Specifies the path to the compilation database')
    parser.add_argument('flags', nargs='+', 
                        help='Flags to add to the compile commands')
    parser.add_argument('--stdout', action='store_true',
                        help='Write the new compilation database to stdout')
    parser.add_argument('-p', '--pretty', action='store_true',
                        help='Pretty print the compilation database')
    parser.add_argument('-r', '--remove', action='store_true',
                        help='Remove specified flags, if applicable')

    args = parser.parse_args()

    db_file = open(args.file, 'r+')
    db = json.load(db_file)
    
    if args.remove:
        flag_set = set(args.flags)
        
        for entry in db:
            flags = entry['command'].split(' ')
            entry['command'] = ' '.join([x for x in flags if x not in flag_set])
    else:
        flags_str = ' '.join(args.flags)
        
        for entry in db:
            entry['command'] = '{} {}'.format(entry['command'], flags_str)
            
    if args.pretty:
        args.pretty = 4
    else:
        args.pretty = None
    
    out = json.dumps(db, indent=args.pretty)
    
    if args.stdout:
        db_file.close()
        db_file = sys.stdout
    else:
        db_file.seek(0)
        db_file.truncate(0)
    
    print('{}'.format(out), file=db_file)

if __name__ == '__main__':
    main()