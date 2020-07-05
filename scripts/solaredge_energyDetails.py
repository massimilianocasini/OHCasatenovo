#!/usr/bin/env python3.4
#-*- coding: utf-8 -*-

"""MISIPINO SCRIPT"""

author      = "Michele Berardi"
copyright   = "Copyright 2018, "
license     = "GPL"
version     = "1.0.1"
maintainer  = "Michele Berardi"
email       = "michymak@gmail.com"
status      = "Production"

import logging
import time
from datetime import datetime
from dateutil.relativedelta import relativedelta

date2 = datetime.today()+ relativedelta(minutes=+0)
date3 = datetime.today()+ relativedelta(minutes=-15)
datenow = date2.strftime('%Y-%m-%dT%H:%M:%S')

startdate = date3.strftime('%Y-%m-%d %H:%M:%S')
enddate = date2.strftime('%Y-%m-%d %H:%M:%S')

#print ("TIME NOW = ",datenow)
#print ("START DATE = ",startdate)
#print ("END DATE = ",enddate)    

url = "https://monitoringapi.solaredge.com/site/1020598/energyDetails?startTime="+startdate+"&endTime="+enddate+"&api_key=I27XHPNSU0EQAXKA1HTCG14GZVT166KA"

print (url)
