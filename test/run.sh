#!/usr/bin/bash

MD5=$(md5sum main | cut -f1 -d " ");

rf --tag n::a=aa,b=bb,c=cc && rf --syntax-only;
rf --tag n::aa=a,bb=b,cc=c && rf --syntax-only;
g++ -Wall -std=c++11 -o main main.cpp;

if [ "$MD5" != "$(md5sum main | cut -f1 -d " ")" ]; then
    echo "**WARNING: MD5 sum of 'main' changed!";
fi

exit;
