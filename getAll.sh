#!/bin/bash

for i in {NYSE,NASDAQ,AMEX}
do
    root -l -b -q 'Analyzer.C("Symbols/'$i'.txt")'
done
