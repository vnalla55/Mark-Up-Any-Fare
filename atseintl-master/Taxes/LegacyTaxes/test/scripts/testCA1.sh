#!/bin/bash

perl -s  /vobs/atseintl/Taxes/LegacyTaxes/test/scripts/taxMillUltra.pl -instrdb=pinhpp75.sabre.com:3306 -h1=piili001:53501  -h2=piili001:53501 -taxcode=CA1 -day=07 -m=`cat cacity`

