# TStock

The script `GetDBFile.C` can be used to create `Output/SymbolsDB.root`, an offline DB to get Stock Price Data.

To create the DB File, you can run:

``` bash
for i in {NYSE,NASDAQ,AMEX}
do  
root -l -b -q 'GetDBFile.C("Symbols/'$i'.txt")'
done
```

The method SetDBFile() can be called to set the file as the source of source:

``` c++
#include "TStock.h"
TStock *Stock = new TStock("DDD","1wk","2017-01-01 00:00:00");
TFile *f = new TFile("Output/SymbolsDB.root","READ");
Stock->SetDBFile(f);
Stock->GetData();
Stock->GetCandleStick()->Draw("A");
```