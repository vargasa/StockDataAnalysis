# TStock

# How to create an offline Database?

The script `GetDBFile.C` can be used to create `Output/SymbolsDB.root`, an offline DB to get Stock Price Data.

To create the DB File, you can run:

``` bash
for i in {NYSE,NASDAQ,AMEX}
do  
root -l -b -q 'GetDBFile.C("Symbols/'$i'.txt","1wk","2000-01-01 00:00:00")'
done
```
The first argument for `GetDBFile.C` is a txt file containing a list of symbols, second is the frequency `1d` `1wk` or `1mo`, and the last argument is the starting date. By default will download all the data available from `2000-01-01` to `now`.

# How to draw a Candlestick plot?

The method `SetDBFile()` can be called to set the file as the source of data:

``` c++
#include "TStock.h"
TStock *Stock = new TStock("DDD","1wk","2017-01-01 00:00:00");
TFile *f = new TFile("Output/SymbolsDB.root","READ");
Stock->SetDBFile(f);
Stock->GetData();
Stock->GetCandleStick()->Draw("A");
```

![Candlestick example](https://i.imgur.com/MNdmjq6.png)

