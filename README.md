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
// Define the stock by its symbol, frequency of the data,
// and starting date
TStock *Stock = new TStock("GOOGL","1wk","2017-01-01 00:00:00");

// Load an offline database
TFile *f = new TFile("Output/SymbolsDB.root","READ");
Stock->SetDBFile(f);
// Load data from database
Stock->GetData();

// BollingerBands for a period of 20 weeks
// and 2.0 standard deviations for the upper
// and lower bands
// Check ROOT Draw options for "A4"
Stock->GetBollingerBands(20, 2.0)->Draw("A4")

// Take the Simple Moving Average with a
// 10 weeks interval (fast)
TGraph *sma10 = Stock->GetSMA(10)
sma10->SetLineColor(kRed)
sma10->Draw()

// Draw a slower moving average and Candlestick plot
Stock->GetSMA(30)->Draw()
Stock->GetCandleStick()->Draw();

```

![Candlestick example](https://i.imgur.com/3Ftp2Et.png)

Check `Analyzer.C` for examples on how to query the graphs!