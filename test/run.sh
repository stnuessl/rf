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
#

g++ -Wall -std=c++11 -o main main.cpp;
MD5=$(md5sum main | cut -f1 -d " ");

rf --tag n::a=aa,b=bb,c=cc && rf --syntax-only;
rf --tag n::aa=a,bb=b,cc=c && rf --syntax-only;
g++ -Wall -std=c++11 -o main main.cpp;

if [ "$MD5" != "$(md5sum main | cut -f1 -d " ")" ]; then
    echo "**WARNING: MD5 sum of 'main' changed!";
fi

exit;
