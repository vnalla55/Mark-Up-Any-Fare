#!/bin/bash

#export PERL5LIB=/login/sg891191/lib/perl5/site_perl/
perl -s  comparator.pl -instrdb=pinhpp75.sabre.com:3306 -h1=picli406:56000 -h2=piilc006:53701 -day=12 -reqtag=PricingRequest -restag=PricingResponse -limit=200 -prefix="test04_" -exclude=S79 

# -istrdb instrumentation database to get requests (host:port)
# -servicename (optional) selects servicename column from table in instrumentation db. "INTLWPI1" if not specified
# -day selects day of month from instr db (two digits)
# -reqtag root tag of requests selected from instr db

# following query is constructed in order to fetch requests from instrumentation db:
# SELECT INPUTCMD FROM XMLREQUEST$day WHERE SERVICENAME = '$servicename' AND INPUTCMD LIKE '<" . $reqtag . "%'

# -m=<regular expression> filters requests - sends only requests matching regular expr
# -s=<regular expression>:<string> - replaces matching part of request to specified string

# -limit specifies number of requests to send
# -transform PricingRequest to AirTaxRQ

# -h1 -h2 addresses of server to compare (host:port)
# -restag root tag expected in response

# -exclude list of attributes separated by ":" to be excluded from comparison, their value will be set to "" in responses before comparing 

# -prefix prefix added to the name of report file(s)
# for example for -prefix="/login/sg891191/test01_" and -day=12, /login/sg891191/test01_12bad.txt file will be created and optionally /login/sg891191/test01_12good.txt
# -savegood requests and responces which don't cause discrepancy are also saved to separate file (*good.txt) 
