# From a Daily.csv file (which contains tick by tick data of
# a portfolio, Output/pvalue.csv is created which is a daily
# summary with open, high, low, and close prices

import numpy as np
import pandas as pd

filename = 'Daily.csv'

f = pd.read_csv( filename,
                names=['date','pvalue','evalue'],
                index_col='date',
                parse_dates = True)

dfin = pd.DataFrame(f)
ByDatedf = [iday[1] for iday in dfin.groupby(dfin.index.date)]

dfout = pd.DataFrame(columns=['date','open','high','low','close'])

for i in range(0, len(ByDatedf)):
    idf = ByDatedf[i]
    dfout.loc[i] = [ idf.index.date[1],
                     idf.loc[idf.pvalue.first_valid_index()].pvalue,
                     idf.loc[idf.pvalue.idxmax()].pvalue,
                     idf.loc[idf.pvalue.idxmin()].pvalue,
                     idf.loc[idf.pvalue.last_valid_index()].pvalue]

dfout.to_csv('Output/pvalue.csv', index = False);
