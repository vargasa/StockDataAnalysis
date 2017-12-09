#!/bin/bash

for i in {NYSE,NASDAQ,AMEX}
do
    while read symbol; do
	root -l -q -b 'Analyzer.C("'$symbol'","1wk","2009-01-01 00:00:00","2017-12-10 00:00:00")'
    done < Symbols/$i.txt
done
