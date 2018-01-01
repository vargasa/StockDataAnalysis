# Given a USERNAME and PASSWORD for Robinhood account
# it logs tick by tick market and equity value to Daily.csv
# file. It must run in the background.

import csv, time
from datetime import datetime
from Robinhood import Robinhood

trader = Robinhood()
logged_in = trader.login(username="USERNAME", password="PASSWORD")

openMarket = datetime.now().replace(hour=9, minute=30, second=0, microsecond=0)
closeMarket = datetime.now().replace(hour=16, minute=0, second=0, microsecond=0)

dailyFile = open('Daily.csv', 'a+');

with dailyFile as csvfile:
    filewriter = csv.writer(csvfile, delimiter=',',
                            quotechar='|',
                            quoting=csv.QUOTE_MINIMAL)
    if logged_in:
        while True:
            date = datetime.now()
            
            if date.weekday() < 5 and date > openMarket and date < closeMarket :
                equity = trader.equity()
                marketValue = trader.market_value()
                filewriter.writerow([date.strftime("%Y-%m-%d %H:%M:%S"),equity,marketValue])
                dailyFile.flush()
                print ( date.strftime("%Y-%m-%d %H:%M:%S") , marketValue , equity )
                time.sleep(10)
            else :
                time.sleep(60)
                openMarket = datetime.now().replace(hour=9, minute=30, second=0, microsecond=0)
                closeMarket = datetime.now().replace(hour=16, minute=0, second=0, microsecond=0)
