#!/usr/bin/python

# A little script to send test data to an influxdb installation
# Attention, the non-core library 'requests' is used. You'll need to install it first:
# http://docs.python-requests.org/en/master/user/install/

import json
import math
import requests
import sys
from time import sleep

IP = "localhost"        # The IP of the machine hosting your influxdb instance
DB = "openhab_db"               # The database to write to, has to exist
USER = "openhab"             # The influxdb user to authenticate with
PASSWORD = "openhab"  # The password of that user
TIME = 1                  # Delay in seconds between two consecutive updates
STATUS_MOD = 5            # The interval in which the updates count will be printed to your console

n = 0
while True:
    for d in range(0, 360):
        v = 'sine_wave value=%s' % math.sin(math.radians(d))
        ## without autentication
        #r = requests.post("http://%s:8086/write?db=%s" %(IP, DB), data=v)
        ## with autentication
        r = requests.post("http://%s:8086/write?db=%s" %(IP, DB), auth=(USER, PASSWORD), data=v)
        if r.status_code != 204:
            print 'Failed to add point to influxdb (%d) - aborting.' %r.status_code
            sys.exit(1)
        n += 1
        sleep(TIME)
        if n % STATUS_MOD == 0:
            print '%d points inserted.' % n
