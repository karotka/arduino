#!/usr/bin/env python
from math import log

THERMISTORNOMINAL = 100000
BCOEFFICIENT = 3950
TEMPERATURENOMINAL = 25

def main():
    average = 90000.0
    print "Average:\t%s" % average
    
    steinhart = average / THERMISTORNOMINAL          # (R/Ro)
    print "1. Steinhart:\t%s" % steinhart
    
    steinhart = log(steinhart)                       #ln(R/Ro)
    print "2. Steinhart:\t%s" % steinhart
    
    steinhart /= BCOEFFICIENT                        # 1/B * ln(R/Ro)
    print "3. Steinhart:\t%s" % steinhart
    
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15) # + (1/To)
    print "4. Steinhart:\t%s" % steinhart
    
    steinhart = 1.0 / steinhart                      #Invert
    print "5. Steinhart:\t%s" % steinhart
    
    steinhart -= 273.15                              #convert to C
    print "6. Steinhart:\t%s" % steinhart


main()
