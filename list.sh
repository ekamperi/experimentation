#!/bin/sh
for i in `find . -name *.c`
do
    echo `basename $i | sort`
done
