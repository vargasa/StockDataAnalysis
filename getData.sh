#!/usr/bin/sh

# Modified from:
# https://stackoverflow.com/questions/44498924/wget-cant-download-yahoo-finance-data-any-more
# Usage:
# $ sh getData.sh SOXL 1wk "Jan 01 2011" "Nov 09 2017" /tmp/
# Freq: 1d, 1wk, 1mo
# SYMBOL Freq StartDate EndDate
# CSV File return Date,Open,High,Low,Close,Adj Close,Volume

symbol=$1
folder=$5

first_date=$(date -d "$3" '+%s')
last_date=$(date -d "$4" '+%s')

alias wget='wget -q -o /dev/null --no-check-certificate'

wget --tries=3 --save-cookies=/tmp/ycookie.txt https://finance.yahoo.com/quote/$symbol/?p=$symbol -O /tmp/ycrumb.store

crumb=$(grep 'root.*App' /tmp/ycrumb.store | sed 's/,/\n/g' | grep CrumbStore | sed 's/"CrumbStore":{"crumb":"\(.*\)"}/\1/')

if wget --load-cookies=/tmp/ycookie.txt "https://query1.finance.yahoo.com/v7/finance/download/$symbol?period1=$first_date&period2=$last_date&interval=$2&events=history&crumb=$crumb" -O $folder/$symbol.csv; then
    return 0 #Sucessful
else
    return 1 #Unsucessful
fi
