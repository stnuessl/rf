#!/usr/bin/bash

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

set -e

function md5_create() {
    printf "$(md5sum "$1" | cut -f1 -d " ")"
}

function md5_compare() {
    if [ "$1" == "$2" ]; then
        return 0
    else
        return 1
    fi
}

python ../utils/make-jcdb.py                            \
            --command "g++ -std=c++11 -o main"          \
            --raw                                       \
            -- "main.cpp"                               \
            > compile_commands.json;


g++ -Wall -std=c++11 -o main main.cpp;
md5_src="$(md5_create main.cpp)";
md5_bin="$(md5_create main)";

rf --tag n::a=aa,b=bb,c=cc,main::ba=baba                \
    --function f=ff                                     \
    --macro M=MM                                        \
    --namespace n=nn,p=pp,p::p=pp,main::o=oo            \
    --variable p::p::x=xx;
rf --syntax-only;
g++ -Wall -std=c++11 -o main main.cpp
rf --tag nn::aa=a,bb=b,cc=c,main::baba=ba               \
    --function ff=f                                     \
    --macro MM=M                                        \
    --namespace nn=n,pp=p,pp::pp=p,main::oo=o           \
    --variable pp::pp::xx=x;
rf --syntax-only;

# Make sure nothing changed
g++ -Wall -std=c++11 -o main main.cpp;

if md5_compare "$md5_src" "$md5_create main.cpp"; then
    printf "**WARNING: MD5 sum of 'main.cpp changed!\n";
fi

if md5_compare "$md5_bin" "$md5_create main"; then
    printf "**WARNING: MD5 sum of 'main changed!\n";
fi

# Basically, the same as above
rf --from-file do_replacements.yaml;
rf --syntax-only;
g++ -Wall -std=c++11 -o main main.cpp
rf --from-file undo_replacements.yaml;
rf --syntax-only;

# Again, make sure nothing changed
g++ -Wall -std=c++11 -o main main.cpp;

if md5_compare "$md5_src" "$md5_create main.cpp"; then
    printf "**WARNING: MD5 sum of 'main.cpp changed!\n";
fi

if md5_compare "$md5_bin" "$md5_create main"; then
    printf "**WARNING: MD5 sum of 'main changed!\n";
fi

exit;
