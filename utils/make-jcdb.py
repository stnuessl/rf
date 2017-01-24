#!/usr/bin/python env

#
# Copyright (C) 2017  Steffen Nüssle
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

def clang_include_directory():
    regex = re.compile(r'[0-9]+\.[0-9]+\.[0-9]+')
    entries = os.listdir('/usr/lib/clang/')
    
    entries = filter(lambda x: regex.match(x), entries)
    entries = sorted(entries, key=distutils.version.StrictVersion)

    if len(entries) == 0:
        raise FileNotFoundError('No clang installation directory found')
    
    return '/usr/lib/clang/{}/include'.format(entries[-1])

def process_command(args):
    if args.raw:
        return args.command
    
    arg_list = args.command.split(' ')
        
    # Filter empty entries
    arg_list = filter(lambda x: x != '', arg_list)
    
    # Remove duplicates
    seen = set()
    arg_list = [x for x in arg_list if not (x in seen or seen.add(x))]
    
    #
    # Add '-Wno-unknown-warning-option' to avoid the clang output:
    #       warning: unknown warning option '-Wno-maybe-uninitialized'; 
    #       did you mean '-Wno-uninitialized'? [-Wunknown-warning-option]
    # which clutters the output as it is printed for each translation unit
    #
    arg_list.append('-Wno-unknown-warning-option')
    
    # Add clang include path
    arg_list.append('-I{}'.format(clang_include_directory()))
    
    def prepend_dashes(l):
        return list(map(lambda x: '-{}'.format(x), l))
    
    # Add additional passed flags 
    if args.add != None:
        arg_list = arg_list + prepend_dashes(args.add)

    # Remove passed flags
    if args.discard != None:
        discard = prepend_dashes(args.discard)
        arg_list = filter(lambda x: x not in discard, arg_list)
    
    # Create one big string again
    command = ' '.join(arg_list)

    if args.discard_dependency_generation:
        '''
        Remove dependency output flags:
            This are commands I found:
                -M -MD -MF -MG -MM -MMD -MP -MQ -MT
            However, these commands have an additional argument:
                -MF -MQ -MT
        '''
        pattern = r'-M([DGP]|M?D?|[FQT]\s+[^\s]*)\b\s*'
        command = re.sub(pattern, '', command)
        
    if args.discard_optimization_levels:
        command = re.sub(r'-O([0-3]|s|g|fast)?\b\s*', '', command)
        
    if args.discard_debugging_levels:
        command = re.sub(r'-g([0-3]|gdb)?\b\s*', '', command)
    
    # Remove remove consecutive spaces
    return re.sub(r' +', ' ', command)
    
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('files',
                        nargs='+',
                        metavar='file',
                        help=('The files used in the compilation process.'))
    parser.add_argument('--command',
                        required=True,
                        help=('The compilation command used passed as one '
                              'string'))
    parser.add_argument('--directory',
                        metavar='<path>',
                        default=os.getcwd(),
                        help=('Path to the directory where the compiler is '
                              'invoked. If none is provided the current '
                              'directory will be used.'))
    parser.add_argument('--no-pp',
                        default=False,
                        action='store_true', 
                        help=('Do not pretty print the compilation '
                        'database.'))
    parser.add_argument('--raw',
                        default=False,
                        action='store_true',
                        help=('Do use the raw input to create the compilation '
                        'database. By default this tool tries to sanitize the '
                        'commands so they work on a system where clang is not '
                        'the default compiler. Ignores all other command '
                        'changing flags.'))
    parser.add_argument('--discard-dependency-generation',
                        default=False,
                        action='store_true',
                        help=('Discards dependency generation arguments '
                              '(-M, -MT, etc.) from the compilation command'))
    parser.add_argument('--discard-optimization-levels',
                        default=False,
                        action='store_true',
                        help=('Discards optimization levels, like e.g. -O2.'))
    parser.add_argument('--discard-debugging-levels',
                        default=False,
                        action='store_true',
                        help=('Discards debugging levels, like e.g. -g3.'))
    parser.add_argument('--add',
                        metavar='<flag>',
                        nargs='+',
                        help=('Add additional compilation flags to the compile '
                        'command. Omit leading dashes.'))
    parser.add_argument('--discard',
                        metavar='<flag>',
                        nargs='+',
                        help=('Discards the passed flags if found in the '
                              'compilation command. Omit leading dashes.'))
    
    args = parser.parse_args()
    
    files = args.files
    command = process_command(args)
    directory = args.directory
    indentation = None if args.no_pp else 4

    func = lambda x: {
        'directory' : directory,
        'command' : '{} {}'.format(command, x),
        'file' : x
    }

    db = list(map(func, files))
    
    output = json.dumps(db, indent=indentation)
    print(output)
    

if __name__ == '__main__':
    main()
