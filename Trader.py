# Need to set USERNAME and PASSWORD for Robinhood account

import threading
import time
import csv, time
from datetime import datetime
from Robinhood import Robinhood

class BackThread:
    
    def __init__(self, trader):
        self.trader = trader
        thread = threading.Thread(target=self.run, args=())
        thread.daemon = True
        thread.start()
        
    def SecOwned(self) :
        
        posinfo = []
        
        dsecowned = trader.securities_owned()['results']
        
        for position in dsecowned:
            
            idstring = position['instrument'].split('/')[4]
            shares = float(position['quantity'])
            if shares > 0 :
                stock = trader.instrument(idstring)
                symbol = stock['symbol']
                name = stock['name']
                qdata = trader.quote_data(symbol)
                pricenow = float(qdata['last_trade_price'])
                prevclose = float(qdata['previous_close'])
                avprice = float(position['average_buy_price'])
                daychange = 100*(pricenow - prevclose)/prevclose
                if avprice!= 0 : netprofit = 100*(pricenow - avprice)/avprice
                posinfo.append([symbol, avprice, pricenow, daychange, idstring])
                if daychange > 5.0  or daychange < -2.0 or netprofit < -5.0 :
                    msg = "{}\n\t{} Requires attention\n\tDailyChange: {:.2f}%\n\tNetProfit: {:.2f}%\n"
                    print msg.format(datetime.now(),symbol,daychange,netprofit)
        return posinfo
            
    def run(self):
        
        openMarket = datetime.now().replace(hour=9, minute=30, second=0, microsecond=0)
        closeMarket = datetime.now().replace(hour=16, minute=0, second=0, microsecond=0)
        
        dfile = open('Daily.csv', 'a+');
        
        with dfile as csvfile:
            filewriter = csv.writer(csvfile, delimiter=',',
                                    quotechar='|',
                                    quoting=csv.QUOTE_MINIMAL)
            while True:
                
                date = datetime.now()
                if date.weekday() < 5 and date > openMarket and date < closeMarket :
                    equity = trader.equity()
                    marketValue = trader.market_value()                
                    filewriter.writerow([date.strftime("%Y-%m-%d %H:%M:%S"),equity,marketValue])
                    dfile.flush()
                    #print (date.strftime("%Y-%m-%d %H:%M:%S") , marketValue , equity)
                    self.SecOwned()
                    time.sleep(10)
                else :
                    time.sleep(60)
                    openMarket = datetime.now().replace(hour=9, minute=30, second=0, microsecond=0)
                    closeMarket = datetime.now().replace(hour=16, minute=0, second=0, microsecond=0)



trader = Robinhood()
logged_in = trader.login(username="USERNAME", password="PASSWORD")

p1 = BackThread(trader=trader)
